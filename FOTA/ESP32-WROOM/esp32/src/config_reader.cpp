#include <Arduino.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>
#include <config_reader.h>
#include <BT_LOGGER.h>
#include <main.h>
#include <string.h>

// JSON Documents
JsonDocument device_doc;
JsonDocument wifi_doc;
JsonDocument devComm_doc;
JsonDocument registration_doc;

// Generic helper to load JSON from SPIFFS
static void read_json_file(const char *path, JsonDocument &doc, const char *tag,
                           bool printContent = false) {
  File file = SPIFFS.open(path, "r");
  if (!file) {
    g_Logger.WriteImmediate(LogLevel::Fatal, LogCategory::SPIFFS, tag,
                     "Failed to open %s..!!!", path);
    return;
  }

  g_Logger.WriteImmediate(LogLevel::Debug, LogCategory::SPIFFS, tag, "Read file %s", path);

  if (printContent) {
    while (file.available()) {
      Serial.write(file.read());
    }
    file.seek(0);
  }

  DeserializationError err = deserializeJson(doc, file);
  if (err) {
    g_Logger.WriteImmediate(LogLevel::Error, LogCategory::SPIFFS, tag,
                     "Failed to parse JSON: %s", err.c_str());
  }
  file.close();
}

void read_device_file(void) {
  read_json_file("/config/device.json", device_doc, "read_device_file", false);
}

void read_wifi_file(void) {
  read_json_file("/config/wifi.json", wifi_doc, "read_wifi_file");
}


void read_devComm_file(void) {
  read_json_file("/config/devComm.json", devComm_doc, "read_devComm_file",
                 false);
}

void read_registration_file(void) {
  read_json_file("/config/registration.json", registration_doc,
                 "read_registration_file");
}

