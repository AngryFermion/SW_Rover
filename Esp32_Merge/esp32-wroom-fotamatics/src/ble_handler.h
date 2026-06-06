/**
 * ble_handler.h - Generic BLE Communication Handler
 *
 * Reusable BLE module for multiple projects
 * Handles:
 * - Server setup and advertising
 * - Client connections
 * - WiFi credential management
 * - Debug/terminal logging over BLE
 * - Log category transmission
 *
 * Author: Narayana Swamy
 * Date: October 2025
 */

#ifndef BLE_HANDLER_H
#define BLE_HANDLER_H

#include "ancit_device.h"

#if BLE_ENABLED

#include <Arduino.h>
#include <NimBLEDevice.h>
#include <ArduinoJson.h>
#include <BT_LOGGER.h>

// Forward declaration
class BLEConfigServer;

// BLE status structure to hold all event flags and status
struct BLEStatus {
    // Event flags
    bool eventSendWifiConfig;
    bool eventSaveWifiConfig;
    bool eventSendDeviceConfig;
    bool eventSaveDeviceConfig;
    bool eventSendLogCategories;
    bool eventTestWifiConnection;
    bool eventWifiScan;
    bool eventSendConnectionInfo;
    bool eventRestartDevice;
    // Add more events here as needed

    // Status flags
    bool deviceConnected;
    bool taskRunning;
    bool categoriesSent;
    bool wifiTestInProgress;

    BLEStatus()
        : eventSendWifiConfig(false)
        , eventSaveWifiConfig(false)
        , eventSendDeviceConfig(false)
        , eventSaveDeviceConfig(false)
        , eventSendLogCategories(false)
        , eventTestWifiConnection(false)
        , eventWifiScan(false)
        , eventSendConnectionInfo(false)
        , eventRestartDevice(false)
        , deviceConnected(false)
        , taskRunning(false)
        , categoriesSent(false)
        , wifiTestInProgress(false)
    {}
};

// BLE UUIDs
#define BLE_SERVICE_UUID        "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define BLE_RX_UUID             "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"  // App writes to this
#define BLE_TX_UUID             "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"  // ESP32 notifies this

// Device name prefix
#define BLE_DEVICE_NAME_PREFIX  "FOTA_"

// Forward declaration
class BLEHandler;

// Callback interface for application-specific commands
class BLEHandlerCallbacks {
public:
    virtual ~BLEHandlerCallbacks() {}

    // Called when application-specific command is received
    virtual void onCustomCommand(JsonDocument& doc, BLEHandler* handler) = 0;

    // Called when get_categories request is received (app can override)
    virtual void onGetCategories(BLEHandler* handler) {}
};

// Log output callback for BLE transmission
class BLELogCallback : public LogOutputCallback {
private:
    BLEHandler* pHandler;

public:
    BLELogCallback(BLEHandler* handler) : pHandler(handler) {}
    void onLogOutput(const char* formattedLog) override;
};

class BLEHandler {
private:
    NimBLEServer* pServer;
    NimBLECharacteristic* pTxCharacteristic;
    NimBLECharacteristic* pRxCharacteristic;

    BT_LOGGER* pLogger;
    BLEHandlerCallbacks* pCallbacks;
    BLELogCallback* pLogCallback;
    BLEConfigServer* pConfigServer;

    // Task control
    uint32_t connectionTime;
    uint32_t lastActivityTime;

    // Timeout constants (in milliseconds)
    static const uint32_t TIMEOUT_UNPAIRED_MS = (1*60000);      // 1 minute when unpaired
    static const uint32_t TIMEOUT_PAIRED_MS = (4*60000);       // 4 minutes when paired with no activity

    // BLE status structure
    BLEStatus* pBleStatus;

    // Receive buffer for chunked data
    std::string rxBuffer;
    uint32_t lastRxTime;
    static const uint32_t RX_TIMEOUT_MS = 1000;  // Clear buffer after 1 second of inactivity
    static const size_t MAX_RX_BUFFER_SIZE = 4096;  // Maximum buffer size

    // WiFi test state machine
    uint32_t wifiTestStartTime;
    static const uint32_t WIFI_TEST_TIMEOUT_MS = 35000;  // 35 seconds

    // Timeout state machine
    bool timeoutTriggered;
    uint32_t timeoutNotificationSentTime;

    // Internal methods
    String getMacAddress();
    void handleReceivedData(const std::string& value);
    void initializeBLE();
    void handleTestWifiConnection();
    void checkWifiTestProgress();
    void handleWifiScan();
    void sendConnectionInfo();

public:
    BLEHandler(BT_LOGGER* logger);
    ~BLEHandler();

    // Set application callbacks
    void setCallbacks(BLEHandlerCallbacks* callbacks);

    // Task control methods
    void begin();           // Start BLE
    void stop();            // Stop BLE completely
    void suspend();         // Suspend BLE task temporarily
    void resume();          // Resume suspended BLE task
    bool isTaskRunning();   // Check if task is running

    // Process timed tasks (call from loop)
    void processTimedTasks();

    // Check if device is connected
    bool isConnected();

    // Send data to app
    void sendData(const char* jsonString);
    void sendData(const uint8_t* data, size_t length);

    // Access to TX characteristic for app
    NimBLECharacteristic* getTxCharacteristic() { return pTxCharacteristic; }

    // Server callbacks (friend classes)
    friend class CustomServerCallbacks;
    friend class CustomRxCallbacks;
};

#endif // BLE_ENABLED

#endif // BLE_HANDLER_H
