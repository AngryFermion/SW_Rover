/**
 * ble_config_server.h - BLE Configuration Server for File/Config Operations
 *
 * Handles data transmission and file operations over BLE
 * Separated from core BLE handler for modularity
 *
 * Author: Narayana Swamy
 * Date: October 2025
 */

#ifndef BLE_CONFIG_SERVER_H
#define BLE_CONFIG_SERVER_H

#include "ancit_device.h"

#if BLE_ENABLED

#include <Arduino.h>
#include <NimBLEDevice.h>
#include <ArduinoJson.h>
#include <BT_LOGGER.h>

class BLEConfigServer {
private:
    BT_LOGGER* pLogger;
    NimBLECharacteristic* pTxCharacteristic;
    bool* pDeviceConnected;

public:
    BLEConfigServer(BT_LOGGER* logger, NimBLECharacteristic* txChar, bool* connected);

    // Data transmission methods
    void sendLogCategories();

    // Generic config file handlers
    void sendConfigFile(const char* filename, const char* responseType);
    void saveConfigFile(const char* filename, const char* responseType, JsonDocument& doc);

    // Convenience wrappers for specific configs
    void sendWifiConfig();
    void saveWifiConfig(JsonDocument& doc);
    void sendDeviceConfig();
    void saveDeviceConfig(JsonDocument& doc);
};

#endif // BLE_ENABLED

#endif // BLE_CONFIG_SERVER_H
