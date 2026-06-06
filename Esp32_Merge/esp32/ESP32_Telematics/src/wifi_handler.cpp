#include "wifi_handler.h"
#include "led_handler.h"
#include "mqtt_handler.h"
#include "utils.h"

WiFiHandler wifiHandler;

WiFiHandler::WiFiHandler() {
}

void WiFiHandler::init() {
  WiFi.onEvent(onWiFiEvent);
}

void WiFiHandler::begin() {
  WiFi.begin(ssid, password);
  DebugLog.Write(LogLevel::Info, LogCategory::WIFI, "begin", "Starting WiFi connection to: " + String(ssid));
}

String WiFiHandler::getWiFiStatusText(wl_status_t status) {
  switch(status) {
    case WL_IDLE_STATUS: return "IDLE";
    case WL_NO_SSID_AVAIL: return "NO_SSID_AVAILABLE";
    case WL_SCAN_COMPLETED: return "SCAN_COMPLETED";
    case WL_CONNECTED: return "CONNECTED";
    case WL_CONNECT_FAILED: return "CONNECT_FAILED";
    case WL_CONNECTION_LOST: return "CONNECTION_LOST";
    case WL_DISCONNECTED: return "DISCONNECTED";
    default: return "UNKNOWN";
  }
}

bool WiFiHandler::isConnected() {
  return WiFi.status() == WL_CONNECTED;
}

String WiFiHandler::getIPAddress() {
  return WiFi.localIP().toString();
}

void WiFiHandler::onWiFiEvent(WiFiEvent_t event) {
  switch(event) {
    case ARDUINO_EVENT_WIFI_STA_CONNECTED:
      DebugLog.Write(LogLevel::Info, LogCategory::WIFI, "handleEvent", "WiFi Connected to Station");
      ledHandler.setState(WIFI_CONNECTED);
      break;

    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      DebugLog.Write(LogLevel::Info, LogCategory::WIFI, "handleEvent", "WiFi Got IP: " + WiFi.localIP().toString());
      ledHandler.setState(WIFI_GOT_IP);
      if (!mqttHandler.isConnected()) {
        mqttHandler.connectToMQTT();
      }
      break;

    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
      DebugLog.Write(LogLevel::Warn, LogCategory::WIFI, "handleEvent", "WiFi Disconnected");
      ledHandler.setState(WIFI_DISCONNECTED);
      break;

    default:
      break;
  }
}