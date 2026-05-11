/**
 * ble_app.h - Application-specific BLE Handler
 *
 * Project-specific BLE functionality:
 * - LED control
 * - Sensor data
 * - Custom commands
 *
 * Author: Narayana Swamy
 * Date: October 2025
 */

#ifndef BLE_APP_H
#define BLE_APP_H

#include "ancit_device.h"

#if BLE_ENABLED

#include <Arduino.h>
#include <ArduinoJson.h>
#include "ble_handler.h"
// #include "led_controller.h"

class BLEApp : public BLEHandlerCallbacks {
private:
    BLEHandler* pBLEHandler;

public:
    BLEApp(BLEHandler* bleHandler);
    ~BLEApp();

    // Implement callbacks from BLEHandler
    void onCustomCommand(JsonDocument& doc, BLEHandler* handler) override;
    void onGetCategories(BLEHandler* handler) override;

    // App-specific methods
    void sendSensorData(float temperature, float humidity, int rssi);
};

#endif // BLE_ENABLED

#endif // BLE_APP_H
