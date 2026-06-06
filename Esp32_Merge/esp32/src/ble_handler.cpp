/**
 * ble_handler.cpp - Generic BLE Communication Handler
 *
 * Reusable BLE module for multiple projects
 */

#include "ble_handler.h"
#include "ble_config_server.h"
#include "device_state_manager.h"
#include "config_reader.h"

#if BLE_ENABLED

// Global config documents (to avoid stack issues in callbacks)
static JsonDocument g_wifiConfigDoc;
static JsonDocument g_deviceConfigDoc;

// Server Callbacks
class CustomServerCallbacks: public NimBLEServerCallbacks {
private:
    BLEHandler* handler;

public:
    CustomServerCallbacks(BLEHandler* h) : handler(h) {}

    void onConnect(NimBLEServer* pServer) {
        handler->pBleStatus->deviceConnected = true;
        handler->pBleStatus->categoriesSent = false;

        // Reset timeout counters and state on connection
        handler->connectionTime = millis();
        handler->lastActivityTime = millis();
        handler->timeoutTriggered = false;
        handler->timeoutNotificationSentTime = 0;

        handler->pLogger->WriteImmediate(LogLevel::Info, LogCategory::BLE, "onConnect",
                                        "Client connected - timeout counters reset");

        // Update device state to BT_STATE_PAIRED
        DeviceStateManager_SetBluetoothState(BT_STATE_PAIRED);
    }

    void onDisconnect(NimBLEServer* pServer) {
        handler->pBleStatus->deviceConnected = false;
        handler->pLogger->WriteImmediate(LogLevel::Info, LogCategory::BLE, "onDisconnect", "Client disconnected");

        // Update device state back to BT_STATE_UNPAIRED
        DeviceStateManager_SetBluetoothState(BT_STATE_UNPAIRED);

        // Restart advertising and reset unpaired timeout counter
        NimBLEDevice::getAdvertising()->start();
        handler->connectionTime = millis();  // Reset unpaired timeout timer
        handler->pLogger->WriteImmediate(LogLevel::Info, LogCategory::BLE, "onDisconnect",
                                        "Advertising restarted - unpaired timeout reset");
    }
};

// RX Characteristic Callbacks
class CustomRxCallbacks: public NimBLECharacteristicCallbacks {
private:
    BLEHandler* handler;

public:
    CustomRxCallbacks(BLEHandler* h) : handler(h) {}

    void onWrite(NimBLECharacteristic* pCharacteristic) {
        std::string value = pCharacteristic->getValue();
        if (value.length() > 0) {
            handler->lastActivityTime = millis();  // Update activity timer on any write
            handler->handleReceivedData(value);
        }
    }
};

// BLELogCallback implementation
void BLELogCallback::onLogOutput(const char* formattedLog) {
    // DISABLED: BLE logging removed - logs are not sent over BLE
    // This prevents interference with actual data messages
    (void)formattedLog; // Suppress unused parameter warning
}

// Constructor
BLEHandler::BLEHandler(BT_LOGGER* logger)
    : pServer(nullptr)
    , pTxCharacteristic(nullptr)
    , pRxCharacteristic(nullptr)
    , pLogger(logger)
    , pCallbacks(nullptr)
    , pLogCallback(nullptr)
    , pConfigServer(nullptr)
    , connectionTime(0)
    , lastActivityTime(0)
    , pBleStatus(nullptr)
    , rxBuffer("")
    , lastRxTime(0)
    , wifiTestStartTime(0)
    , timeoutTriggered(false)
    , timeoutNotificationSentTime(0)
{
    // Create BLE status structure
    pBleStatus = new BLEStatus();
    if (!pBleStatus) {
        Serial.println("FATAL: Failed to allocate BLEStatus");
        while(1); // Halt - cannot continue without status
    }

    // Log callback disabled - logs not sent over BLE
    pLogCallback = nullptr;
}

// Destructor
BLEHandler::~BLEHandler() {
    // Log callback not used
    if (pConfigServer) {
        delete pConfigServer;
        pConfigServer = nullptr;
    }
    if (pBleStatus) {
        delete pBleStatus;
        pBleStatus = nullptr;
    }
    stop();
}

// Set application callbacks
void BLEHandler::setCallbacks(BLEHandlerCallbacks* callbacks) {
    pCallbacks = callbacks;
}

// Get MAC address
String BLEHandler::getMacAddress() {
    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    char macStr[13];
    sprintf(macStr, "%02X%02X%02X%02X%02X%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return String(macStr);
}

// Initialize BLE
void BLEHandler::initializeBLE() {
    pLogger->WriteImmediate(LogLevel::Info, LogCategory::BLE, "initializeBLE", "Starting BLE...");

    // Get device name with MAC
    String deviceName = String(BLE_DEVICE_NAME_PREFIX) + getMacAddress();
    pLogger->WriteImmediate(LogLevel::Info, LogCategory::BLE, "initializeBLE", "Device name: %s", deviceName.c_str());

    // Initialize BLE
    NimBLEDevice::init(deviceName.c_str());

    // Create BLE Server
    pServer = NimBLEDevice::createServer();
    pServer->setCallbacks(new CustomServerCallbacks(this));

    // Create BLE Service
    NimBLEService* pService = pServer->createService(BLE_SERVICE_UUID);

    // Create TX Characteristic (ESP32 → App)
    pTxCharacteristic = pService->createCharacteristic(
        BLE_TX_UUID,
        NIMBLE_PROPERTY::NOTIFY
    );

    // Create RX Characteristic (App → ESP32)
    pRxCharacteristic = pService->createCharacteristic(
        BLE_RX_UUID,
        NIMBLE_PROPERTY::WRITE
    );
    pRxCharacteristic->setCallbacks(new CustomRxCallbacks(this));

    // Start the service
    pService->start();

    // Start advertising
    NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(BLE_SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->start();

    // Create config server after BLE is initialized
    pConfigServer = new BLEConfigServer(pLogger, pTxCharacteristic, &(pBleStatus->deviceConnected));

    pLogger->WriteImmediate(LogLevel::Info, LogCategory::BLE, "initializeBLE", "BLE device is ready and advertising!");
    pLogger->WriteImmediate(LogLevel::Info, LogCategory::BLE, "initializeBLE", "Waiting for client connection...");
}

// Process timed tasks - called from loop()
void BLEHandler::processTimedTasks() {
    if (!pBleStatus->taskRunning) {
        return;
    }

    // Clear stale RX buffer if timeout expired
    if (rxBuffer.length() > 0 && (millis() - lastRxTime) > RX_TIMEOUT_MS) {
        Serial.println("RX buffer timeout - clearing stale data");
        rxBuffer.clear();
    }

    // Check BLE timeout - state machine approach to avoid blocking
    if (pBleStatus->deviceConnected) {
        // Get current Bluetooth state
        BluetoothModeState_t btState = DeviceStateManager_GetBluetoothState();
        uint32_t currentTime = millis();
        uint32_t elapsedTime = currentTime - lastActivityTime;

        // Determine timeout based on state
        uint32_t timeoutMs = (btState == BT_STATE_PAIRED) ? TIMEOUT_PAIRED_MS : TIMEOUT_UNPAIRED_MS;

        if (!timeoutTriggered && elapsedTime >= timeoutMs) {
            // Step 1: Send timeout notification
            const char* timeoutMsg = "{\"type\":\"ble_timeout\",\"message\":\"BLE session timed out due to inactivity\"}";
            if (pTxCharacteristic) {
                pTxCharacteristic->setValue((uint8_t*)timeoutMsg, strlen(timeoutMsg));
                pTxCharacteristic->notify();
            }

            pLogger->Write(LogLevel::Info, LogCategory::BLE, "processTimedTasks",
                          "BLE timeout - %s state, %lu ms elapsed",
                          btState == BT_STATE_PAIRED ? "PAIRED" : "UNPAIRED",
                          elapsedTime);

            timeoutTriggered = true;
            timeoutNotificationSentTime = millis();
        }

        // Step 2: After notification sent, disconnect (non-blocking state machine)
        if (timeoutTriggered && timeoutNotificationSentTime > 0 && (millis() - timeoutNotificationSentTime) >= 100) {
            // Disconnect after 100ms delay for notification to send
            if (pServer && pServer->getConnectedCount() > 0) {
                pServer->disconnect(pServer->getPeerInfo(0).getConnHandle());
            }

            // Mark notification time as 0 to indicate we've disconnected
            timeoutNotificationSentTime = 0;
        }

        // Step 3: After disconnect, restore mode
        if (timeoutTriggered && timeoutNotificationSentTime == 0 && !pBleStatus->deviceConnected) {
            // Already disconnected, restore mode
            DeviceMode_t restoreMode = DeviceStateManager_GetBluetoothRestoreMode();
            pLogger->Write(LogLevel::Info, LogCategory::BLE, "processTimedTasks",
                          "Timeout complete - restoring to mode %d", restoreMode);

            DeviceStateManager_SetMode(restoreMode);
            timeoutTriggered = false;  // Reset for next timeout cycle
        }
    } else if (!pBleStatus->deviceConnected && pBleStatus->taskRunning) {
        // Check timeout when advertising (unpaired state)
        BluetoothModeState_t btState = DeviceStateManager_GetBluetoothState();
        if (btState == BT_STATE_UNPAIRED) {
            uint32_t currentTime = millis();
            uint32_t elapsedTime = currentTime - connectionTime;  // Use connectionTime as BLE start time

            if (elapsedTime >= TIMEOUT_UNPAIRED_MS) {
                pLogger->Write(LogLevel::Info, LogCategory::BLE, "processTimedTasks",
                              "BLE advertising timeout - %lu ms elapsed",
                              elapsedTime);

                // Get restore mode and trigger mode change
                DeviceMode_t restoreMode = DeviceStateManager_GetBluetoothRestoreMode();
                pLogger->Write(LogLevel::Info, LogCategory::BLE, "processTimedTasks",
                              "Advertising timeout - restoring to mode %d", restoreMode);

                // Trigger mode change - this will call stop() via state manager
                DeviceStateManager_SetMode(restoreMode);
            }
        }
    }

    // Check WiFi test progress (non-blocking)
    if (pBleStatus->wifiTestInProgress) {
        checkWifiTestProgress();
    }

    // Process BLE events (safe in main loop with large stack)
    if (pConfigServer) {
        if (pBleStatus->eventSendWifiConfig) {
            pBleStatus->eventSendWifiConfig = false;
            pConfigServer->sendWifiConfig();
        }

        if (pBleStatus->eventSaveWifiConfig) {
            pBleStatus->eventSaveWifiConfig = false;
            pConfigServer->saveWifiConfig(g_wifiConfigDoc);
        }

        if (pBleStatus->eventSendDeviceConfig) {
            pBleStatus->eventSendDeviceConfig = false;
            pConfigServer->sendDeviceConfig();
        }

        if (pBleStatus->eventSaveDeviceConfig) {
            pBleStatus->eventSaveDeviceConfig = false;
            pConfigServer->saveDeviceConfig(g_deviceConfigDoc);
        }

        if (pBleStatus->eventTestWifiConnection) {
            pBleStatus->eventTestWifiConnection = false;
            handleTestWifiConnection();
        }

        if (pBleStatus->eventWifiScan) {
            pBleStatus->eventWifiScan = false;
            handleWifiScan();
        }

        if (pBleStatus->eventSendConnectionInfo) {
            pBleStatus->eventSendConnectionInfo = false;
            sendConnectionInfo();
        }

        if (pBleStatus->eventRestartDevice) {
            pBleStatus->eventRestartDevice = false;
            pLogger->Write(LogLevel::Info, LogCategory::BLE, "loop", "Restart device requested");

            // Send acknowledgment
            const char* ack = "{\"type\":\"restart_device\",\"status\":\"ok\",\"message\":\"Restarting...\"}";
            sendData(ack);

            // Small delay to ensure message is sent
            delay(100);

            // Restart the ESP32
            ESP.restart();
        }
    }

    // Log categories auto-send disabled (Terminal feature removed)
}

// Start BLE
void BLEHandler::begin() {
    if (pBleStatus->taskRunning) {
        pLogger->WriteImmediate(LogLevel::Warn, LogCategory::BLE, "begin", "BLE already running");
        return;
    }

    // Initialize BLE
    initializeBLE();
    pBleStatus->taskRunning = true;
    connectionTime = millis();  // Track BLE start time for timeout
    lastActivityTime = millis();

    pLogger->WriteImmediate(LogLevel::Info, LogCategory::BLE, "begin", "BLE initialized");
}

// Stop BLE
void BLEHandler::stop() {
    if (!pBleStatus->taskRunning) return;

    pLogger->Write(LogLevel::Info, LogCategory::BLE, "stop", "Stopping BLE...");

    // Explicitly stop advertising first
    if (NimBLEDevice::getAdvertising()->isAdvertising()) {
        NimBLEDevice::getAdvertising()->stop();
        pLogger->Write(LogLevel::Info, LogCategory::BLE, "stop", "Advertising stopped");
    }

    // Clear connection state
    pBleStatus->deviceConnected = false;
    pBleStatus->taskRunning = false;
    pBleStatus->categoriesSent = false;

    // Reset timeout state
    timeoutTriggered = false;
    timeoutNotificationSentTime = 0;

    // Deinitialize BLE
    NimBLEDevice::deinit(true);

    pLogger->Write(LogLevel::Info, LogCategory::BLE, "stop", "BLE stopped");
}

// Suspend BLE task
void BLEHandler::suspend() {
    pBleStatus->taskRunning = false;
    pLogger->Write(LogLevel::Info, LogCategory::BLE, "suspend", "BLE task suspended");
}

// Resume BLE task
void BLEHandler::resume() {
    pBleStatus->taskRunning = true;
    pLogger->Write(LogLevel::Info, LogCategory::BLE, "resume", "BLE task resumed");
}

// Check if task is running
bool BLEHandler::isTaskRunning() {
    return pBleStatus->taskRunning;
}

// Check if connected
bool BLEHandler::isConnected() {
    return pBleStatus->deviceConnected;
}

// Handle received data from app
void BLEHandler::handleReceivedData(const std::string& value) {
    // Append to buffer
    rxBuffer += value;
    lastRxTime = millis();

    // Check for buffer overflow
    if (rxBuffer.length() > MAX_RX_BUFFER_SIZE) {
        Serial.println("RX buffer overflow - clearing");
        rxBuffer.clear();
        return;
    }

    // Try to parse JSON
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, rxBuffer.c_str());

    if (error) {
        // JSON incomplete - wait for more chunks
        // But if it's not a memory issue, clear the buffer after a certain size
        if (error != DeserializationError::IncompleteInput && rxBuffer.length() > 512) {
            Serial.print("JSON parse error: ");
            Serial.println(error.c_str());
            Serial.printf("Buffer contents (%d bytes): %.100s...\n", rxBuffer.length(), rxBuffer.c_str());
            rxBuffer.clear();
        }
        return;
    }

    // Successfully parsed - clear buffer and process
    Serial.printf("Successfully parsed JSON from buffer (%d bytes)\n", rxBuffer.length());
    rxBuffer.clear();

    // get_categories removed (Terminal feature removed)

    // Handle get WiFi config request - set event instead of calling directly
    if (doc["type"].is<const char*>() && strcmp(doc["type"], "get_wifi_config") == 0) {
        Serial.println("Get WiFi config request - setting event");
        pBleStatus->eventSendWifiConfig = true;
        return;
    }

    // Handle save WiFi config request - copy to global and set event
    if (doc["type"].is<const char*>() && strcmp(doc["type"], "save_wifi_config") == 0) {
        Serial.println("Save WiFi config request - setting event");
        g_wifiConfigDoc = doc;  // Copy to global document
        pBleStatus->eventSaveWifiConfig = true;
        return;
    }

    // Handle get Device config request - set event
    if (doc["type"].is<const char*>() && strcmp(doc["type"], "get_device_config") == 0) {
        Serial.println("Get Device config request - setting event");
        pBleStatus->eventSendDeviceConfig = true;
        return;
    }

    // Handle save Device config request - copy to global and set event
    if (doc["type"].is<const char*>() && strcmp(doc["type"], "save_device_config") == 0) {
        Serial.println("Save Device config request - setting event");
        g_deviceConfigDoc = doc;  // Copy to device config document
        pBleStatus->eventSaveDeviceConfig = true;
        return;
    }

    // Handle test WiFi connection request
    if (doc["type"].is<const char*>() && strcmp(doc["type"], "test_wifi_connection") == 0) {
        Serial.println("Test WiFi connection request - setting event");
        pBleStatus->eventTestWifiConnection = true;
        return;
    }

    // Handle WiFi scan request
    if (doc["type"].is<const char*>() && strcmp(doc["type"], "wifi_scan") == 0) {
        Serial.println("WiFi scan request - setting event");
        pBleStatus->eventWifiScan = true;
        return;
    }

    // Handle get connection info request (for wf_ip_addr, etc.)
    if (doc["type"].is<const char*>() && strcmp(doc["type"], "get_connection_info") == 0) {
        Serial.println("Get connection info request - setting event");
        pBleStatus->eventSendConnectionInfo = true;
        return;
    }

    // Handle restart device request
    if (doc["type"].is<const char*>() && strcmp(doc["type"], "restart_device") == 0) {
        Serial.println("Restart device request - setting event");
        pBleStatus->eventRestartDevice = true;
        return;
    }

    // Log level and category changes removed (Terminal feature removed)

    // Forward to application callbacks for custom commands
    if (pCallbacks) {
        pCallbacks->onCustomCommand(doc, this);
    }

    // Send simple acknowledgment (avoid large stack allocations in callback)
    const char* ack = "{\"status\":\"ok\"}";
    pTxCharacteristic->setValue((uint8_t*)ack, strlen(ack));
    pTxCharacteristic->notify();
}

// Send data to app (const char*)
void BLEHandler::sendData(const char* jsonString) {
    if (!pBleStatus->deviceConnected) return;

    pTxCharacteristic->setValue((uint8_t*)jsonString, strlen(jsonString));
    pTxCharacteristic->notify();
}

// Send data to app (binary)
void BLEHandler::sendData(const uint8_t* data, size_t length) {
    if (!pBleStatus->deviceConnected) return;

    pTxCharacteristic->setValue((uint8_t*)data, length);
    pTxCharacteristic->notify();
}

// Handle WiFi connection test (non-blocking start)
void BLEHandler::handleTestWifiConnection() {
    extern EventGroupHandle_t WifiManager_event_group;

    pLogger->Write(LogLevel::Info, LogCategory::BLE, "handleTestWifiConnection",
                   "Starting WiFi connection test...");

    // Send acknowledgment that test has started
    static char startMsg[128];
    snprintf(startMsg, sizeof(startMsg),
             "{\"type\":\"test_wifi_response\",\"status\":\"testing\",\"message\":\"Testing WiFi connection...\"}");
    sendData(startMsg);

    // Trigger WiFi test mode
    xEventGroupSetBits(WifiManager_event_group, EVENT_WIFI_TEST_BLE_MODE_BIT);

    // Start the non-blocking test
    pBleStatus->wifiTestInProgress = true;
    wifiTestStartTime = millis();
}

// Check WiFi test progress (called periodically from processTimedTasks)
void BLEHandler::checkWifiTestProgress() {
    extern ble_wifi_test_result_t WifiManager_GetBleTestResult(void);

    // Check if still connected
    if (!pBleStatus->deviceConnected) {
        pLogger->Write(LogLevel::Warn, LogCategory::BLE, "checkWifiTestProgress",
                       "Device disconnected during test");
        pBleStatus->wifiTestInProgress = false;
        return;
    }

    // Check for timeout
    if (millis() - wifiTestStartTime > WIFI_TEST_TIMEOUT_MS) {
        const char* timeoutMsg = "{\"type\":\"test_wifi_response\",\"status\":\"failed\",\"error\":\"Test timeout\"}";

        // Send timeout response FIRST, before any logging
        if (pTxCharacteristic && pBleStatus->deviceConnected) {
            pTxCharacteristic->setValue((uint8_t*)timeoutMsg, strlen(timeoutMsg));
            pTxCharacteristic->notify();
            delay(100);  // Give time for notification to send before logs
        }

        // Now log after the notification is sent
        pLogger->Write(LogLevel::Error, LogCategory::BLE, "checkWifiTestProgress",
                       "Test timeout - response sent");

        pBleStatus->wifiTestInProgress = false;
        return;
    }

    // Check if test is complete
    ble_wifi_test_result_t result = WifiManager_GetBleTestResult();
    if (result.test_complete) {
        // Build result JSON FIRST
        char resultMsg[256];
        if (result.test_success) {
            snprintf(resultMsg, sizeof(resultMsg),
                     "{\"type\":\"test_wifi_response\",\"status\":\"success\",\"ip\":\"%s\",\"message\":\"%s\"}",
                     result.ip_address, result.error_message);
        } else {
            snprintf(resultMsg, sizeof(resultMsg),
                     "{\"type\":\"test_wifi_response\",\"status\":\"failed\",\"error\":\"%s\"}",
                     result.error_message);
        }

        // Send the result IMMEDIATELY - before any logging
        if (pTxCharacteristic && pBleStatus->deviceConnected) {
            pTxCharacteristic->setValue((uint8_t*)resultMsg, strlen(resultMsg));
            pTxCharacteristic->notify();
            delay(100);  // Give more time for notification to send before logs flood in
        }

        // Now we can log (after the important notification is sent)
        pLogger->Write(LogLevel::Info, LogCategory::BLE, "checkWifiTestProgress",
                       "Test result sent - success=%d, ip=%s",
                       result.test_success ? 1 : 0,
                       result.test_success ? result.ip_address : "N/A");

        pBleStatus->wifiTestInProgress = false;
    }
}

// Handle WiFi scan request - perform scan and send results
void BLEHandler::handleWifiScan() {
    pLogger->Write(LogLevel::Info, LogCategory::BLE, "handleWifiScan",
                   "Starting WiFi scan...");

    // Send acknowledgment that scan has started
    const char* startMsg = "{\"type\":\"wifi_scan_response\",\"status\":\"scanning\",\"message\":\"Scanning for networks...\"}";
    sendData(startMsg);

    // Perform the WiFi scan (blocking, but should be quick)
    int found_networks = WiFi.scanNetworks();

    // Build JSON response
    JsonDocument doc;
    doc["type"] = "wifi_scan_response";

    if (found_networks <= 0) {
        // No networks found or scan failed
        doc["status"] = "error";
        doc["result"] = 0;
        doc["message"] = found_networks == 0 ? "No networks found" : "Scan failed";
        doc["list"] = JsonArray();
    } else {
        // Limit the number of networks
        const int MAX_NETWORKS = 20;
        if (found_networks > MAX_NETWORKS) found_networks = MAX_NETWORKS;

        doc["status"] = "ok";
        doc["result"] = 1;
        doc["message"] = "Networks found";
        doc["found_networks"] = found_networks;

        JsonArray list = doc["list"].to<JsonArray>();

        for (int i = 0; i < found_networks; i++) {
            JsonObject item = list.add<JsonObject>();
            item["ssid"] = WiFi.SSID(i);

            // Convert RSSI to percentage (similar to HTML version)
            int rssi = WiFi.RSSI(i);
            int rssi_percent;
            if (rssi >= -50) rssi_percent = 100;
            else if (rssi <= -100) rssi_percent = 0;
            else rssi_percent = 2 * (rssi + 100);
            item["rssi"] = rssi_percent;

            // Get encryption type
            wifi_auth_mode_t encryption = WiFi.encryptionType(i);
            const char* encType;
            switch (encryption) {
                case WIFI_AUTH_OPEN: encType = "Open"; break;
                case WIFI_AUTH_WEP: encType = "WEP"; break;
                case WIFI_AUTH_WPA_PSK: encType = "WPA"; break;
                case WIFI_AUTH_WPA2_PSK: encType = "WPA2"; break;
                case WIFI_AUTH_WPA_WPA2_PSK: encType = "WPA/WPA2"; break;
                case WIFI_AUTH_WPA2_ENTERPRISE: encType = "WPA2-Enterprise"; break;
                default: encType = "Unknown"; break;
            }
            item["encryption"] = encType;
        }
    }

    // Serialize and send
    String response;
    serializeJson(doc, response);

    // Send in chunks if needed (BLE has MTU limits)
    const size_t MTU_SIZE = 180;
    if (response.length() <= MTU_SIZE) {
        sendData(response.c_str());
    } else {
        // Send in chunks
        for (size_t i = 0; i < response.length(); i += MTU_SIZE) {
            size_t chunkSize = min(MTU_SIZE, response.length() - i);
            String chunk = response.substring(i, i + chunkSize);
            sendData((uint8_t*)chunk.c_str(), chunk.length());
            delay(50);  // Small delay between chunks
        }
    }

    pLogger->Write(LogLevel::Info, LogCategory::BLE, "handleWifiScan",
                   "Scan complete - found %d networks", found_networks);
}

// Send connection info (IP address, etc.)
void BLEHandler::sendConnectionInfo() {
    pLogger->Write(LogLevel::Info, LogCategory::BLE, "sendConnectionInfo",
                   "Sending connection info");

    // Build JSON response
    JsonDocument doc;
    doc["type"] = "connection_info";
    doc["status"] = "ok";

    // Get IP address from device_doc
    if (device_doc["wf_ip_addr"].is<const char*>()) {
        doc["wf_ip_addr"] = device_doc["wf_ip_addr"].as<String>();
    } else {
        doc["wf_ip_addr"] = "";
    }

    // Add other connection info if needed
    if (device_doc["wf_conn_ssid"].is<const char*>()) {
        doc["wf_conn_ssid"] = device_doc["wf_conn_ssid"].as<String>();
    }

    // Serialize and send
    String response;
    serializeJson(doc, response);
    sendData(response.c_str());

    pLogger->Write(LogLevel::Info, LogCategory::BLE, "sendConnectionInfo",
                   "Connection info sent: %s", response.c_str());
}

#endif // BLE_ENABLED
