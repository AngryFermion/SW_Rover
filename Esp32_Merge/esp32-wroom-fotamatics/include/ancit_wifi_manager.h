#pragma once

#include <Arduino.h>
#include <WiFi.h>

#ifdef __cplusplus
extern "C" {
#endif

// Constants
#define WIFI_DISCONNECTED_TIMEOUT 8
#define WIFI_NO_IP_TIMEOUT 10
#define WIFI_TASK_DELAY_MS 1000

#define ACCESS_POINT_NAME "MetiFoTA"
#define ACCESS_POINT_PASSWORD "meti1234"
#define ACCESS_POINT_IP "10.10.10.10"
#define ACCESS_POINT_GATEWAY "10.10.10.0"
#define ACCESS_POINT_SUBNET "255.255.255.0"

#define FORCED_AP_MODE_RESET_TIMEOUT 3   // 3 seconds(cheked every 50ms)
#define FORCED_AP_MODE_EXIT_TIMEOUT 120  // 120 seconds(cheked every 1 second)
#define FORCED_AP_MODE_NO_CONFIG_TIMEOUT \
  300  // 300 seconds(cheked every 1 second)

// Variables
enum wifi_led_states {
  WIFI_CONFIG_MODE_ERROR = 0,    // RED ON
  WIFI_CONFIG_MODE_UNCONNECTED,  // RED BLINK FAST
  WIFI_CONFIG_MODE_CONNECTED,    // RED BLINK SLOW
  WIFI_ACTIVE_MODE_UNCONNECTED,  // BLUE FAST BLINK
  WIFI_ACTIVE_MODE_CONNECTED     // BLUE STABLE
};

#define EVENT_WIFI_1000MS            (1 << 1)
#define EVENT_WIFI_SCAN_REQUEST      (1 << 2)
#define EVENT_WIFI_TEST_MODE_BIT     (1 << 3)
#define EVENT_WIFI_TEST_BLE_MODE_BIT (1 << 4)

#define SCAN_DEVICES_MAX 32

#define WIFI_TEST_MODE_OFF 0
#define WIFI_TEST_MODE_START 1
#define WIFI_TEST_MODE_WAIT_FOR_CONNECT 2
#define WIFI_TEST_MODE_WAIT_FOR_IP_DETAILS 3
#define WIFI_TEST_MODE_DONE 4

// Narayana - this needs to go to a different file
//  Multi LED state mappings (assumed to be defined elsewhere)
#define MULTI_LED_STATE_RED_ON 10
#define MULTI_LED_STATE_RED_BLINK_FAST 11
#define MULTI_LED_STATE_RED_BLINK_SLOW 12
#define MULTI_LED_STATE_BLUE_BLINK_FAST 13
#define MULTI_LED_STATE_BLUE_ON 14

// Variables

typedef struct {
  bool config_mode;
  bool got_ip;

  bool forced_ap_mode;
  int forced_ap_mode_exit_ctr;
  int forced_ap_mode_config_file_access_ctr;

} global_wifi_t;

// BLE WiFi Test Result Structure
typedef struct {
  bool test_complete;
  bool test_success;
  char ip_address[16];
  char error_message[64];
} ble_wifi_test_result_t;

// Helper functions for safe WiFi operations
void WifiManager_SafeModeChange(wifi_mode_t mode);
void WifiManager_SafeDisconnect(bool wifiOff);

// Initializes WiFi (STA or AP based on conditions)
void WifiManager_Start(void);
void WifiManager_SetupSoftAp(bool serverStart);
void WifiManager_SetupStation(bool serverStart);
void WifiManager_StopSoftAp(void);
void WifiManager_StopStation(void);
void WifiManager_StoreIp(void);

// WiFi event handler
void WifiManager_Event(WiFiEvent_t event);

// WiFi connection check and reconnection logic
void WifiManager_CheckStatus(void);
void WifiManager_Reconnect(void);

// FreeRTOS task to monitor WiFi periodically
void WifiManager_Task(void *param);
void WifiManager_100msJob(void);
void WifiManager_StoreStaIpAddress(void);

static void WifiManager_HandleEvent1000ms(void);
static void WifiManager_HandleScanRequest(void);
static void WifiManager_HandleTestMode(void);
static void WifiManager_HandleTestMode_BLE(void);

// Get BLE WiFi test result (for BLE handler to read)
ble_wifi_test_result_t WifiManager_GetBleTestResult(void);

#ifdef __cplusplus
}
#endif
