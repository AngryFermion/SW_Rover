#include "wifi_manager.h"
#include "../config/wifi_config.h"
#include "../config/system_config.h"
#include "uart_manager.h"

WiFiManager wifiManager;

WiFiManager::WiFiManager() {
    currentState          = WIFI_STATE_DISCONNECTED;
    lastConnectionAttempt = 0;
    connectionStartTime   = 0;
    reconnectAttempts     = 0;
}

void WiFiManager::init() {
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(false);

    #if ENABLE_SERIAL_DEBUG
        uartManager.loggerWriteLine("[WIFI] Manager initialized");
        uartManager.loggerPrintf("[WIFI] SSID: %s\n", WIFI_SSID);
    #endif

    changeState(WIFI_STATE_DISCONNECTED);
}

void WiFiManager::update() {
    switch (currentState) {
        case WIFI_STATE_DISCONNECTED:  handleDisconnected();  break;
        case WIFI_STATE_CONNECTING:    handleConnecting();    break;
        case WIFI_STATE_CONNECTED:     handleConnected();     break;
        case WIFI_STATE_RECONNECTING:  handleReconnecting();  break;
    }
}

void WiFiManager::handleDisconnected() {
    startConnection();
    changeState(WIFI_STATE_CONNECTING);
}

void WiFiManager::handleConnecting() {
    if (WiFi.status() == WL_CONNECTED) {
        changeState(WIFI_STATE_CONNECTED);
        #if ENABLE_SERIAL_DEBUG
            uartManager.loggerWriteLine("[WIFI] Connected successfully!");
            uartManager.loggerPrintf("[WIFI] IP Address: %s\n", WiFi.localIP().toString().c_str());
            uartManager.loggerPrintf("[WIFI] RSSI: %d dBm\n", WiFi.RSSI());
        #endif
        reconnectAttempts = 0;
        return;
    }
    if (millis() - connectionStartTime >= WIFI_CONNECTION_TIMEOUT_MS) {
        #if ENABLE_SERIAL_DEBUG
            uartManager.loggerWriteLine("[WIFI] Connection timeout!");
        #endif
        WiFi.disconnect();
        changeState(WIFI_STATE_RECONNECTING);
    }
}

void WiFiManager::handleConnected() {
    if (WiFi.status() != WL_CONNECTED) {
        #if ENABLE_SERIAL_DEBUG
            uartManager.loggerWriteLine("[WIFI] Connection lost!");
        #endif
        changeState(WIFI_STATE_RECONNECTING);
    }
}

void WiFiManager::handleReconnecting() {
    if (millis() - lastConnectionAttempt >= WIFI_RECONNECT_INTERVAL_MS) {
        reconnectAttempts++;
        #if ENABLE_SERIAL_DEBUG
            uartManager.loggerPrintf("[WIFI] Reconnection attempt #%d\n", reconnectAttempts);
        #endif
        if (WIFI_MAX_RECONNECT_ATTEMPTS > 0 &&
            reconnectAttempts >= WIFI_MAX_RECONNECT_ATTEMPTS) {
            #if ENABLE_SERIAL_DEBUG
                uartManager.loggerWriteLine("[WIFI] Max reconnect attempts reached!");
            #endif
            changeState(WIFI_STATE_DISCONNECTED);
            return;
        }
        startConnection();
        changeState(WIFI_STATE_CONNECTING);
    }
}

void WiFiManager::startConnection() {
    connectionStartTime   = millis();
    lastConnectionAttempt = millis();
    #if ENABLE_SERIAL_DEBUG
        uartManager.loggerPrintf("[WIFI] Connecting to: %s\n", WIFI_SSID);
    #endif
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void WiFiManager::changeState(WiFiState newState) {
    if (currentState != newState) {
        #if ENABLE_SERIAL_DEBUG
            uartManager.loggerPrintf("[WIFI] State: %d -> %d\n", currentState, newState);
        #endif
        currentState = newState;
    }
}

WiFiState WiFiManager::getState()    { return currentState; }
bool      WiFiManager::isConnected() { return (currentState == WIFI_STATE_CONNECTED && WiFi.status() == WL_CONNECTED); }

String WiFiManager::getIPAddress() {
    return isConnected() ? WiFi.localIP().toString() : "Not Connected";
}

int WiFiManager::getRSSI() {
    return isConnected() ? WiFi.RSSI() : 0;
}

void WiFiManager::disconnect() {
    #if ENABLE_SERIAL_DEBUG
        uartManager.loggerWriteLine("[WIFI] Manual disconnect requested");
    #endif
    WiFi.disconnect();
    changeState(WIFI_STATE_DISCONNECTED);
}

void WiFiManager::reconnect() {
    #if ENABLE_SERIAL_DEBUG
        uartManager.loggerWriteLine("[WIFI] Manual reconnect requested");
    #endif
    WiFi.disconnect();
    reconnectAttempts = 0;
    changeState(WIFI_STATE_DISCONNECTED);
}
