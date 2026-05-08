#include "status_manager.h"
#include "../config/system_config.h"
#include "uart_manager.h"

StatusManager statusManager;

StatusManager::StatusManager() {
    currentStatus = LED_STATUS_OFF;
    lastWiFiState = WIFI_STATE_DISCONNECTED;
}

void StatusManager::init() {
    FastLED.addLeds<WS2812, RGB_LED_PIN, GRB>(leds, NUM_LEDS);
    FastLED.setBrightness(LED_BRIGHTNESS);
    leds[0] = CRGB::Black;
    FastLED.show();

    #if ENABLE_SERIAL_DEBUG
        uartManager.loggerWriteLine("[STATUS] Manager initialized");
        uartManager.loggerPrintf("[STATUS] RGB LED on GPIO %d\n", RGB_LED_PIN);
    #endif
}

void StatusManager::update() {
    WiFiState currentWiFiState = wifiManager.getState();
    if (currentWiFiState != lastWiFiState) {
        lastWiFiState = currentWiFiState;
        updateLEDFromWiFiState();
    }
}

void StatusManager::updateLEDFromWiFiState() {
    switch (lastWiFiState) {
        case WIFI_STATE_DISCONNECTED:
            setLEDStatus(LED_STATUS_RED);
            #if ENABLE_SERIAL_DEBUG
                uartManager.loggerWriteLine("[STATUS] LED -> RED (WiFi Disconnected)");
            #endif
            break;
        case WIFI_STATE_CONNECTING:
            setLEDStatus(LED_STATUS_YELLOW);
            #if ENABLE_SERIAL_DEBUG
                uartManager.loggerWriteLine("[STATUS] LED -> YELLOW (WiFi Connecting)");
            #endif
            break;
        case WIFI_STATE_CONNECTED:
            setLEDStatus(LED_STATUS_GREEN);
            #if ENABLE_SERIAL_DEBUG
                uartManager.loggerWriteLine("[STATUS] LED -> GREEN (WiFi Connected)");
            #endif
            break;
        case WIFI_STATE_RECONNECTING:
            setLEDStatus(LED_STATUS_YELLOW);
            #if ENABLE_SERIAL_DEBUG
                uartManager.loggerWriteLine("[STATUS] LED -> YELLOW (WiFi Reconnecting)");
            #endif
            break;
    }
}

void StatusManager::setLEDStatus(LEDStatus status) {
    if (currentStatus == status) return;
    currentStatus = status;
    switch (status) {
        case LED_STATUS_OFF:    applyLEDColor(CRGB::Black);  break;
        case LED_STATUS_RED:    applyLEDColor(CRGB::Red);    break;
        case LED_STATUS_GREEN:  applyLEDColor(CRGB::Green);  break;
        case LED_STATUS_YELLOW: applyLEDColor(CRGB::Yellow); break;
        case LED_STATUS_BLUE:   applyLEDColor(CRGB::Blue);   break;
    }
}

void StatusManager::applyLEDColor(CRGB color) {
    leds[0] = color;
    FastLED.show();
}

LEDStatus StatusManager::getLEDStatus() { return currentStatus; }
