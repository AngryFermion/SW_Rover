// mqtt_client_setup.cpp

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
#include "aws_iot_certs.h"  // Embedded AWS IoT certificates

// WiFi and MQTT client instances
WiFiClient wifiClient;
WiFiClientSecure wifiClientSecure;
PubSubClient mqttClient(wifiClient);

// MQTT config cache
MqttConfig mqtt_config;

// MQTT task handle
static TaskHandle_t mqttTaskHandle = NULL;

// Static buffer for CA certificate
static char mqtt_ca_cert_buffer[2048] = {0};

static const char TOPIC_TELEMATICS_STATUS[] = "SmartWheelsNS/telematics/status";

// Forward declarations
void MqttClient_OnMessage(char* topic, byte* payload, unsigned int length);
void MqttClient_Reconnect();

// Flag to indicate if using embedded AWS IoT certificates (extern in header)
bool useEmbeddedAWSCerts = false;

bool MqttClient_ApplyCaCert() {
  // If using embedded AWS certificates, apply CA directly from PROGMEM
  if (useEmbeddedAWSCerts) {
    g_Logger.Write(LogLevel::Info, LogCategory::MQTT, "ApplyCaCert",
                     "Applying embedded AWS Root CA directly from PROGMEM");
    wifiClientSecure.setCACert(AWS_IOT_ROOT_CA);
    return true;
  }

  // Otherwise use CA from config (String variable)
  String caCert = mqtt_config.ca_cert;

  caCert.trim();
  if (caCert.length() <= 0) {
    g_Logger.Write(LogLevel::Error, LogCategory::MQTT, "ApplyCaCert",
                     "No CA cert provided in config.");
    return false;
  }

  String certFixed = mqtt_config.ca_cert;
  certFixed.replace("\\n", "\n");

  if (certFixed.length() >= sizeof(mqtt_ca_cert_buffer)) {
    g_Logger.Write(LogLevel::Error, LogCategory::MQTT, "ApplyCaCert",
                     "CA cert exceeds buffer size (%d bytes).",
                     sizeof(mqtt_ca_cert_buffer));
    return false;
  }

  strncpy(mqtt_ca_cert_buffer, certFixed.c_str(),
          sizeof(mqtt_ca_cert_buffer) - 1);
  mqtt_ca_cert_buffer[sizeof(mqtt_ca_cert_buffer) - 1] = '\0';

  wifiClientSecure.setCACert(mqtt_ca_cert_buffer);
  return true;
}

// Apply device certificate and private key for mutual TLS (AWS IoT)
bool MqttClient_ApplyDeviceCerts() {
  // Set timeout for TLS handshake (default is too short for AWS IoT)
  wifiClientSecure.setTimeout(15);  // 15 seconds timeout

  // If using embedded AWS certificates, apply them directly from PROGMEM
  if (useEmbeddedAWSCerts) {
    g_Logger.Write(LogLevel::Info, LogCategory::MQTT, "ApplyDeviceCerts",
                     "Applying embedded AWS IoT certificates directly from PROGMEM");
    wifiClientSecure.setCertificate(AWS_IOT_DEVICE_CERT);
    wifiClientSecure.setPrivateKey(AWS_IOT_PRIVATE_KEY);
    g_Logger.Write(LogLevel::Info, LogCategory::MQTT, "ApplyDeviceCerts",
                     "AWS IoT device certificates applied for mutual TLS");
    return true;
  }

  // Otherwise use certificates from config (String variables)
  if (mqtt_config.device_cert.isEmpty() || mqtt_config.private_key.isEmpty()) {
    g_Logger.Write(LogLevel::Info, LogCategory::MQTT, "ApplyDeviceCerts",
                     "No device cert/key - using CA-only TLS");
    return true;  // Not an error, just means not using mutual TLS
  }

  String deviceCert = mqtt_config.device_cert;
  String privateKey = mqtt_config.private_key;

  deviceCert.replace("\\n", "\n");
  privateKey.replace("\\n", "\n");

  g_Logger.Write(LogLevel::Debug, LogCategory::MQTT, "ApplyDeviceCerts",
                   "Applying device certificate (%d bytes) and private key (%d bytes)",
                   deviceCert.length(), privateKey.length());

  // Configure TLS settings for AWS IoT
  wifiClientSecure.setCertificate(deviceCert.c_str());
  wifiClientSecure.setPrivateKey(privateKey.c_str());

  g_Logger.Write(LogLevel::Info, LogCategory::MQTT, "ApplyDeviceCerts",
                   "Device certificates applied for mutual TLS");
  return true;
}

void MqttClient_ConfigureConnection() {
  // Validate server before proceeding
  if (mqtt_config.server.isEmpty()) {
    g_Logger.Write(LogLevel::Error, LogCategory::MQTT, "ConfigureConnection",
                     "MQTT server is empty. Cannot configure client.");
    return;
  }

  // Choose client based on encryption
  if (mqtt_config.encrypt) {
    if (!MqttClient_ApplyCaCert()) {
      g_Logger.Write(
          LogLevel::Error, LogCategory::MQTT, "ConfigureConnection",
          "Failed to apply CA certificate. TLS connection may fail.");
    }
    // Apply device certificates for mutual TLS (AWS IoT)
    if (!MqttClient_ApplyDeviceCerts()) {
      g_Logger.Write(
          LogLevel::Error, LogCategory::MQTT, "ConfigureConnection",
          "Failed to apply device certificates. AWS IoT connection may fail.");
    }
    mqttClient.setClient(wifiClientSecure);
  } else {
    mqttClient.setClient(wifiClient);
  }

  // Set MQTT server and port
  mqttClient.setServer(mqtt_config.server.c_str(), mqtt_config.port);

  // Set keep-alive timer
  mqttClient.setKeepAlive(mqtt_config.keepAlive);

  // Set buffer size for large FOTA messages (like in old project)
  mqttClient.setBufferSize(MQTT_BUFFER_SIZE);

  // Set message callback
  mqttClient.setCallback(MqttClient_OnMessage);

  g_Logger.Write(LogLevel::Debug, LogCategory::MQTT, "ConfigureConnection",
                   "MQTT client configured for %s:%d (encrypt: %s, buffer: %d)",
                   mqtt_config.server.c_str(), mqtt_config.port,
                   mqtt_config.encrypt ? "yes" : "no", MQTT_BUFFER_SIZE);
}

void MqttClient_OnMessage(char* topic, byte* payload, unsigned int length) {
  // Only log FOTA messages to reduce spam
  String topicStr = String(topic);
  if (topicStr.indexOf("SmartWheelsNS/fota/") >= 0) {
    g_Logger.Write(LogLevel::Debug, LogCategory::MQTT, "FOTA Message",
                     "Topic: %s, Length: %d", topic, length);
  }

  // Route messages to appropriate handlers
  fota_handle_mqtt_message(topic, payload, length);

  // Future: Add other message handlers here as needed
  // e.g., device_handle_mqtt_message(topic, payload, length);

  // Yield after message processing to prevent watchdog timeout
  taskYIELD();
}

void MqttClient_Reconnect() {
  static unsigned long lastReconnectAttempt = 0;

  if (!mqttClient.connected()) {
    unsigned long now = millis();

    // Only attempt reconnection every 5 seconds to prevent spam
    if (now - lastReconnectAttempt > 5000) {
      g_Logger.Write(LogLevel::Debug, LogCategory::MQTT, "Reconnect",
                       "Attempting MQTT connection...");

      bool connected = false;

      // AWS IoT uses simple connection with Thing name as client ID (no username/password/will)
      if (useEmbeddedAWSCerts) {
        g_Logger.Write(LogLevel::Info, LogCategory::MQTT, "Reconnect",
                         "Connecting to AWS IoT with Thing ID: %s", mqtt_config.clientId.c_str());
        connected = mqttClient.connect(mqtt_config.clientId.c_str());
      } else {
        // For other brokers, use will message and credentials if available
        String willTopic = String(TOPIC_TELEMATICS_STATUS) + "/" + mqtt_config.clientId;

        if (!mqtt_config.username.isEmpty() && !mqtt_config.password.isEmpty()) {
          // Connect with credentials and will message
          connected = mqttClient.connect(mqtt_config.clientId.c_str(),
                                       mqtt_config.username.c_str(),
                                       mqtt_config.password.c_str(),
                                       willTopic.c_str(),
                                       mqtt_config.qos,
                                       MQTT_NO_RETAIN,
                                       "offline");
        } else {
          // Connect without credentials but with will message
          connected = mqttClient.connect(mqtt_config.clientId.c_str(),
                                       willTopic.c_str(),
                                       mqtt_config.qos,
                                       MQTT_NO_RETAIN,
                                       "offline");
        }
      }

      // Update timestamp AFTER connection attempt to ensure proper 5-second interval
      lastReconnectAttempt = millis();

      if (connected) {
        g_Logger.Write(LogLevel::Info, LogCategory::MQTT, "Connected",
                         "Connected to MQTT broker successfully");

        // For AWS IoT, subscribe to the wildcard topic
        if (useEmbeddedAWSCerts) {
          // Subscribe to AWS IoT topic: sdv/vehicles/sdv-vehicle-001/#
          extern const char* topic_smartwheels;  // Defined in ancit_fota_handler.cpp
          bool subSuccess = MqttClient_Subscribe(topic_smartwheels);
          g_Logger.Write(LogLevel::Info, LogCategory::MQTT, "Connected",
                           "AWS IoT subscription to %s: %s",
                           topic_smartwheels, subSuccess ? "SUCCESS" : "FAILED");
        } else {
          // Publish online status for non-AWS brokers with will topic
          String willTopic = String(TOPIC_TELEMATICS_STATUS) + "/" + mqtt_config.clientId;
          mqttClient.publish(willTopic.c_str(), "online", MQTT_NO_RETAIN);
          g_Logger.Write(LogLevel::Debug, LogCategory::MQTT, "Connected",
                           "Published online status to will topic");
        }

        // Initialize FOTA MQTT functionality
        fota_mqtt_init();

        g_Logger.Write(LogLevel::Info, LogCategory::MQTT, "Connected",
                         "FOTA MQTT initialized");

        netLed->setState(LedMode::BLINK_ONCE, LedColor::GREEN);
      } else {
        int rc = mqttClient.state();
        const char* error_msg = "Unknown error";

        switch(rc) {
          case -4: error_msg = "MQTT_CONNECTION_TIMEOUT - Server didn't respond within keepalive time"; break;
          case -3: error_msg = "MQTT_CONNECTION_LOST - Network connection broken"; break;
          case -2: error_msg = "MQTT_CONNECT_FAILED - Network connection failed"; break;
          case -1: error_msg = "MQTT_DISCONNECTED - Client disconnected cleanly"; break;
          case 1:  error_msg = "MQTT_CONNECT_BAD_PROTOCOL - Unsupported protocol version"; break;
          case 2:  error_msg = "MQTT_CONNECT_BAD_CLIENT_ID - Invalid client ID"; break;
          case 3:  error_msg = "MQTT_CONNECT_UNAVAILABLE - Server unavailable"; break;
          case 4:  error_msg = "MQTT_CONNECT_BAD_CREDENTIALS - Invalid username/password"; break;
          case 5:  error_msg = "MQTT_CONNECT_UNAUTHORIZED - Client not authorized"; break;
          default: error_msg = "Unknown MQTT error"; break;
        }

        g_Logger.Write(LogLevel::Error, LogCategory::MQTT, "Reconnect",
                         "Failed to connect, rc=%d (%s). Will retry in 5 seconds...",
                         rc, error_msg);
        netLed->setState(LedMode::BLINK_ONCE, LedColor::RED);
      }
    }
  }
}

bool MqttClient_Publish(const char* topic, const char* payload, bool retain) {
  if (!mqttClient.connected()) {
    g_Logger.Write(LogLevel::Error, LogCategory::MQTT, "Publish",
                     "MQTT client not connected");
    return false;
  }

  bool result = mqttClient.publish(topic, payload, retain);
  if (result) {
    g_Logger.Write(LogLevel::Debug, LogCategory::MQTT, "Published",
                     "Topic: %s, Payload: %s", topic, payload);
    netLed->setState(LedMode::BLINK_ONCE, LedColor::GREEN);
  } else {
    g_Logger.Write(LogLevel::Error, LogCategory::MQTT, "Publish",
                     "Failed to publish to topic: %s", topic);
    netLed->setState(LedMode::BLINK_ONCE, LedColor::RED);
  }
  return result;
}

bool MqttClient_Subscribe(const char* topic) {
  if (!mqttClient.connected()) {
    g_Logger.Write(LogLevel::Error, LogCategory::MQTT, "Subscribe",
                     "MQTT client not connected");
    return false;
  }

  bool result = mqttClient.subscribe(topic, mqtt_config.qos);
  if (result) {
    g_Logger.Write(LogLevel::Debug, LogCategory::MQTT, "Subscribed",
                     "Topic: %s", topic);
  } else {
    g_Logger.Write(LogLevel::Error, LogCategory::MQTT, "Subscribe",
                     "Failed to subscribe to topic: %s", topic);
  }
  return result;
}

bool MqttClient_IsConnected() {
  return mqttClient.connected();
}

void MqttClient_Loop() {
  if (!mqttClient.connected()) {
    MqttClient_Reconnect();
  }
  mqttClient.loop();

  // Yield to prevent watchdog timeout
  vTaskDelay(pdMS_TO_TICKS(10));
}

// Public init function (called from task)
void MqttClient_Init(const MqttConfig& config) {
  MqttClient_SetClientId();
  MqttClient_ConfigureConnection();

  g_Logger.Write(LogLevel::Debug, LogCategory::MQTT, "MqttClient_Init",
                   "MQTT client initialized for %s:%d",
                   mqtt_config.server.c_str(), mqtt_config.port);
}

void MqttClient_SetClientId() {
  // Use fixed client ID for AWS IoT (Thing Name)
  // For AWS IoT, the client ID should match the Thing Name registered in AWS
  mqtt_config.clientId = "sdv-vehicle-001";

  g_Logger.Write(LogLevel::Info, LogCategory::MQTT, "SetClientId",
                   "Using client ID: %s", mqtt_config.clientId.c_str());
}

// Helper function to load certificate from SPIFFS file
String MqttClient_LoadCertFromFile(const String& filepath) {
  if (filepath.isEmpty()) {
    return "";
  }

  if (!SPIFFS.exists(filepath.c_str())) {
    g_Logger.Write(LogLevel::Error, LogCategory::MQTT, "LoadCertFromFile",
                     "Certificate file not found: %s", filepath.c_str());
    return "";
  }

  File file = SPIFFS.open(filepath.c_str(), "r");
  if (!file) {
    g_Logger.Write(LogLevel::Error, LogCategory::MQTT, "LoadCertFromFile",
                     "Failed to open certificate file: %s", filepath.c_str());
    return "";
  }

  String content = file.readString();
  file.close();

  g_Logger.Write(LogLevel::Debug, LogCategory::MQTT, "LoadCertFromFile",
                   "Loaded certificate from %s (%d bytes)",
                   filepath.c_str(), content.length());
  return content;
}

void MqttClient_LoadFromJson(const JsonDocument& doc) {
  JsonObjectConst mqttObj = doc["mqtt"];
  if (!mqttObj) return;

  mqtt_config.enabled = mqttObj["en"] | false;
  mqtt_config.server = mqttObj["server"] | "";
  mqtt_config.encrypt = mqttObj["encrypt"] | false;
  mqtt_config.port = mqttObj["port"] | 1883;
  mqtt_config.keepAlive = mqttObj["ka"] | 60;
  mqtt_config.username = mqttObj["un"] | "";
  mqtt_config.password = mqttObj["pw"] | "";
  mqtt_config.qos = mqttObj["qos"] | 0;
  mqtt_config.publish_interval = mqttObj["pi"] | 60;

  // Detect AWS IoT endpoint and use embedded PROGMEM certificates
  if (mqtt_config.encrypt && mqtt_config.server.indexOf("amazonaws.com") >= 0) {
    g_Logger.Write(LogLevel::Info, LogCategory::MQTT, "LoadFromJson",
                     "Detected AWS IoT endpoint - will use embedded certificates from PROGMEM");
    useEmbeddedAWSCerts = true;  // Set flag to use PROGMEM directly in ApplyCaCert/ApplyDeviceCerts
    // Don't convert to String - certificates will be applied directly from PROGMEM
    // This prevents corruption of certificate format
  } else {
    useEmbeddedAWSCerts = false;
    // Load CA certificate (from inline string or file)
    String caCertValue = mqttObj["ca_cert"] | "";
    if (caCertValue.isEmpty()) {
      String caCertFile = mqttObj["ca_cert_file"] | "";
      mqtt_config.ca_cert = MqttClient_LoadCertFromFile(caCertFile);
    } else {
      mqtt_config.ca_cert = caCertValue;
    }

    // Load device certificate (from inline string or file) - for AWS IoT
    String deviceCertValue = mqttObj["device_cert"] | "";
    if (deviceCertValue.isEmpty()) {
      String deviceCertFile = mqttObj["device_cert_file"] | "";
      mqtt_config.device_cert = MqttClient_LoadCertFromFile(deviceCertFile);
    } else {
      mqtt_config.device_cert = deviceCertValue;
    }

    // Load private key (from inline string or file) - for AWS IoT
    String privateKeyValue = mqttObj["private_key"] | "";
    if (privateKeyValue.isEmpty()) {
      String privateKeyFile = mqttObj["private_key_file"] | "";
      mqtt_config.private_key = MqttClient_LoadCertFromFile(privateKeyFile);
    } else {
      mqtt_config.private_key = privateKeyValue;
    }
  }

  // Log certificate status
  g_Logger.Write(LogLevel::Info, LogCategory::MQTT, "LoadFromJson",
                   "Certificates loaded - CA: %d bytes, Device: %d bytes, Key: %d bytes",
                   mqtt_config.ca_cert.length(),
                   mqtt_config.device_cert.length(),
                   mqtt_config.private_key.length());

  if (mqtt_config.server.length() >= 3 && mqtt_config.port > 0) {
    mqtt_config.uri_full = (mqtt_config.encrypt ? "mqtts://" : "mqtt://") +
                           mqtt_config.server + ":" + String(mqtt_config.port);
  } else {
    mqtt_config.uri_full = "";
  }
}

// Call this explicitly when ready to launch MQTT task
void MqttClient_CreateTask(void) {
  if (mqttTaskHandle != NULL) {
    g_Logger.Write(LogLevel::Warn, LogCategory::MQTT, "CreateTask",
                   "MQTT task already exists, skipping creation");
    return;
  }

  xTaskCreatePinnedToCore(MqttClient_ApplicationTask,
                          "mqtt_app_task",
                          ANCIT_MQTT_APP_STACK_SIZE,
                          NULL,
                          ANCIT_MQTT_APP_PRIORITY,
                          &mqttTaskHandle,
                          ANCIT_MQTT_APP_TASK_CORE);

  g_Logger.Write(LogLevel::Debug, LogCategory::DEVICE, "CreateTask",
                   "MQTT Application task created successfully");
}

// Stop MQTT task and disconnect
void MqttClient_Stop(void) {
  if (mqttTaskHandle == NULL) {
    g_Logger.Write(LogLevel::Debug, LogCategory::MQTT, "Stop",
                   "MQTT task not running, nothing to stop");
    return;
  }

  g_Logger.Write(LogLevel::Info, LogCategory::MQTT, "Stop",
                 "Stopping MQTT client and task...");

  // Disconnect from MQTT broker
  if (mqttClient.connected()) {
    mqttClient.disconnect();
    g_Logger.Write(LogLevel::Debug, LogCategory::MQTT, "Stop",
                   "Disconnected from MQTT broker");
  }

  // CRITICAL: Remove task from watchdog BEFORE deleting it
  // Store handle and clear it first to prevent re-entry
  TaskHandle_t taskToDelete = mqttTaskHandle;
  mqttTaskHandle = NULL;

  // Delete watchdog registration for the task
  esp_task_wdt_delete(taskToDelete);

  // Delete the task
  vTaskDelete(taskToDelete);

  g_Logger.Write(LogLevel::Info, LogCategory::MQTT, "Stop",
                 "MQTT task stopped successfully");
}

// Restart MQTT (stop and start)
void MqttClient_Restart(void) {
  g_Logger.Write(LogLevel::Info, LogCategory::MQTT, "Restart",
                 "Restarting MQTT client...");

  MqttClient_Stop();
  vTaskDelay(pdMS_TO_TICKS(500)); // Small delay before restart
  MqttClient_CreateTask();

  g_Logger.Write(LogLevel::Info, LogCategory::MQTT, "Restart",
                 "MQTT client restarted successfully");
}

static void MqttClient_WaitForWifi() {
  int logCount = 0;
  while (!globals.wifi.got_ip) {
    if (logCount < 3) {
      g_Logger.Write(LogLevel::Debug, LogCategory::MQTT, "MqttClient_WaitForWifi",
                       "Waiting for WiFi connection...");
      logCount++;
    }
    esp_task_wdt_reset();  // Reset watchdog while waiting
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
  g_Logger.Write(LogLevel::Debug, LogCategory::MQTT, "MqttClient_WaitForWifi",
                   "WiFi connected, proceeding with MQTT setup");
}

static void MqttClient_WaitForValidTime() {
  int logCount = 0;
  while (!globals.time.valid) {
    if (logCount < 10) {
      g_Logger.Write(LogLevel::Debug, LogCategory::MQTT,
                       "MqttClient_WaitForValidTime",
                       "Waiting for valid time before publishing...");
      logCount++;
    }
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

static void MqttClient_PublishOneTimeParams() {
  // This function can be extended for other one-time MQTT publications
  // FOTA initialization is now handled in fota_mqtt_init() called from OnConnect

  g_Logger.Write(LogLevel::Debug, LogCategory::MQTT,
                   "MqttClient_PublishOneTimeParams",
                   "One-time MQTT parameters published");
}

// Generic task tied to MQTT logic (status, periodic ops, etc.)
void MqttClient_ApplicationTask(void* pvParams) {
  extern JsonDocument devComm_doc;

  // Add this task to the watchdog timer
  esp_task_wdt_add(NULL);

  // Load MQTT configuration from JSON document
  MqttClient_LoadFromJson(devComm_doc);

  // Check if MQTT is enabled in the configuration
  if (!mqtt_config.enabled) {
    g_Logger.Write(LogLevel::Warn, LogCategory::MQTT, "MqttClient_ApplicationTask",
                     "MQTT is disabled in config, exiting task");
    esp_task_wdt_delete(NULL);
    vTaskDelete(NULL);
  }

  // Wait for WiFi connection before proceeding
  MqttClient_WaitForWifi();
  // Initialize MQTT client with loaded config
  MqttClient_Init(mqtt_config);
  // Wait for valid time before publishing sensor values - COMMENTED OUT FOR NOW
  // MqttClient_WaitForValidTime();
  // Publish one-time parameters like device and sensor info
  MqttClient_PublishOneTimeParams();

  // wait for device data ready event
  // Device_WaitForDataReady("MqttClient_ApplicationTask");

  // Start periodic MQTT loop and publishing
  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xInterval =
      pdMS_TO_TICKS(mqtt_config.publish_interval * 1000);

  while (true) {
    // Reset watchdog at the start of each loop iteration
    esp_task_wdt_reset();

    if (globals.wifi.got_ip) {
      // Keep MQTT connection alive and process messages
      MqttClient_Loop();

      // Yield to other tasks frequently
      vTaskDelay(pdMS_TO_TICKS(100)); // 100ms delay to prevent watchdog timeout

      // Check if it's time for periodic publishing
      if ((xTaskGetTickCount() - xLastWakeTime) >= xInterval) {
        xLastWakeTime = xTaskGetTickCount();
        // Add any periodic MQTT publishing here if needed
      }
    } else {
      g_Logger.Write(LogLevel::Warn, LogCategory::MQTT,
                       "MqttClient_ApplicationTask",
                       "WiFi not connected. Waiting 5 seconds...");
      vTaskDelay(pdMS_TO_TICKS(5000));
    }
  }
}