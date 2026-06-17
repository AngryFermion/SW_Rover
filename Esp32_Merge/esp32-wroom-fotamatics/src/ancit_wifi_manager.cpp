// optimized_wifi_manager.cpp

#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <WiFi.h>
#include <config_reader.h>
#include <main.h>
#include <ancit_device.h>
#include <ancit_pins.h>
#include <ancit_server.h>
#include <ancit_utils.h>
#include <ancit_wifi_manager.h>
#include <device_state_manager.h>
#include <ancit_config_switch.h>

#include "ancit_crypto.h"
#include "AncitRgbLed.h"
#include "ancit_ntp.h"
#include "ancit_timer.h"
#include "ancit_tasks.h"
#include "BT_LOGGER.h"
#include "esp_wifi.h"
#include "esp_task_wdt.h"
#include "nvs_flash.h"

TaskHandle_t WifiManager_task_handle = NULL;
EventGroupHandle_t WifiManager_event_group;
extern EventGroupHandle_t ntp_event_group;
JsonDocument cached_scan_result;
bool scan_data_ready = false;

// BLE WiFi Test Result (global for BLE handler to read)
static ble_wifi_test_result_t g_ble_test_result = {
  false,  // test_complete
  false,  // test_success
  "0.0.0.0",  // ip_address
  ""  // error_message
};

static bool WifiManager_IsFullyConnected(void) {
  return (WiFi.status() == WL_CONNECTED && globals.wifi.got_ip);
}

// Helper function to safely change WiFi mode with proper delay
// This prevents coexistence layer crashes by allowing hardware to initialize
void WifiManager_SafeModeChange(wifi_mode_t mode) {
  WiFi.mode(mode);
  delay(100);  // Allow WiFi subsystem to initialize/switch modes
}

// Helper function to safely disconnect WiFi with proper delay
// wifiOff: true = disconnect and turn off WiFi, false = disconnect only
void WifiManager_SafeDisconnect(bool wifiOff) {
  WiFi.disconnect(wifiOff);
  delay(100);  // Allow disconnect operation to complete
}

// DEPRECATED: Boot mode determination is now handled by DeviceStateManager
// This function is kept for backwards compatibility but should not be used
wifi_mode_t WifiManager_DetermineMode(void) {
  g_Logger.Write(LogLevel::Debug, LogCategory::WIFI, "WifiManager_DetermineMode",
                   "sta_enabled:%s (DEPRECATED - use DeviceStateManager)",
                   wifi_doc["sta_enabled"].as<String>().c_str());

  if (!wifi_doc["sta_enabled"]) return WIFI_AP;
  return WIFI_STA;
}

void WifiManager_Event(WiFiEvent_t event) {
  switch (event) {
    case ARDUINO_EVENT_WIFI_STA_START:
      g_Logger.Write(LogLevel::Debug, LogCategory::WIFI, "WifiManager_Event",
                       "WiFi STA mode started");
      DeviceStateManager_SetNormalState(NORMAL_STATE_WIFI_DISCONNECTED);
      break;

    case ARDUINO_EVENT_WIFI_STA_CONNECTED:
      g_Logger.Write(LogLevel::Debug, LogCategory::WIFI, "WifiManager_Event",
                       "Connected to router");
      DeviceStateManager_SetNormalState(NORMAL_STATE_WIFI_CONNECTED);
      break;

    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
      globals.wifi.got_ip = false;
      g_Logger.Write(LogLevel::Warn, LogCategory::WIFI, "WifiManager_Event",
                       "WiFi disconnected");
      DeviceStateManager_SetNormalState(NORMAL_STATE_WIFI_DISCONNECTED);
      break;

    case ARDUINO_EVENT_WIFI_STA_LOST_IP:
      globals.wifi.got_ip = false;
      g_Logger.Write(LogLevel::Warn, LogCategory::WIFI, "WifiManager_Event",
                       "Lost IP address");
      DeviceStateManager_SetNormalState(NORMAL_STATE_WIFI_CONNECTED);
      break;

    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      globals.wifi.got_ip = true;
      g_Logger.Write(LogLevel::Info, LogCategory::WIFI, "WifiManager_Event",
                       "Got IP: %s", WiFi.localIP().toString().c_str());
      DeviceStateManager_SetNormalState(NORMAL_STATE_WIFI_GOT_IP);

      if ((xEventGroupGetBits(WifiManager_event_group) &
           (EVENT_WIFI_TEST_MODE_BIT | EVENT_WIFI_TEST_BLE_MODE_BIT)) == 0) {
        WifiManager_StoreIp();
#ifdef ENABLE_NTP
        xEventGroupSetBits(ntp_event_group, NTP_EVENT_START_BIT);
#endif
      } else {
        g_Logger.Write(LogLevel::Debug, LogCategory::WIFI, "WifiManager_Event",
                         "Test mode active, skipping IP storage and NTP start");
      }
      break;

    case ARDUINO_EVENT_WIFI_AP_STACONNECTED:
      g_Logger.Write(LogLevel::Debug, LogCategory::WIFI, "WifiManager_Event",
                       "Device connected to AP");
      DeviceStateManager_SetConfigState(CONFIG_STATE_CONNECTED);
      break;

    case ARDUINO_EVENT_WIFI_AP_STADISCONNECTED:
      g_Logger.Write(LogLevel::Debug, LogCategory::WIFI, "WifiManager_Event",
                       "Device disconnected from AP");
      DeviceStateManager_SetConfigState(CONFIG_STATE_UNCONNECTED);
      break;

    default:
      break;
  }
}

void WifiManager_Reconnect(void) {
  g_Logger.Write(LogLevel::Debug, LogCategory::WIFI, "WifiManager_Reconnect",
                   "Performing WiFi reconnection...");

  esp_wifi_disconnect();
  WiFi.disconnect(false);
  delay(500);
  // WiFi.reconnect() does not re-supply credentials and is unreliable on ESP32.
  // Re-calling SetupStation forces a clean WiFi.begin(ssid, pass).
  WifiManager_SetupStation(false);
}

void WifiManager_CheckStatus(void) {
  static uint8_t no_ip_counter = 0;
  static uint8_t disconnect_counter = 0;

  wl_status_t status = WiFi.status();

  if (status == WL_CONNECTED) {
    if (!globals.wifi.got_ip) {
      if (++no_ip_counter > WIFI_NO_IP_TIMEOUT) {
        no_ip_counter = 0;
        g_Logger.Write(
            LogLevel::Error, LogCategory::WIFI, "WifiManager_CheckStatus",
            "Connected but got_ip flag is false, forcing reconnect.");
        WifiManager_Reconnect();
      }
    } else {
      no_ip_counter = 0;
      disconnect_counter = 0;
    }
  } else {
    no_ip_counter = 0;
    if (++disconnect_counter > WIFI_DISCONNECTED_TIMEOUT) {
      disconnect_counter = 0;
      g_Logger.Write(LogLevel::Error, LogCategory::WIFI, "WifiManager_CheckStatus",
                       "WiFi not connected (status: %d), reconnecting...",
                       status);
      WifiManager_Reconnect();
    }
  }
}

void WifiManager_SetupSoftAp(bool serverStart) {
  const char *dev_id = device_doc["dev_id"];
  const char *device_name = devComm_doc["user"]["device_name"];
  char ap_name[31];
  char decrypted_password[17];

  g_Logger.Write(LogLevel::Info, LogCategory::WIFI, "WifiManager_SetupSoftAp",
                   "Starting in Configuration mode");

  globals.wifi.config_mode = true;
  snprintf(ap_name, sizeof(ap_name), "%.15s(%s)", device_name, dev_id);

  IPAddress local_IP, gateway, subnet;
  local_IP.fromString(ACCESS_POINT_IP);
  gateway.fromString(ACCESS_POINT_GATEWAY);
  subnet.fromString(ACCESS_POINT_SUBNET);

  // Attempt to decrypt the stored WiFi password
  if (decrypt_password(decrypted_password)) {
    // Switch to AP mode first (required before disconnect)
    WifiManager_SafeModeChange(WIFI_AP);

    // Ensure WiFi STA is fully disconnected before starting AP
    WifiManager_SafeDisconnect(true);

    // Start the soft AP
    WiFi.softAP(ap_name, decrypted_password);

    delay(50);  // Allow time for AP interface to stabilize

    // Apply the static IP configuration
    WiFi.softAPConfig(local_IP, gateway, subnet);
    g_Logger.Write(LogLevel::Info, LogCategory::WIFI, "WifiManager_SetupSoftAp",
                     "Config Mode Started - SSID: %s | IP: %s", ap_name,
                     WiFi.softAPIP().toString().c_str());

    // Optionally start the web server
    if (serverStart) {
      setup_server();
    }
  } else {
    g_Logger.Write(
        LogLevel::Info, LogCategory::WIFI, "WifiManager_SetupSoftAp",
        "Unable to decrypt WiFi configuration password, Contact support.");
  }
}

void WifiManager_SetupStation(bool serverStart) {
  globals.wifi.config_mode = false;
  g_Logger.Write(LogLevel::Debug, LogCategory::WIFI, "WifiManager_SetupStation",
                   "Setting up WiFi in active mode...");

  String wifi_config;
  serializeJson(wifi_doc, wifi_config);
  g_Logger.Write(LogLevel::Debug, LogCategory::WIFI, "WifiManager_SetupStation",
                   "WiFi Config: %s", wifi_config.c_str());

  // Extract WiFi credentials from the configuration
  JsonObject wf_w1 = wifi_doc["w1"];

  // Use String objects to safely handle SSIDs/passwords with spaces and special characters
  String wf_ssid = wf_w1["ssid"].as<String>();
  String wf_pw = wf_w1["pw"].as<String>();

  // Validate credentials
  if (wf_ssid.isEmpty()) {
    g_Logger.Write(LogLevel::Error, LogCategory::WIFI, "WifiManager_SetupStation",
                   "SSID is empty or invalid");
    return;
  }

  g_Logger.Write(LogLevel::Debug, LogCategory::WIFI, "WifiManager_SetupStation",
                 "SSID: '%s' (length: %d)", wf_ssid.c_str(), wf_ssid.length());

  // Check if static IP configuration is enabled
  JsonObject wf_static = wf_w1["static"];
  bool is_static = wf_static["en"];

  if (is_static) {
    // Parse static IP settings using String to avoid pointer issues
    String ip_str = wf_static["ip"].as<String>();
    String gw_str = wf_static["gw"].as<String>();
    String sn_str = wf_static["sn"].as<String>();
    String dns_str = wf_static["dns"].as<String>();

    IPAddress ip, gw, sn, dns;
    ip.fromString(ip_str);
    gw.fromString(gw_str);
    sn.fromString(sn_str);
    dns.fromString(dns_str);

    // Apply static IP configuration
    if (!WiFi.config(ip, gw, sn, dns)) {
      g_Logger.Write(LogLevel::Error, LogCategory::WIFI, "WifiManager_SetupStation",
                     "Failed to configure static IP");
    } else {
      g_Logger.Write(LogLevel::Debug, LogCategory::WIFI, "WifiManager_SetupStation",
                     "Static IP configured: %s", ip_str.c_str());
    }

    // Mark the device as using static IP
    device_doc["wf_ip_static"] = true;
  } else {
    // Mark the device as not using static IP
    device_doc["wf_ip_static"] = false;
  }

  // Set WiFi mode to station and begin connection
  WifiManager_SafeModeChange(WIFI_STA);

#if BLE_ENABLED
  // MIN_MODEM sleep lets the radio listen to every DTIM beacon (coexists with BLE)
  // without the aggressive duty-cycling of full modem sleep that drops connections
  // under sustained MQTT traffic.
  esp_wifi_set_ps(WIFI_PS_MIN_MODEM);
#endif

  // Disconnect any existing connection before starting new one
  WifiManager_SafeDisconnect(false);

  WiFi.begin(wf_ssid.c_str(), wf_pw.c_str());

  g_Logger.Write(LogLevel::Info, LogCategory::WIFI, "WifiManager_SetupStation",
                   "Attempting to connect to SSID: '%s' (pwd length: %d)",
                   wf_ssid.c_str(), wf_pw.length());
  if (serverStart) setup_server();
}

void WifiManager_StopSoftAp(void) {
  g_Logger.Write(LogLevel::Info, LogCategory::WIFI, "WifiManager_StopSoftAp",
                 "Stopping WiFi AP mode...");

  // Disconnect all clients and stop the soft AP
  esp_wifi_disconnect();
  WiFi.softAPdisconnect(true);
  delay(100);  // Allow AP disconnect to complete (softAPdisconnect is different API)

  g_Logger.Write(LogLevel::Info, LogCategory::WIFI, "WifiManager_StopSoftAp",
                 "WiFi AP stopped");
}

void WifiManager_StopStation(void) {
  g_Logger.Write(LogLevel::Info, LogCategory::WIFI, "WifiManager_StopStation",
                 "Stopping WiFi STA mode...");

  // Disconnect from the access point and stop STA mode
  esp_wifi_disconnect();
  WifiManager_SafeDisconnect(true);

  g_Logger.Write(LogLevel::Info, LogCategory::WIFI, "WifiManager_StopStation",
                 "WiFi STA stopped");
}

void WifiManager_Start(void) {
  g_Logger.WriteImmediate(LogLevel::Info, LogCategory::WIFI, "WifiManager_Start",
                          "Initializing WiFi Manager (mode controlled by DeviceStateManager)...");

  globals.wifi = {};

  // Initialize wifi global variables
  globals.wifi.forced_ap_mode_exit_ctr = FORCED_AP_MODE_EXIT_TIMEOUT;
  globals.wifi.forced_ap_mode_config_file_access_ctr =
      FORCED_AP_MODE_NO_CONFIG_TIMEOUT;
  globals.wifi.forced_ap_mode = false;

  // Create event group for WiFi events FIRST
  WifiManager_event_group = xEventGroupCreate();

  // CRITICAL: Initialize NVS (Non-Volatile Storage) - required by WiFi
  g_Logger.WriteImmediate(LogLevel::Info, LogCategory::WIFI, "WifiManager_Start",
                          "Initializing NVS...");
  esp_err_t err = nvs_flash_init();
  if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    // NVS partition was truncated, erase and retry
    g_Logger.WriteImmediate(LogLevel::Warn, LogCategory::WIFI, "WifiManager_Start",
                            "NVS partition issue, erasing and retrying...");
    ESP_ERROR_CHECK(nvs_flash_erase());
    err = nvs_flash_init();
  }
  if (err == ESP_OK) {
    g_Logger.WriteImmediate(LogLevel::Info, LogCategory::WIFI, "WifiManager_Start",
                            "NVS initialized successfully");
  } else {
    g_Logger.WriteImmediate(LogLevel::Error, LogCategory::WIFI, "WifiManager_Start",
                            "NVS init failed: %d (WiFi may fail)", err);
  }

  // Initialize WiFi to OFF mode to ensure clean state
  // This prevents crashes when later switching to STA or AP mode
  WifiManager_SafeModeChange(WIFI_OFF);

  // Register WiFi event handler
  WiFi.onEvent(WifiManager_Event);

  g_Logger.WriteImmediate(LogLevel::Info, LogCategory::WIFI, "WifiManager_Start",
                          "WiFi initialized and event handler registered");

  // NOTE: WiFi mode (AP/STA) is now started by DeviceStateManager via callbacks
  // Do NOT call WifiManager_SetupSoftAp() or WifiManager_SetupStation() here

  // Start WiFi monitor task on Core 1
  xTaskCreatePinnedToCore(
      WifiManager_Task,          /* Task function. */
      "WifiManager_Task",        /* name of task. */
      ANCIT_WIFI_TASK_STACK_SIZE, /* Stack size of task */
      NULL,                      /* parameter of the task */
      ANCIT_WIFI_TASK_PRIORITY,   /* priority of the task */
      &WifiManager_task_handle,  /* Task handle to keep track of created task */
      ANCIT_WIFI_TASK_CORE);      /* Task to be executed in Core-1 only */

  g_Logger.WriteImmediate(LogLevel::Info, LogCategory::WIFI, "WifiManager_Start",
                          "WiFi Manager task created");
}

void WifiManager_CheckForcedApEntry(void) {
#ifdef ENABLE_CONFIG_SWITCH
  static int countdown = FORCED_AP_MODE_RESET_TIMEOUT;
  bool buttonPressed = ConfigSwitch_IsPressed();

  if (buttonPressed) {
    if (--countdown <= 0) {
      g_Logger.Write(LogLevel::Info, LogCategory::WIFI,
                       "WifiManager_CheckForcedApEntry",
                       "Forced AP Mode - Resetting device...");
      ancit_device_restart();
    }
  } else {
    countdown = FORCED_AP_MODE_RESET_TIMEOUT;
  }
#endif
}

void WifiManager_ForcedApTimeout(void) {
  if (!globals.wifi.forced_ap_mode) return;

  int connectedDevices = device_doc["wf_conn_devs"];
  if (connectedDevices == 0) {
    if (--globals.wifi.forced_ap_mode_exit_ctr <= 0) {
      g_Logger.Write(LogLevel::Warn, LogCategory::WIFI,
                       "WifiManager_ForcedApTimeout",
                       "No devices connected. Restarting device...");
      ancit_device_restart();
    }
  } else {
    globals.wifi.forced_ap_mode_exit_ctr = FORCED_AP_MODE_EXIT_TIMEOUT;
    if (--globals.wifi.forced_ap_mode_config_file_access_ctr <= 0) {
      g_Logger.Write(LogLevel::Warn, LogCategory::WIFI,
                       "WifiManager_ForcedApTimeout",
                       "No config page accessed. Restarting device...");
      ancit_device_restart();
    }
  }
}

void WifiManager_1000msActivities(void) {
  if (WiFi.getMode() == WIFI_STA) {
    WifiManager_CheckStatus();
  }
}

static float WifiManager_ConvertRssi(float RssI) {
  RssI = isnan(RssI) ? -100.0 : RssI;
  RssI = min(max(2 * (RssI + 100.0), 0.0), 100.0);
  return RssI;
}

String WifiManager_GetEncryptionType(uint8_t i) {
  String retVal = "Unknown";
  wifi_auth_mode_t encryptionType = WiFi.encryptionType(i);
  switch (encryptionType) {
    case WIFI_AUTH_OPEN:
      retVal = "OPEN";
      break;
    case WIFI_AUTH_WEP:
      retVal = "WEP";
      break;
    case WIFI_AUTH_WPA_PSK:
      retVal = "WPA_PSK";
      break;
    case WIFI_AUTH_WPA2_PSK:
      retVal = "WPA2_PSK";
      break;
    case WIFI_AUTH_WPA_WPA2_PSK:
      retVal = "WPA_WPA2_PSK";
      break;
    case WIFI_AUTH_WPA2_ENTERPRISE:
      retVal = "WPA2_ENTERPRISE";
      break;
    case WIFI_AUTH_MAX:
      retVal = "Unknown";
      break;
  }
  return retVal;
}

void WifiManager_WriteIpToFile(const String &new_ip, bool reset_ip) {
  g_Logger.Write(LogLevel::Debug, LogCategory::WIFI, "WifiManager_WriteIpToFile",
                   "Stored new IP address: %s", new_ip.c_str());
  device_doc["wf_ip_addr"] = new_ip;

  if (reset_ip) {
    device_doc["wf_conn_ssid"] = "Connection failed!!";
    device_doc["wf_gw"] = "0.0.0.0";
    device_doc["wf_sn"] = "0.0.0.0";
    device_doc["wf_dns"] = "0.0.0.0";
  } else {
    device_doc["wf_conn_ssid"] = WiFi.SSID();
    device_doc["wf_gw"] = WiFi.gatewayIP().toString();
    device_doc["wf_sn"] = WiFi.subnetMask().toString();
    device_doc["wf_dns"] = WiFi.dnsIP().toString();
  }

  File file = SPIFFS.open("/config/device.json", "w");
  if (file) {
    serializeJson(device_doc, file);
    file.close();
  } else {
    g_Logger.Write(LogLevel::Error, LogCategory::WIFI, "WifiManager_WriteIpToFile",
                     "Failed to open /config/device.json for writing");
  }
}

void WifiManager_StoreIp(void) {
  String new_ip = WiFi.localIP().toString();
  const char *wf_ip_addr = device_doc["wf_ip_addr"];
  if (!new_ip.equals(wf_ip_addr)) {
    WifiManager_WriteIpToFile(new_ip, false);
  } else {
    g_Logger.Write(LogLevel::Debug, LogCategory::WIFI, "WifiManager_StoreIp",
                     "Old IP, skip storage: %s", new_ip.c_str());
  }
}

void WifiManager_Task(void *param) {
  EventBits_t events;

  // Event group is already created in WifiManager_Start() — do not recreate it here.
  // Creating a second group would overwrite the global handle and leak the first one.
  if (WifiManager_event_group == NULL) {
    g_Logger.Write(LogLevel::Error, LogCategory::WIFI, "WifiManager_Task",
                     "Event group not initialised — was WifiManager_Start() called?");
    vTaskDelete(NULL);
    return;
  }

  // Add this task to watchdog
  esp_task_wdt_add(NULL);

  for (;;) {
    // Wait for events with timeout to allow watchdog reset
    events = xEventGroupWaitBits(
        WifiManager_event_group,
        EVENT_WIFI_1000MS | EVENT_WIFI_SCAN_REQUEST | EVENT_WIFI_TEST_MODE_BIT | EVENT_WIFI_TEST_BLE_MODE_BIT,
        pdTRUE, pdFALSE, pdMS_TO_TICKS(5000));  // 5 second timeout instead of portMAX_DELAY

    // Reset watchdog every loop iteration
    esp_task_wdt_reset();

    if (events & EVENT_WIFI_1000MS) WifiManager_HandleEvent1000ms();
    if (events & EVENT_WIFI_SCAN_REQUEST) WifiManager_HandleScanRequest();
    if (events & EVENT_WIFI_TEST_MODE_BIT) WifiManager_HandleTestMode();
    if (events & EVENT_WIFI_TEST_BLE_MODE_BIT) WifiManager_HandleTestMode_BLE();
  }
}

static void WifiManager_HandleEvent1000ms(void) {
  // If not in test mode, check for forced AP mode entry
  if ((xEventGroupGetBits(WifiManager_event_group) &
       EVENT_WIFI_TEST_MODE_BIT) == 0) {
    WifiManager_CheckForcedApEntry();
  }

  if (WiFi.getMode() == WIFI_STA) {
    WifiManager_CheckStatus();
  }
}

// This function handles the WiFi scan request
static void WifiManager_HandleScanRequest(void) {
  // Save the current WiFi mode and switch to STA mode for scanning
  // wifi_mode_t prev_mode = WiFi.getMode();
  WifiManager_SafeModeChange(WIFI_STA);  // safer mode for scanning

  // Log the start of the WiFi scan
  g_Logger.Write(LogLevel::Debug, LogCategory::WIFI, "WifiManager_ScanRequest",
                   "Wifi Scan initiated...");

  // Start async WiFi scan (non-blocking)
  esp_task_wdt_reset();
  int16_t scan_result = WiFi.scanNetworks(true);  // true = async mode

  if (scan_result == WIFI_SCAN_RUNNING) {
    g_Logger.Write(LogLevel::Debug, LogCategory::WIFI, "WifiManager_ScanRequest",
                     "Async scan started, waiting for results...");

    // Poll for scan completion (with watchdog resets)
    int max_wait = 60;  // 30 seconds max (500ms * 60)
    while (scan_result == WIFI_SCAN_RUNNING && max_wait-- > 0) {
      vTaskDelay(pdMS_TO_TICKS(500));
      esp_task_wdt_reset();  // Reset watchdog while waiting
      scan_result = WiFi.scanComplete();
    }
  }

  int found_networks = scan_result;
  esp_task_wdt_reset();

  cached_scan_result.clear();  // Clear any previous scan results

  if (found_networks <= 0) {
    // No networks found or scan failed
    cached_scan_result["result"] = 0;
    cached_scan_result["message"] =
        found_networks == 0 ? "No networks found" : "Scan failed";
  } else {
    // Limit the number of networks to the maximum allowed
    if (found_networks > SCAN_DEVICES_MAX) found_networks = SCAN_DEVICES_MAX;

    // Populate the scan result with the found networks
    cached_scan_result["result"] = 1;
    cached_scan_result["message"] = "Networks found";
    cached_scan_result["found_networks"] = found_networks;

    JsonArray list = cached_scan_result["list"].to<JsonArray>();
    String ssid_log;

    for (uint8_t i = 0; i < found_networks; i++) {
      // Add each network's details to the result
      JsonObject item = list.add<JsonObject>();
      String ssid = WiFi.SSID(i);
      item["ssid"] = ssid;
      item["rssi"] = WifiManager_ConvertRssi(WiFi.RSSI(i));
      item["encryption"] = WifiManager_GetEncryptionType(i);

      // Build a log string of SSIDs
      ssid_log += ssid;
      if (i < found_networks - 1) ssid_log += ", ";
    }

    // Log the list of scanned SSIDs
    g_Logger.Write(LogLevel::Debug, LogCategory::WIFI, "wifi_scan_task",
                     "Scanned SSIDs: %s", ssid_log.c_str());
  }

  // Mark the scan data as ready and go back to the configuration mode
  scan_data_ready = true;

  WifiManager_SafeModeChange(WIFI_AP);
}

// This function handles the WiFi test mode
static void WifiManager_HandleTestMode(void) {
  g_Logger.Write(LogLevel::Debug, LogCategory::WIFI, "wifi_task",
                   "Starting WiFi test mode...");

  // Reset the got_ip flag and set up WiFi in station mode
  globals.wifi.got_ip = false;
  WifiManager_SetupStation(false);

  // Retry connecting to WiFi
  int retries = 300;  // 30 sec @100ms
  while (retries-- > 0 && !globals.wifi.got_ip) {
    vTaskDelay(pdMS_TO_TICKS(100));
  }

  // Check if WiFi connection was successful
  if (globals.wifi.got_ip) {
    // Log success and store the IP address
    g_Logger.Write(LogLevel::Debug, LogCategory::WIFI, "wifi_task",
                     "WiFi test mode completed, IP: %s",
                     WiFi.localIP().toString().c_str());
    WifiManager_WriteIpToFile(WiFi.localIP().toString(), false);
  } else {
    // Log failure and store a default IP address
    g_Logger.Write(LogLevel::Info, LogCategory::WIFI, "wifi_task",
                     "WiFi test mode failed, no IP address assigned");
    WifiManager_WriteIpToFile("0.0.0.0", true);
  }

  // Switch back to Access Point mode
  WifiManager_SetupSoftAp(true);

  // Clear the WiFi test mode event bit
  xEventGroupClearBits(WifiManager_event_group, EVENT_WIFI_TEST_MODE_BIT);

  g_Logger.Write(LogLevel::Debug, LogCategory::WIFI, "wifi_task",
                   "Exiting WiFi test mode, restarting configuration mode...");
}

// This function handles the BLE WiFi test mode
static void WifiManager_HandleTestMode_BLE(void) {
  g_Logger.Write(LogLevel::Info, LogCategory::WIFI, "WifiManager_HandleTestMode_BLE",
                   "Starting BLE WiFi test mode...");

  // Reset test result
  g_ble_test_result.test_complete = false;
  g_ble_test_result.test_success = false;
  strcpy(g_ble_test_result.ip_address, "0.0.0.0");
  strcpy(g_ble_test_result.error_message, "");

  // Reset the got_ip flag before testing
  globals.wifi.got_ip = false;

  // Call SetupStation to switch to STA mode and connect
  WifiManager_SetupStation(false);

  // Retry connecting to WiFi
  int retries = 300;  // 30 sec @100ms
  g_Logger.Write(LogLevel::Debug, LogCategory::WIFI, "WifiManager_HandleTestMode_BLE",
                   "Waiting for connection (max 30s)...");

  while (retries-- > 0 && !globals.wifi.got_ip) {
    vTaskDelay(pdMS_TO_TICKS(100));
    esp_task_wdt_reset();  // Reset watchdog to prevent timeout during long wait
    if ((retries % 10) == 0) {  // Log every second
      g_Logger.Write(LogLevel::Debug, LogCategory::WIFI, "WifiManager_HandleTestMode_BLE",
                       "Waiting... got_ip=%d, WiFi.status=%d, retries=%d",
                       globals.wifi.got_ip, WiFi.status(), retries);
    }
  }

  g_Logger.Write(LogLevel::Debug, LogCategory::WIFI, "WifiManager_HandleTestMode_BLE",
                   "Wait complete - got_ip=%d, WiFi.status=%d, IP=%s",
                   globals.wifi.got_ip, WiFi.status(), WiFi.localIP().toString().c_str());

  // Check if WiFi connection was successful
  if (globals.wifi.got_ip) {
    // Log success and populate result
    g_Logger.Write(LogLevel::Info, LogCategory::WIFI, "WifiManager_HandleTestMode_BLE",
                     "WiFi test SUCCESS, IP: %s", WiFi.localIP().toString().c_str());

    g_ble_test_result.test_success = true;
    strncpy(g_ble_test_result.ip_address, WiFi.localIP().toString().c_str(),
            sizeof(g_ble_test_result.ip_address) - 1);
    snprintf(g_ble_test_result.error_message, sizeof(g_ble_test_result.error_message),
             "Connected successfully to %s", WiFi.SSID().c_str());
  } else {
    // Log failure and populate error
    wl_status_t status = WiFi.status();
    g_Logger.Write(LogLevel::Warn, LogCategory::WIFI, "WifiManager_HandleTestMode_BLE",
                     "WiFi test FAILED, status: %d", status);

    g_ble_test_result.test_success = false;
    strcpy(g_ble_test_result.ip_address, "0.0.0.0");

    // Determine error message based on WiFi status
    switch (status) {
      case WL_NO_SSID_AVAIL:
        strcpy(g_ble_test_result.error_message, "Network not found");
        break;
      case WL_CONNECT_FAILED:
        strcpy(g_ble_test_result.error_message, "Connection failed - check password");
        break;
      case WL_CONNECTION_LOST:
        strcpy(g_ble_test_result.error_message, "Connection lost");
        break;
      case WL_DISCONNECTED:
        strcpy(g_ble_test_result.error_message, "Failed to connect - timeout");
        break;
      default:
        snprintf(g_ble_test_result.error_message, sizeof(g_ble_test_result.error_message),
                 "Connection failed (status: %d)", status);
        break;
    }
  }

  // Disconnect WiFi and return to previous mode
  esp_wifi_disconnect();
  WifiManager_SafeDisconnect(false);

  // Return to BLE-friendly mode (turn off WiFi to keep BLE stable)
  WifiManager_SafeModeChange(WIFI_OFF);

  g_Logger.Write(LogLevel::Info, LogCategory::WIFI, "WifiManager_HandleTestMode_BLE",
                   "WiFi turned off, BLE mode preserved");

  // Mark test as complete
  g_ble_test_result.test_complete = true;

  // Clear the BLE WiFi test mode event bit
  xEventGroupClearBits(WifiManager_event_group, EVENT_WIFI_TEST_BLE_MODE_BIT);

  g_Logger.Write(LogLevel::Info, LogCategory::WIFI, "WifiManager_HandleTestMode_BLE",
                   "BLE WiFi test completed - Result: %s",
                   g_ble_test_result.test_success ? "SUCCESS" : "FAILED");
}

// Get BLE WiFi test result (for BLE handler to read)
ble_wifi_test_result_t WifiManager_GetBleTestResult(void) {
  return g_ble_test_result;
}

