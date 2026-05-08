#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>

typedef enum {
    WIFI_STATE_DISCONNECTED,
    WIFI_STATE_CONNECTING,
    WIFI_STATE_CONNECTED,
    WIFI_STATE_RECONNECTING
} WiFiState;

class WiFiManager {
public:
    WiFiManager();

    void init();
    void update();

    WiFiState getState();
    bool      isConnected();
    String    getIPAddress();
    int       getRSSI();
    void      disconnect();
    void      reconnect();

private:
    WiFiState     currentState;
    unsigned long lastConnectionAttempt;
    unsigned long connectionStartTime;
    int           reconnectAttempts;

    void handleDisconnected();
    void handleConnecting();
    void handleConnected();
    void handleReconnecting();
    void startConnection();
    void changeState(WiFiState newState);
};

extern WiFiManager wifiManager;

#endif // WIFI_MANAGER_H
