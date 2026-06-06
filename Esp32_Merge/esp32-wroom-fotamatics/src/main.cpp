#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP32Time.h>
#include <ESPAsyncWebServer.h>
#include <WiFi.h>

#include <ancit_device.h>
#include <ancit_pins.h>
#include <ancit_tasks.h>
#include <ancit_time.h>
#include <ancit_utils.h>
#include <ancit_wifi_manager.h>
#include <ancit_fota_handler.h>
#include <ancit_fota_serial_handler.h>
#include <ancit_mqtt_client.h>
#include <config_reader.h>
#include <esp_task_wdt.h>
#include <BT_LOGGER.h>
#include <main.h>
#include <ancit_config_switch.h>
#include <AncitRgbLed.h>
#include <device_state_manager.h>
#include <telematics_config.h>
#include "app_config.h"

#if BLE_ENABLED
#include "ble_handler.h"
#include "ble_app.h"
BLEHandler* bleHandler = nullptr;
BLEApp*     bleApp     = nullptr;
#endif

Logger* g_pLogger = NULL;  // Deprecated

void setup(void) {
    // ===== Phase 1: Core hardware & immediate visual feedback =====
    setCpuFrequencyMhz(240);

    Serial.begin(DEBUG_SERIAL_BAUD_RATE, SERIAL_8N1,
                 DEBUG_SERIAL_RX_PIN, DEBUG_SERIAL_TX_PIN);
    delay(200);

    RgbLed_Start();  // Shows red slow blink while booting

    // ===== Phase 2: Essential services =====
    esp_task_wdt_init(15, true);
    esp_task_wdt_add(NULL);

    g_Logger.SetLogLevel(LogLevel::Debug);
    BT_LOGGER::PrintVersion();

    g_Logger.WriteImmediate(LogLevel::Info, LogCategory::SETUP, "setup",
                            "SmartWheels ESP32-WROOM FOTAmatics starting...");

    // Active feature mode banner
#if FOTAMATICS_MODE == APP_MODE_FOTA_ONLY
    g_Logger.WriteImmediate(LogLevel::Info, LogCategory::SETUP, "setup",
                            "Mode: FOTA_ONLY  — telematics task and SmartKit topics disabled");
#elif FOTAMATICS_MODE == APP_MODE_TELEMATICS_ONLY
    g_Logger.WriteImmediate(LogLevel::Info, LogCategory::SETUP, "setup",
                            "Mode: TELEMATICS_ONLY — FOTA subscriptions and serial handler disabled");
#else
    g_Logger.WriteImmediate(LogLevel::Info, LogCategory::SETUP, "setup",
                            "Mode: BOTH — FOTA + Telematics active on shared MQTT + Serial1");
#endif

#if FOTAMATICS_MODE != APP_MODE_FOTA_ONLY
#ifdef USE_DUMMY_DATA
    g_Logger.WriteImmediate(LogLevel::Info, LogCategory::SETUP, "setup",
                            "[Telematics] Dummy data mode — Serial1 not used for telematics RX");
#else
    g_Logger.WriteImmediate(LogLevel::Info, LogCategory::SETUP, "setup",
                            "[Telematics] UART mode — Serial1 GPIO%d(RX)/GPIO%d(TX)",
                            TELEMATICS_SERIAL_RX_PIN, TELEMATICS_SERIAL_TX_PIN);
#endif
#endif

    start_file_storage();
    read_devComm_file();
    read_device_file();
    read_registration_file();

#ifdef ENABLE_WIFI
    read_wifi_file();
#endif

    // ===== Phase 3: Device & application setup =====
    mtime_init();
    setup_device_params();
    Device_ValidateRegistration();

    // ===== Phase 4: State machine & user input =====
#ifdef ENABLE_CONFIG_SWITCH
    ConfigSwitch_Start();
#endif
    DeviceStateManager_Start();

    // ===== Phase 5: Application tasks =====
    // Starts: WiFi, Timer, NTP, MQTT, HTTP, Telematics, (optional BLE)
    AncitTasks_setup();

    g_Logger.WriteImmediate(LogLevel::Info, LogCategory::SETUP, "setup",
                            "Setup complete — all tasks running");
}

void loop(void) {
    BT_LOGGER::ProcessQueue();

#if BLE_ENABLED
    if (bleHandler != nullptr) bleHandler->processTimedTasks();
#endif

    static unsigned long lastLog = 0;
    unsigned long now = millis();

    if (now - lastLog >= 5000) {
        lastLog = now;

        const char* fotaStateStr[] = {"IDLE", "RECEIVING", "READY_TO_TRANSMIT"};
        const char* bootStepStr[]  = {"", "BOOT", "PROGRAM", "TX", "VERIFY", "RESET"};
        const char* modeStr[]      = {"BOOT", "CONFIG", "NORMAL", "BT"};

        int bootStep = ancitFotaSerialHandler.getCurrentBootloadStep();
        int lineNum  = ancitFotaSerialHandler.getCurrentLine();
        DeviceMode_t mode = DeviceStateManager_GetMode();

        String modeInfo = String("Mode:") + modeStr[mode];
        if (mode == DEVICE_MODE_NORMAL && DeviceStateManager_GetEditEnabled())
            modeInfo += "(EDIT)";

        String wifiStatus = WiFi.getMode() == WIFI_STA ? "STA" :
                            (WiFi.getMode() == WIFI_AP ? "AP" : "OFF");
        wifiStatus += "|";
        if (WiFi.status() == WL_CONNECTED) {
            wifiStatus += "Conn|";
            wifiStatus += WiFi.localIP().toString();
        } else {
            wifiStatus += "Disc";
        }

        g_Logger.Write(LogLevel::Info, LogCategory::SETUP, "loop",
                       "%s | WiFi:%s | MQTT:%s | FOTA:%s(%d/%d) | Serial:%s[%d]Line:%d | Heap:%d",
                       modeInfo.c_str(),
                       wifiStatus.c_str(),
                       MqttClient_IsConnected() ? "Conn" : "Disc",
                       fotaStateStr[fotaHandler.getState()],
                       fotaHandler.getReceivedChunks(),
                       fotaHandler.getExpectedChunks(),
                       bootStepStr[bootStep], bootStep, lineNum,
                       ESP.getFreeHeap());
    }

    delay(50);
    esp_task_wdt_reset();
}
