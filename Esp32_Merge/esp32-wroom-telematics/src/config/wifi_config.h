/**
 * wifi_config.h — SmartWheels ESP32-WROOM Telematics
 */

#ifndef WIFI_CONFIG_H
#define WIFI_CONFIG_H

// ===== WiFi CREDENTIALS =====
#define WIFI_SSID     "SDV Demo"
#define WIFI_PASSWORD "12345678"

// ===== CONNECTION PARAMETERS =====
#define WIFI_CONNECTION_TIMEOUT_MS   10000
#define WIFI_RECONNECT_INTERVAL_MS   5000
#define WIFI_MAX_RECONNECT_ATTEMPTS  10

#endif // WIFI_CONFIG_H
