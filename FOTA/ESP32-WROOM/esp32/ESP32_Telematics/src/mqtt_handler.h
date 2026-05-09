#ifndef MQTT_HANDLER_H
#define MQTT_HANDLER_H

#include <Arduino.h>
#include <WiFi.h>

#include <PubSubClient.h>
#include "config.h"
#if USE_AWS_IOT
  #include <WiFiClientSecure.h>
#endif
class MQTTHandler {
private:
#if USE_AWS_IOT
  WiFiClientSecure espClient;
#else
  WiFiClient espClient;
#endif
  PubSubClient mqttClient;
  unsigned long lastConnectionAttempt = 0;
  static const unsigned long CONNECTION_RETRY_DELAY = 5000; // 5 seconds

public:
  MQTTHandler();
  void init();
  void connectToMQTT();
  void handleConnection();
  void loop();
  bool isConnected();
  void publish(const char* topic, const char* message);
  String getMQTTStateText(int state);
  static void mqttCallback(char* topic, byte* payload, unsigned int length);
};

extern MQTTHandler mqttHandler;

#endif