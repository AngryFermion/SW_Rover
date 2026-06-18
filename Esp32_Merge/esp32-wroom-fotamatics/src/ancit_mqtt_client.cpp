// ancit_mqtt_client.cpp — SmartWheels ESP32-WROOM FOTAmatics
// Unified MQTT client: handles FOTA topics (SmartWheelsNS/...) and
// telematics topics (SmartKit/...) on the same EMQX broker connection.

#include "ancit_mqtt_client.h"
#include <ArduinoJson.h>
#include <WiFi.h>
#include <BT_LOGGER.h>
#include <esp_task_wdt.h>
#include "config_reader.h"
#include "ancit_device.h"
#include "ancit_fota_handler.h"
#include "AncitRgbLed.h"
#include "ancit_time.h"
#include "ancit_tasks.h"
#include "aws_iot_certs.h"
#include "telematics_config.h"
#include "app_config.h"

// MQTT client instances
WiFiClient       wifiClient;
WiFiClientSecure wifiClientSecure;
PubSubClient     mqttClient(wifiClient);

MqttConfig mqtt_config;

static TaskHandle_t     mqttTaskHandle = NULL;
static char             mqtt_ca_cert_buffer[2048] = {0};
// Recursive mutex: PubSubClient is not thread-safe. Both the MQTT task (loop)
// and the telematics task (publish) run on Core 1 — unguarded concurrent access
// corrupts socket state. Recursive allows publishChunkAck() to re-enter
// MqttClient_Publish() from inside the mqttClient.loop() callback.
static SemaphoreHandle_t mqttMutex = NULL;

static const char TOPIC_TELEMATICS_STATUS[] = "SmartWheelsNS/telematics/status";

bool useEmbeddedAWSCerts = false;

// Forward declarations
void MqttClient_OnMessage(char* topic, byte* payload, unsigned int length);
void MqttClient_Reconnect(void);

// ============================================================================
// TLS certificate helpers (AWS IoT path — unused with EMQX)
// ============================================================================

bool MqttClient_ApplyCaCert() {
    if (useEmbeddedAWSCerts) {
        wifiClientSecure.setCACert(AWS_IOT_ROOT_CA);
        return true;
    }
    String caCert = mqtt_config.ca_cert;
    caCert.trim();
    if (caCert.length() == 0) return false;
    String certFixed = caCert;
    certFixed.replace("\\n", "\n");
    if (certFixed.length() >= sizeof(mqtt_ca_cert_buffer)) return false;
    strncpy(mqtt_ca_cert_buffer, certFixed.c_str(), sizeof(mqtt_ca_cert_buffer) - 1);
    mqtt_ca_cert_buffer[sizeof(mqtt_ca_cert_buffer) - 1] = '\0';
    wifiClientSecure.setCACert(mqtt_ca_cert_buffer);
    return true;
}

bool MqttClient_ApplyDeviceCerts() {
    wifiClientSecure.setTimeout(15);
    if (useEmbeddedAWSCerts) {
        wifiClientSecure.setCertificate(AWS_IOT_DEVICE_CERT);
        wifiClientSecure.setPrivateKey(AWS_IOT_PRIVATE_KEY);
        return true;
    }
    if (mqtt_config.device_cert.isEmpty() || mqtt_config.private_key.isEmpty()) return true;
    String deviceCert = mqtt_config.device_cert;
    String privateKey = mqtt_config.private_key;
    deviceCert.replace("\\n", "\n");
    privateKey.replace("\\n", "\n");
    wifiClientSecure.setCertificate(deviceCert.c_str());
    wifiClientSecure.setPrivateKey(privateKey.c_str());
    return true;
}

// ============================================================================
// Connection configuration
// ============================================================================

void MqttClient_ConfigureConnection() {
    if (mqtt_config.server.isEmpty()) return;

    if (mqtt_config.encrypt) {
        MqttClient_ApplyCaCert();
        MqttClient_ApplyDeviceCerts();
        mqttClient.setClient(wifiClientSecure);
    } else {
        mqttClient.setClient(wifiClient);
    }

    mqttClient.setServer(mqtt_config.server.c_str(), mqtt_config.port);
    mqttClient.setKeepAlive(mqtt_config.keepAlive);
    mqttClient.setBufferSize(MQTT_BUFFER_SIZE);
    mqttClient.setCallback(MqttClient_OnMessage);

    g_Logger.Write(LogLevel::Info, LogCategory::MQTT, "ConfigureConnection",
                   "MQTT → %s:%d (encrypt:%s buffer:%d)",
                   mqtt_config.server.c_str(), mqtt_config.port,
                   mqtt_config.encrypt ? "yes" : "no", MQTT_BUFFER_SIZE);
}

// ============================================================================
// Message callback — routes FOTA and telematics topics
// ============================================================================

void MqttClient_OnMessage(char* topic, byte* payload, unsigned int length) {
    String topicStr = String(topic);

#if FOTAMATICS_MODE != APP_MODE_FOTA_ONLY
    // ---- Telematics control: SmartKit/Control → Serial1 TX → MCU ----
    if (topicStr == TELEMATICS_TOPIC_CONTROL) {
        String msg = "";
        for (unsigned int i = 0; i < length; i++) msg += (char)payload[i];

        g_Logger.Write(LogLevel::Info, LogCategory::MQTT, "OnMessage",
                       "[Telematics] Control → MCU: %s", msg.c_str());

        Serial1.print(msg);
        Serial1.print("\r\n");
        return;
    }
#endif

#if FOTAMATICS_MODE != APP_MODE_TELEMATICS_ONLY
    // ---- FOTA messages ----
    if (topicStr.indexOf("SmartWheelsNS/") >= 0 ||
        topicStr.indexOf("sdv/vehicles/")  >= 0) {
        g_Logger.Write(LogLevel::Debug, LogCategory::MQTT, "OnMessage",
                       "[FOTA] Topic: %s len: %d", topic, length);
        fota_handle_mqtt_message(topic, payload, length);
        return;
    }
#endif

    g_Logger.Write(LogLevel::Debug, LogCategory::MQTT, "OnMessage",
                   "Unhandled topic: %s", topic);

    taskYIELD();
}

// ============================================================================
// Reconnect
// ============================================================================

void MqttClient_Reconnect() {
    static unsigned long lastAttempt = 0;
    if (mqttClient.connected()) return;

    unsigned long now = millis();
    if (now - lastAttempt < 5000) return;
    lastAttempt = now;

    g_Logger.Write(LogLevel::Debug, LogCategory::MQTT, "Reconnect",
                   "Attempting MQTT connection to %s:%d ...",
                   mqtt_config.server.c_str(), mqtt_config.port);

    bool connected = false;

    if (useEmbeddedAWSCerts) {
        connected = mqttClient.connect(mqtt_config.clientId.c_str());
    } else {
        String willTopic = String(TOPIC_TELEMATICS_STATUS) + "/" + mqtt_config.clientId;
        if (!mqtt_config.username.isEmpty() && !mqtt_config.password.isEmpty()) {
            connected = mqttClient.connect(mqtt_config.clientId.c_str(),
                                           mqtt_config.username.c_str(),
                                           mqtt_config.password.c_str(),
                                           willTopic.c_str(),
                                           mqtt_config.qos,
                                           MQTT_NO_RETAIN,
                                           "offline");
        } else {
            connected = mqttClient.connect(mqtt_config.clientId.c_str(),
                                           willTopic.c_str(),
                                           mqtt_config.qos,
                                           MQTT_NO_RETAIN,
                                           "offline");
        }
    }

    if (connected) {
        g_Logger.Write(LogLevel::Info, LogCategory::MQTT, "Reconnect",
                       "Connected to MQTT broker");

        // --- FOTA subscriptions ---
        if (useEmbeddedAWSCerts) {
            extern const char* topic_smartwheels;
            MqttClient_Subscribe(topic_smartwheels);
        } else {
            // fota_mqtt_init() will subscribe FOTA topics after topics are built
        }

#if FOTAMATICS_MODE != APP_MODE_FOTA_ONLY
        // Telematics: publish online status, subscribe control topic
        if (!useEmbeddedAWSCerts) {
            String willTopic = String(TOPIC_TELEMATICS_STATUS) + "/" + mqtt_config.clientId;
            mqttClient.publish(willTopic.c_str(), "online", MQTT_NO_RETAIN);
        }
        MqttClient_Subscribe(TELEMATICS_TOPIC_CONTROL);
        g_Logger.Write(LogLevel::Info, LogCategory::MQTT, "Reconnect",
                       "Subscribed to telematics control: %s", TELEMATICS_TOPIC_CONTROL);
#endif

#if FOTAMATICS_MODE != APP_MODE_TELEMATICS_ONLY
        // Initialize FOTA MQTT topics and serial handler (idempotent)
        fota_mqtt_init();
#endif

        netLed->setState(LedMode::BLINK_ONCE, LedColor::GREEN);
    } else {
        int rc = mqttClient.state();
        g_Logger.Write(LogLevel::Error, LogCategory::MQTT, "Reconnect",
                       "Connection failed rc=%d — retry in 5 s", rc);
        netLed->setState(LedMode::BLINK_ONCE, LedColor::RED);
    }
}

// ============================================================================
// Public API
// ============================================================================

bool MqttClient_Publish(const char* topic, const char* payload, bool retain) {
    if (!mqttClient.connected()) {
        g_Logger.Write(LogLevel::Error, LogCategory::MQTT, "Publish", "Not connected");
        return false;
    }
    if (mqttMutex == NULL || xSemaphoreTakeRecursive(mqttMutex, pdMS_TO_TICKS(200)) != pdTRUE) {
        g_Logger.Write(LogLevel::Warn, LogCategory::MQTT, "Publish", "Mutex timeout — skipped");
        return false;
    }
    bool ok = mqttClient.publish(topic, payload, retain);
    xSemaphoreGiveRecursive(mqttMutex);
    if (ok) {
        g_Logger.Write(LogLevel::Debug, LogCategory::MQTT, "Publish",
                       "%s → %s", topic, payload);
        netLed->setState(LedMode::BLINK_ONCE, LedColor::GREEN);
    } else {
        g_Logger.Write(LogLevel::Error, LogCategory::MQTT, "Publish",
                       "Failed: %s", topic);
        netLed->setState(LedMode::BLINK_ONCE, LedColor::RED);
    }
    return ok;
}

bool MqttClient_Subscribe(const char* topic) {
    if (!mqttClient.connected()) return false;
    bool ok = mqttClient.subscribe(topic, mqtt_config.qos);
    if (ok) {
        g_Logger.Write(LogLevel::Debug, LogCategory::MQTT, "Subscribe", "→ %s", topic);
    }
    return ok;
}

bool MqttClient_IsConnected() { return mqttClient.connected(); }

void MqttClient_Loop() {
    if (mqttMutex == NULL) return;
    // 200 ms timeout matches MqttClient_Publish — ensures loop() always gets
    // the mutex even when one publish is in progress (TCP write ~20-30 ms).
    if (xSemaphoreTakeRecursive(mqttMutex, pdMS_TO_TICKS(200)) == pdTRUE) {
        if (!mqttClient.connected()) MqttClient_Reconnect();
        mqttClient.loop();
        xSemaphoreGiveRecursive(mqttMutex);
    }
    vTaskDelay(pdMS_TO_TICKS(10));
}

void MqttClient_Init(const MqttConfig& config) {
    if (mqttMutex == NULL) mqttMutex = xSemaphoreCreateRecursiveMutex();
    MqttClient_SetClientId();
    MqttClient_ConfigureConnection();
}

void MqttClient_SetClientId() {
    mqtt_config.clientId = "sdv-vehicle-001";
    g_Logger.Write(LogLevel::Info, LogCategory::MQTT, "SetClientId",
                   "Client ID: %s", mqtt_config.clientId.c_str());
}

String MqttClient_LoadCertFromFile(const String& filepath) {
    if (filepath.isEmpty() || !SPIFFS.exists(filepath.c_str())) return "";
    File f = SPIFFS.open(filepath.c_str(), "r");
    if (!f) return "";
    String content = f.readString();
    f.close();
    return content;
}

void MqttClient_LoadFromJson(const JsonDocument& doc) {
    JsonObjectConst mqttObj = doc["mqtt"];
    if (!mqttObj) return;

    mqtt_config.enabled          = mqttObj["en"]      | false;
    mqtt_config.server           = mqttObj["server"]  | "";
    mqtt_config.encrypt          = mqttObj["encrypt"] | false;
    mqtt_config.port             = mqttObj["port"]    | 1883;
    mqtt_config.keepAlive        = mqttObj["ka"]      | 60;
    mqtt_config.username         = mqttObj["un"]      | "";
    mqtt_config.password         = mqttObj["pw"]      | "";
    mqtt_config.qos              = mqttObj["qos"]     | 0;
    mqtt_config.publish_interval = mqttObj["pi"]      | 60;

    if (mqtt_config.encrypt && mqtt_config.server.indexOf("amazonaws.com") >= 0) {
        useEmbeddedAWSCerts = true;
    } else {
        useEmbeddedAWSCerts = false;
        String caCertValue = mqttObj["ca_cert"] | "";
        mqtt_config.ca_cert = caCertValue.isEmpty()
            ? MqttClient_LoadCertFromFile(mqttObj["ca_cert_file"] | "")
            : caCertValue;
        String dcValue = mqttObj["device_cert"] | "";
        mqtt_config.device_cert = dcValue.isEmpty()
            ? MqttClient_LoadCertFromFile(mqttObj["device_cert_file"] | "")
            : dcValue;
        String pkValue = mqttObj["private_key"] | "";
        mqtt_config.private_key = pkValue.isEmpty()
            ? MqttClient_LoadCertFromFile(mqttObj["private_key_file"] | "")
            : pkValue;
    }

    if (mqtt_config.server.length() >= 3 && mqtt_config.port > 0) {
        mqtt_config.uri_full = (mqtt_config.encrypt ? "mqtts://" : "mqtt://") +
                               mqtt_config.server + ":" + String(mqtt_config.port);
    }
}

void MqttClient_CreateTask(void) {
    if (mqttTaskHandle != NULL) return;
    xTaskCreatePinnedToCore(MqttClient_ApplicationTask,
                            "mqtt_app_task",
                            ANCIT_MQTT_APP_STACK_SIZE,
                            NULL,
                            ANCIT_MQTT_APP_PRIORITY,
                            &mqttTaskHandle,
                            ANCIT_MQTT_APP_TASK_CORE);
    g_Logger.Write(LogLevel::Info, LogCategory::MQTT, "CreateTask", "MQTT task created");
}

void MqttClient_Stop(void) {
    if (mqttTaskHandle == NULL) return;
    if (mqttClient.connected()) mqttClient.disconnect();
    TaskHandle_t h = mqttTaskHandle;
    mqttTaskHandle = NULL;
    esp_task_wdt_delete(h);
    vTaskDelete(h);
    g_Logger.Write(LogLevel::Info, LogCategory::MQTT, "Stop", "MQTT task stopped");
}

void MqttClient_Restart(void) {
    MqttClient_Stop();
    vTaskDelay(pdMS_TO_TICKS(500));
    MqttClient_CreateTask();
}

// ============================================================================
// Application task
// ============================================================================

static void MqttClient_WaitForWifi() {
    int n = 0;
    while (!globals.wifi.got_ip) {
        if (n++ < 3) g_Logger.Write(LogLevel::Debug, LogCategory::MQTT,
                                     "WaitForWifi", "Waiting for WiFi...");
        esp_task_wdt_reset();
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void MqttClient_ApplicationTask(void* pvParams) {
    extern JsonDocument devComm_doc;

    esp_task_wdt_add(NULL);
    MqttClient_LoadFromJson(devComm_doc);

    if (!mqtt_config.enabled) {
        g_Logger.Write(LogLevel::Warn, LogCategory::MQTT, "AppTask",
                       "MQTT disabled in config — exiting task");
        esp_task_wdt_delete(NULL);
        vTaskDelete(NULL);
        return;
    }

    MqttClient_WaitForWifi();
    MqttClient_Init(mqtt_config);

    TickType_t xLastWake = xTaskGetTickCount();
    const TickType_t xInterval = pdMS_TO_TICKS(mqtt_config.publish_interval * 1000);

    while (true) {
        esp_task_wdt_reset();
        if (globals.wifi.got_ip) {
            MqttClient_Loop();
            vTaskDelay(pdMS_TO_TICKS(100));
            if ((xTaskGetTickCount() - xLastWake) >= xInterval) {
                xLastWake = xTaskGetTickCount();
            }
        } else {
            g_Logger.Write(LogLevel::Warn, LogCategory::MQTT, "AppTask",
                           "WiFi down — waiting 5 s");
            vTaskDelay(pdMS_TO_TICKS(5000));
        }
    }
}
