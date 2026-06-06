#ifndef _HEADER_MAIN_H_
#define _HEADER_MAIN_H_
#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP32Time.h>
#include <ESPAsyncWebServer.h>
// Note: Include <BT_LOGGER.h> directly in files that need it to avoid conflicts

// Forward declaration for old logger (deprecated)
class Logger;

// Constatnts

// Variables
extern AsyncWebServer server;
extern Logger *g_pLogger;  // Deprecated - use g_Logger instead
// extern bool g_valid_subscription;

extern ESP32Time rtc;

// Functions
void timer_timeout_1ms(void);
void timer_timeout_10ms(void);
void timer_timeout_50ms(void);
void timer_timeout_100ms(void);
void timer_timeout_250ms(void);
void timer_timeout_500ms(void);
void timer_timeout_1000ms(void);

void setup_device_params(void);
void wifi_1000_ms_activities(void);
#endif