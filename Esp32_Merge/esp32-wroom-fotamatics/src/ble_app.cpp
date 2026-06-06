/**
 * ble_app.cpp - Application-specific BLE Handler Implementation
 *
 * Project-specific BLE functionality
 */

#include "ble_app.h"
#include "ancit_device.h"

#if BLE_ENABLED

// Constructor
BLEApp::BLEApp(BLEHandler* bleHandler)
    : pBLEHandler(bleHandler)
{
    // Register ourselves as callbacks for the generic handler
    pBLEHandler->setCallbacks(this);
}

// Destructor
BLEApp::~BLEApp() {
}

// Handle custom application-specific commands
void BLEApp::onCustomCommand(JsonDocument& doc, BLEHandler* handler) {
    // Add custom commands here as needed
    // Example:
    // if (doc.containsKey("motor_speed")) {
    //     setMotorSpeed(doc["motor_speed"]);
    // }
}

// Override get_categories if needed (optional)
void BLEApp::onGetCategories(BLEHandler* handler) {
    // Use default behavior (send categories immediately)
    // Override this if you need custom behavior
}

// Send sensor data to app
void BLEApp::sendSensorData(float temperature, float humidity, int rssi) {
    if (!pBLEHandler->isConnected()) return;

    JsonDocument doc;
    doc["type"] = "sensor";
    doc["temperature"] = temperature;
    doc["humidity"] = humidity;
    doc["rssi"] = rssi;
    doc["uptime"] = millis() / 1000;

    String jsonString;
    serializeJson(doc, jsonString);

    pBLEHandler->sendData(jsonString.c_str());
}

#endif // BLE_ENABLED
