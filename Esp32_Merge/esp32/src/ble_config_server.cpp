/**
 * ble_config_server.cpp - BLE Configuration Server Implementation
 *
 * Handles data transmission and file operations over BLE
 */

#include "ble_config_server.h"
#include <SPIFFS.h>
#include <config_reader.h>

#if BLE_ENABLED

// Constructor
BLEConfigServer::BLEConfigServer(BT_LOGGER* logger, NimBLECharacteristic* txChar, bool* connected)
    : pLogger(logger)
    , pTxCharacteristic(txChar)
    , pDeviceConnected(connected)
{
}

// Send log categories to BLE client
void BLEConfigServer::sendLogCategories() {
    if (!(*pDeviceConnected)) {
        return;
    }

    // Use static buffer to avoid dynamic allocation
    static char buffer[600];

    // Get categories JSON from logger
    String categoriesJson = pLogger->GetAllCategoriesJson();
    const char* catJson = categoriesJson.c_str();

    // Build JSON message directly in buffer
    int len = snprintf(buffer, sizeof(buffer),
                      "{\"type\":\"log_categories\",\"categories\":%s}",
                      catJson);

    if (len >= sizeof(buffer)) {
        pLogger->Write(LogLevel::Error, LogCategory::BLE, "sendLogCategories", "Buffer too small");
        return;
    }

    pLogger->Write(LogLevel::Debug, LogCategory::BLE, "sendLogCategories", "Sending %d bytes in chunks", len);

    // Send in smaller chunks
    const int chunkSize = 240;

    for (int i = 0; i < len; i += chunkSize) {
        int chunkLen = min(chunkSize, len - i);
        pTxCharacteristic->setValue((uint8_t*)(buffer + i), chunkLen);
        pTxCharacteristic->notify();
        pLogger->Write(LogLevel::Debug, LogCategory::BLE, "sendLogCategories", "Sent chunk %d (%d bytes)", (i / chunkSize) + 1, chunkLen);
        delay(100); // Delay between chunks
    }

    pLogger->Write(LogLevel::Info, LogCategory::BLE, "sendLogCategories", "Categories sent successfully");
}

// Generic method to send any config file
void BLEConfigServer::sendConfigFile(const char* filename, const char* responseType) {
    if (!(*pDeviceConnected)) {
        return;
    }

    // Read config file
    File file = SPIFFS.open(filename, "r");
    if (!file) {
        pLogger->Write(LogLevel::Error, LogCategory::BLE, "sendConfigFile", "Failed to open %s", filename);

        // Build error response
        String errorMsg = "{\"type\":\"";
        errorMsg += responseType;
        errorMsg += "\",\"error\":\"File not found\"}";

        pTxCharacteristic->setValue((uint8_t*)errorMsg.c_str(), errorMsg.length());
        pTxCharacteristic->notify();
        return;
    }

    // Read file content directly into String (heap allocated)
    String fileContent = file.readString();
    file.close();

    // Build response using String (heap allocated, not stack)
    String response = "{\"type\":\"";
    response += responseType;
    response += "\",\"data\":";
    response += fileContent;
    response += "}";

    int len = response.length();
    pLogger->Write(LogLevel::Info, LogCategory::BLE, "sendConfigFile", "Sending %s: %d bytes", filename, len);

    // Send in chunks directly from String
    const int chunkSize = 240;
    for (int i = 0; i < len; i += chunkSize) {
        int chunkLen = min(chunkSize, len - i);
        pTxCharacteristic->setValue((uint8_t*)(response.c_str() + i), chunkLen);
        pTxCharacteristic->notify();
        delay(100);
    }

    pLogger->Write(LogLevel::Info, LogCategory::BLE, "sendConfigFile", "%s sent successfully", filename);
}

// Generic method to save any config file
void BLEConfigServer::saveConfigFile(const char* filename, const char* responseType, JsonDocument& doc) {
    if (!doc["config"].is<JsonObject>()) {
        pLogger->Write(LogLevel::Error, LogCategory::BLE, "saveConfigFile", "No config data in request");
        return;
    }

    // Write to file
    File file = SPIFFS.open(filename, "w");
    if (!file) {
        pLogger->Write(LogLevel::Error, LogCategory::BLE, "saveConfigFile", "Failed to open %s for writing", filename);

        // Build error response
        String errorMsg = "{\"type\":\"save_";
        errorMsg += responseType;
        errorMsg += "\",\"status\":\"error\",\"message\":\"File write failed\"}";

        pTxCharacteristic->setValue((uint8_t*)errorMsg.c_str(), errorMsg.length());
        pTxCharacteristic->notify();
        return;
    }

    // Serialize config to file
    serializeJson(doc["config"], file);
    file.close();

    pLogger->Write(LogLevel::Info, LogCategory::BLE, "saveConfigFile", "%s saved successfully", filename);

    // Build success response
    String successMsg = "{\"type\":\"save_";
    successMsg += responseType;
    successMsg += "\",\"status\":\"ok\",\"message\":\"Configuration saved\"}";

    pTxCharacteristic->setValue((uint8_t*)successMsg.c_str(), successMsg.length());
    pTxCharacteristic->notify();
}

// Convenience wrapper for WiFi config
void BLEConfigServer::sendWifiConfig() {
    sendConfigFile("/config/wifi.json", "wifi_config");
}

// Convenience wrapper for WiFi config save
void BLEConfigServer::saveWifiConfig(JsonDocument& doc) {
    saveConfigFile("/config/wifi.json", "wifi_config", doc);

    // Update the global wifi_doc to keep in-memory config synchronized
    if (doc["config"].is<JsonObject>()) {
        wifi_doc.clear();
        wifi_doc.set(doc["config"]);
        pLogger->Write(LogLevel::Info, LogCategory::BLE, "saveWifiConfig", "Global wifi_doc updated");
    }
}

// Convenience wrapper for Device config
void BLEConfigServer::sendDeviceConfig() {
    sendConfigFile("/config/devComm.json", "device_config");
}

// Convenience wrapper for Device config save
void BLEConfigServer::saveDeviceConfig(JsonDocument& doc) {
    saveConfigFile("/config/devComm.json", "device_config", doc);

    // Update the global devComm_doc to keep in-memory config synchronized
    if (doc["config"].is<JsonObject>()) {
        devComm_doc.clear();
        devComm_doc.set(doc["config"]);
        pLogger->Write(LogLevel::Info, LogCategory::BLE, "saveDeviceConfig", "Global devComm_doc updated");
    }
}

#endif // BLE_ENABLED
