#ifndef STATUS_MANAGER_H
#define STATUS_MANAGER_H

#include <Arduino.h>
#include <FastLED.h>
#include "wifi_manager.h"

typedef enum {
    LED_STATUS_OFF,
    LED_STATUS_RED,     // WiFi disconnected
    LED_STATUS_GREEN,   // WiFi connected
    LED_STATUS_YELLOW,  // WiFi connecting / reconnecting
    LED_STATUS_BLUE     // Custom
} LEDStatus;

class StatusManager {
public:
    StatusManager();

    void      init();
    void      update();
    void      setLEDStatus(LEDStatus status);
    LEDStatus getLEDStatus();

private:
    CRGB      leds[1];
    LEDStatus currentStatus;
    WiFiState lastWiFiState;

    void updateLEDFromWiFiState();
    void applyLEDColor(CRGB color);
};

extern StatusManager statusManager;

#endif // STATUS_MANAGER_H
