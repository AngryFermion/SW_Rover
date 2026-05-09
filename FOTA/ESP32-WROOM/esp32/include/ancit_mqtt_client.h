#ifndef _HEADER_ANCIT_MQTT_CLIENT_H_
#define _HEADER_ANCIT_MQTT_CLIENT_H_

#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

#include <PubSubClient.h>

/* MQTT Configuration */
#define MQTT_BUFFER_SIZE 8192

/* mqtt server/host */
#define MQTT_QOS_0 0  // fire and forget
#define MQTT_QOS_1 1  // publish with acknoledgement,
                      // no ack retry, with multiple acks
#define MQTT_QOS_2 2  // two times handshake, Guaranteed one time delivary

/* retain flag if client disconnect from server */
#define MQTT_RETAIN true
#define MQTT_NO_RETAIN false

struct MqttConfig {
  bool enabled;
  String server;
  bool encrypt;  // Use TLS/SSL for connection
  String ca_cert;
  String device_cert;  // Device certificate for mutual TLS (AWS IoT)
  String private_key;  // Private key for mutual TLS (AWS IoT)
  uint16_t port;
  uint16_t keepAlive;
  String username;
  String password;
  uint8_t qos;      // Added for onTopic subscription level
  String clientId;  // Client ID for MQTT connection
  uint32_t publish_interval;  // Interval for publishing sensor values
  String uri_full;
};

extern WiFiClient wifiClient;
extern WiFiClientSecure wifiClientSecure;
extern PubSubClient mqttClient;
extern MqttConfig mqtt_config;
extern bool useEmbeddedAWSCerts;  // Flag to indicate if using AWS IoT certificates

void MqttClient_Init(const MqttConfig& config);
void MqttClient_SetClientId(void);
void MqttClient_CreateTask(void);
void MqttClient_Stop(void);
void MqttClient_Restart(void);
void MqttClient_LoadFromJson(const JsonDocument& doc);
void MqttClient_ApplicationTask(void* pvParams);

// New PubSubClient specific functions
bool MqttClient_Publish(const char* topic, const char* payload, bool retain = false);
bool MqttClient_Subscribe(const char* topic);
bool MqttClient_IsConnected();
void MqttClient_Loop();

#endif  //_HEADER_ANCIT_MQTT_CLIENT_H_
