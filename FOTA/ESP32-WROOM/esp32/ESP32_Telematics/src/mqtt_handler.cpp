#include "mqtt_handler.h"
#include "wifi_handler.h"
#include "led_handler.h"
#include "fota_handler.h"
#include "utils.h"
#if USE_AWS_IOT
  #include "certs.h"
#endif

MQTTHandler mqttHandler;

MQTTHandler::MQTTHandler() : mqttClient(espClient) {
}

void MQTTHandler::init() {
#if USE_AWS_IOT
  // Configure secure client with AWS IoT certificates
  espClient.setCACert(AWS_CERT_CA);
  espClient.setCertificate(AWS_CERT_CRT);
  espClient.setPrivateKey(AWS_CERT_PRIVATE);
  DebugLog.Write(LogLevel::Info, LogCategory::MQTT, "init", "AWS IoT certificates loaded");
#endif

  mqttClient.setServer(mqtt_server, mqtt_port);
  mqttClient.setBufferSize(MQTT_BUFFER_SIZE);
  mqttClient.setCallback(mqttCallback);

#if USE_AWS_IOT
  DebugLog.Write(LogLevel::Info, LogCategory::MQTT, "init", "AWS IoT Core MQTT client initialized");
#else
  DebugLog.Write(LogLevel::Info, LogCategory::MQTT, "init", "Public MQTT broker client initialized");
#endif
}

void MQTTHandler::connectToMQTT() {
  unsigned long currentTime = millis();

  if (wifiHandler.isConnected() && !mqttClient.connected()) {
    // Throttle connection attempts
    if (currentTime - lastConnectionAttempt < CONNECTION_RETRY_DELAY) {
      return;
    }

    lastConnectionAttempt = currentTime;
    DebugLog.Write(LogLevel::Info, LogCategory::MQTT, "connectToMQTT", "Attempting MQTT connection...");

    mqttClient.setKeepAlive(MQTT_KEEPALIVE);

#if USE_AWS_IOT
    // AWS IoT uses Thing name as client ID (no username/password)
    String uniqueClientId = String(mqtt_client_id);
    DebugLog.Write(LogLevel::Info, LogCategory::MQTT, "connectToMQTT", "Using AWS Thing ID: " + uniqueClientId);

    if (mqttClient.connect(uniqueClientId.c_str())) {
      DebugLog.Write(LogLevel::Info, LogCategory::MQTT, "connectToMQTT", "Connected to AWS IoT Core successfully");
#else
    // Generate unique client ID using MAC address for public broker
    String macAddress = WiFi.macAddress();
    macAddress.replace(":", "");
    String uniqueClientId = String(mqtt_client_id) + "_" + macAddress;
    DebugLog.Write(LogLevel::Info, LogCategory::MQTT, "connectToMQTT", "Using client ID: " + uniqueClientId);

    if (mqttClient.connect(uniqueClientId.c_str(), mqtt_username, mqtt_password)) {
      DebugLog.Write(LogLevel::Info, LogCategory::MQTT, "connectToMQTT", "MQTT connected successfully");
#endif

      mqttClient.subscribe(topic_smartwheels);
      DebugLog.Write(LogLevel::Info, LogCategory::MQTT, "connectToMQTT", "Subscribed to: " + String(topic_smartwheels));

    } else {
      DebugLog.Write(LogLevel::Error, LogCategory::MQTT, "connectToMQTT", "MQTT connection failed, rc=" + String(mqttClient.state()));
    }
  }
}

void MQTTHandler::handleConnection() {
  static bool wasConnected = false;

  if (wifiHandler.isConnected()) {
    if (!mqttClient.connected()) {
      if (wasConnected) {
        DebugLog.Write(LogLevel::Warn, LogCategory::MQTT, "handleConnection", "MQTT connection lost, attempting to reconnect...");
        wasConnected = false;
      }
      connectToMQTT();
    } else {
      if (!wasConnected) {
        wasConnected = true;
      }
      mqttClient.loop();
    }
  } else if (wasConnected) {
    DebugLog.Write(LogLevel::Warn, LogCategory::MQTT, "handleConnection", "WiFi disconnected, MQTT connection lost");
    wasConnected = false;
  }
}

void MQTTHandler::loop() {
  mqttClient.loop();
}

bool MQTTHandler::isConnected() {
  return mqttClient.connected();
}

void MQTTHandler::publish(const char* topic, const char* message) {
  if (mqttClient.connected()) {
    mqttClient.publish(topic, message);
  }
}

String MQTTHandler::getMQTTStateText(int state) {
  switch(state) {
    case -4: return "CONNECTION_TIMEOUT";
    case -3: return "CONNECTION_LOST";
    case -2: return "CONNECT_FAILED";
    case -1: return "DISCONNECTED";
    case 0: return "CONNECTED";
    case 1: return "CONNECT_BAD_PROTOCOL";
    case 2: return "CONNECT_BAD_CLIENT_ID";
    case 3: return "CONNECT_UNAVAILABLE";
    case 4: return "CONNECT_BAD_CREDENTIALS";
    case 5: return "CONNECT_UNAUTHORIZED";
    default: return "UNKNOWN_STATE_" + String(state);
  }
}

void MQTTHandler::mqttCallback(char* topic, byte* payload, unsigned int length) {
  DebugLog.Write(LogLevel::Info, LogCategory::MQTT, "mqttCallback", "MQTT Message received on topic: " + String(topic));
  DebugLog.Write(LogLevel::Info, LogCategory::MQTT, "mqttCallback", "Message length: " + String(length) + " bytes");

  if (length > 1000) {
    DebugLog.Write(LogLevel::Info, LogCategory::MQTT, "mqttCallback", "Large message detected, processing in chunks");
    String preview = "";
    for (int i = 0; i < min(100, (int)length); i++) {
      preview += (char)payload[i];
    }
    DebugLog.Write(LogLevel::Debug, LogCategory::MQTT, "mqttCallback", "Message preview: " + preview + "...");
  } else {
    String message = "";
    for (int i = 0; i < length; i++) {
      message += (char)payload[i];
    }
    DebugLog.Write(LogLevel::Debug, LogCategory::MQTT, "mqttCallback", "Message: " + message.substring(0, 100) + (message.length() > 100 ? "..." : ""));
  }

#if USE_AWS_IOT
  // AWS IoT topic pattern: sdv/vehicles/sdv-vehicle-001/*
  if (String(topic).startsWith("sdv/vehicles/")) {
    if (String(topic).indexOf("fota") >= 0) {
      DebugLog.Write(LogLevel::Info, LogCategory::MQTT, "mqttCallback", "AWS FOTA message received");
      fotaHandler.handleMessage(topic, payload, length);
      ledHandler.flashBlue(1, 100);
    } else if (String(topic).indexOf("data") >= 0) {
      DebugLog.Write(LogLevel::Info, LogCategory::MQTT, "mqttCallback", "AWS Data message received");
    } else {
      DebugLog.Write(LogLevel::Info, LogCategory::MQTT, "mqttCallback", "AWS message received");
      ledHandler.flashGreen(1, 100);
    }
  }
#else
  // Public MQTT broker topic pattern: SmartWheels-Ancit/*
  if (String(topic).startsWith("SmartWheels-Ancit/")) {
    if (String(topic).indexOf("fota") >= 0) {
      DebugLog.Write(LogLevel::Info, LogCategory::MQTT, "mqttCallback", "FOTA message received");
      fotaHandler.handleMessage(topic, payload, length);
      ledHandler.flashBlue(1, 100);
    } else if (String(topic).indexOf("data") >= 0) {
      DebugLog.Write(LogLevel::Info, LogCategory::MQTT, "mqttCallback", "Data message received");
    } else {
      DebugLog.Write(LogLevel::Info, LogCategory::MQTT, "mqttCallback", "SmartWheels message received");
      ledHandler.flashGreen(1, 100);
    }
  }
#endif
}