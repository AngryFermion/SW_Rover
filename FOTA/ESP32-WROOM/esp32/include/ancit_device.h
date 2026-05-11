#ifndef _HEADER_ANCIT_DEVICE_H_
#define _HEADER_ANCIT_DEVICE_H_
#include <Arduino.h>
// #include <ancit_mqtt.h>
#include <ancit_wifi_manager.h>
// #include <ancit_AsyncTCP.h>
// #include <ancit_live.h>
#include <ancit_rtc_hw.h>
#include <ancit_time.h>

#define ANCIT_TRUE 1
#define ANCIT_FALSE 0

#define MANDATORY_RESTART_INTERVAL_BACKUP 9000 * 1000
#define MANDATORY_RESTART_SKIP_INTERVAL 10 * 1000

#define ENABLE_WIFI
// #define ENABLE_MODBUS
// #define ENABLE_MODBUS_MASTER
#define ENABLE_NTP
#define ENABLE_NVS
// #define ENABLE_SD_STORAGE
// #define ENABLE_DEVICE_REGISTRATION_CHECK
// #define ENABLE_HTTP_CLIENT
#define ENABLE_MQTT
#define ENABLE_CATEGORY_WISE_LOG
// #define ENABLE_DATA_LOGGER

// #define ENABLE_CONFIG_SWITCH  // Enable this if hardware has a config button (GPIO 34)

// #define BLE_ENABLED 1  // Disabled temporarily - causes WiFi coexistence crash
// #define RTC_HARWARE_EXISTS

#define EVENT_BIT_DEVICE_DATA_READY (1 << 0)
#define EVENT_BIT_DEVICE_REGISTERED (1 << 1)

typedef struct {
  String md5;
} globals_device_t;

typedef struct {
  globals_device_t device;
  global_wifi_t wifi;
  global_time_t time;
} globals_t;

extern globals_t globals;

// extern bool g_valid_subscription;

extern EventGroupHandle_t device_event_group;

void update_params(void);
void ancit_device_restart(void);
void init_time(void);

String Device_GetMd5(const String& str);
void Device_ValidateRegistration(void);

void mandatory_restart_check(void);
// void setup_log_category(void);  // Not needed - gSetLogCategory is initialized in BT_LOGGER
void start_file_storage(void);
void validate_iot_method(void);
bool Device_IsRegistrationValid(void);
void Device_WaitForDataReady(const char* caller);
#endif