#ifndef WIFI_HANDLER_H
#define WIFI_HANDLER_H

#include <Arduino.h>
#include <WiFi.h>
#include "config.h"

class WiFiHandler {
public:
  WiFiHandler();
  void init();
  void begin();
  String getWiFiStatusText(wl_status_t status);
  bool isConnected();
  String getIPAddress();
  static void onWiFiEvent(WiFiEvent_t event);
};

extern WiFiHandler wifiHandler;

#endif