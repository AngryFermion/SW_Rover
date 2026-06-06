#include <esp_task_wdt.h>
#include <main.h>
#include <ancit_device.h>
#include <ancit_http_client.h>
#include <ancit_mqtt_client.h>
#include <ancit_ntp.h>
#include <ancit_timer.h>
#include <ancit_wifi_manager.h>
#include <ancit_tasks.h>
#include "BT_LOGGER.h"

#if BLE_ENABLED
#include "ble_handler.h"
#include "ble_app.h"

// External declarations for BLE objects (defined in main.cpp)
extern BLEHandler* bleHandler;
extern BLEApp* bleApp;
#endif

void AncitTasks_setup(void) {
  // Create the tasks
  WifiManager_Start();
  vTimerTriggerTask_start();

#ifdef ENABLE_NTP
  ntp_start();
#endif

#ifdef ENABLE_MQTT
  if (!Device_IsRegistrationValid()) {
    g_Logger.Write(LogLevel::Error, LogCategory::MQTT, "AncitTasks_setup",
                     "Device is not registered. Skipping MQTT task creation.");
  } else {
    MqttClient_CreateTask();
  }
#endif

#ifdef ENABLE_HTTP_CLIENT
  HttpClient_CreateTask();
#endif

#if BLE_ENABLED
  // Initialize BLE BEFORE WiFi to avoid coexistence issues
  bleHandler = new BLEHandler(&g_Logger);
  bleApp = new BLEApp(bleHandler);
  // bleHandler->begin();
#endif

}
