#ifndef _HEADER_ANCIT_MQTT_CLIENT_H_
#define _HEADER_ANCIT_MQTT_CLIENT_H_

#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

// ===== MQTT buffer (must be large enough for FOTA chunk messages) =====
#define MQTT_BUFFER_SIZE 8192

// QoS levels
#define MQTT_QOS_0 0
#define MQTT_QOS_1 1
#define MQTT_QOS_2 2

// Retain flag
#define MQTT_RETAIN    true
#define MQTT_NO_RETAIN false

struct MqttConfig {
    bool    enabled;
    String  server;
    bool    encrypt;
    String  ca_cert;
    String  device_cert;
    String  private_key;
    uint16_t port;
    uint16_t keepAlive;
    String  username;
    String  password;
    uint8_t qos;
    String  clientId;
    uint32_t publish_interval;
    String  uri_full;
};

extern WiFiClient       wifiClient;
extern WiFiClientSecure wifiClientSecure;
extern PubSubClient     mqttClient;
extern MqttConfig       mqtt_config;
extern bool             useEmbeddedAWSCerts;

// Core MQTT API
void MqttClient_Init(const MqttConfig& config);
void MqttClient_SetClientId(void);
void MqttClient_CreateTask(void);
void MqttClient_Stop(void);
void MqttClient_Restart(void);
void MqttClient_LoadFromJson(const JsonDocument& doc);
void MqttClient_ApplicationTask(void* pvParams);

bool MqttClient_Publish(const char* topic, const char* payload, bool retain = false);
bool MqttClient_Subscribe(const char* topic);
bool MqttClient_IsConnected(void);
void MqttClient_Loop(void);

#endif  // _HEADER_ANCIT_MQTT_CLIENT_H_
