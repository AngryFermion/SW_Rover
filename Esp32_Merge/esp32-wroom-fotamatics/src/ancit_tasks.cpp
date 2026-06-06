#include <esp_task_wdt.h>
#include <main.h>
#include <ancit_device.h>
#include <ancit_http_client.h>
#include <ancit_mqtt_client.h>
#include <ancit_ntp.h>
#include <ancit_timer.h>
#include <ancit_wifi_manager.h>
#include <ancit_tasks.h>
#include <telematics_manager.h>
#include "app_config.h"
#include "BT_LOGGER.h"

#if BLE_ENABLED
#include "ble_handler.h"
#include "ble_app.h"
extern BLEHandler* bleHandler;
extern BLEApp* bleApp;
#endif

void AncitTasks_setup(void) {
    WifiManager_Start();
    vTimerTriggerTask_start();

#ifdef ENABLE_NTP
    ntp_start();
#endif

#ifdef ENABLE_MQTT
#if FOTAMATICS_MODE == APP_MODE_TELEMATICS_ONLY
    // Telematics-only mode: FOTA registration is not required, MQTT is always needed
    MqttClient_CreateTask();
#else
    if (!Device_IsRegistrationValid()) {
        g_Logger.Write(LogLevel::Error, LogCategory::MQTT, "AncitTasks_setup",
                       "Device not registered — skipping MQTT task");
    } else {
        MqttClient_CreateTask();
    }
#endif
#endif

#ifdef ENABLE_HTTP_CLIENT
    HttpClient_CreateTask();
#endif

#if FOTAMATICS_MODE == APP_MODE_TELEMATICS_ONLY || FOTAMATICS_MODE == APP_MODE_BOTH
    // Telematics task — CAN→MQTT gateway + optional FOTA-aware UART reader
    xTaskCreatePinnedToCore(
        TelematicsTask,
        "telematics_task",
        ANCIT_TELEMATICS_TASK_STACK_SIZE,
        NULL,
        ANCIT_TELEMATICS_TASK_PRIORITY,
        NULL,
        ANCIT_TELEMATICS_TASK_CORE
    );
    g_Logger.Write(LogLevel::Info, LogCategory::OTHERS, "AncitTasks_setup",
                   "Telematics task created (core %d, stack %d)",
                   ANCIT_TELEMATICS_TASK_CORE, ANCIT_TELEMATICS_TASK_STACK_SIZE);
#else
    g_Logger.Write(LogLevel::Info, LogCategory::OTHERS, "AncitTasks_setup",
                   "Telematics task DISABLED (FOTA-only mode)");
#endif

#if BLE_ENABLED
    bleHandler = new BLEHandler(&g_Logger);
    bleApp = new BLEApp(bleHandler);
#endif
}
