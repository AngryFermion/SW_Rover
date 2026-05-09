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

#if BLE_ENABLED
#include "ble_handler.h"
#include "ble_app.h"
BLEHandler* bleHandler = nullptr;
BLEApp* bleApp = nullptr;
#endif

Logger *g_pLogger = NULL;  // Deprecated - use g_Logger instead


void setup(void) {
  // ========== Phase 1: Core Hardware & Immediate Feedback ==========
  setCpuFrequencyMhz(240);

  Serial.begin(DEBUG_SERIAL_BAUD_RATE, SERIAL_8N1, DEBUG_SERIAL_RX_PIN,
               DEBUG_SERIAL_TX_PIN);
  delay(200);

  // Start LED FIRST - immediate visual feedback to user
  RgbLed_Start();  // Shows red slow blink by default

  // ========== Phase 2: Essential Services ==========
  // Watchdog timer (15 seconds - tasks reset watchdog during long operations)
  esp_task_wdt_init(15, true);
  esp_task_wdt_add(NULL);

  // Initialize logger BEFORE tasks that use it
  g_Logger.SetLogLevel(LogLevel::Debug);
  BT_LOGGER::PrintVersion();

  g_Logger.WriteImmediate(LogLevel::Info, LogCategory::SETUP, "setup",
                   "ServeXL FoTA modbus Start...");
  g_Logger.WriteImmediate(LogLevel::Info, LogCategory::SETUP, "setup",
                   "Set Log categories %04X", gSetLogCategory);

  // Mount file system BEFORE reading configs
  start_file_storage();

  // Load all configuration files
  read_devComm_file();
  read_device_file();
  read_registration_file();

#ifdef ENABLE_WIFI
  read_wifi_file();  // MUST be before DeviceStateManager_Start()
#endif

  // ========== Phase 3: Device & Application Setup ==========

  // Time and device parameter setup
  mtime_init();
  setup_device_params();
  Device_ValidateRegistration();

  // ========== Phase 4: State Machine & User Input ==========
  // Now that configs are loaded, start state management
#ifdef ENABLE_CONFIG_SWITCH
  ConfigSwitch_Start();       // Monitor button presses
#endif
  DeviceStateManager_Start(); // Takes over LED, reads wifi.json, starts WiFi

  // ========== Phase 5: Application Tasks ==========
  AncitTasks_setup();  // WiFi mode already started by state manager

  g_Logger.WriteImmediate(LogLevel::Info, LogCategory::SETUP, "setup",
                   "Project Setup Complete");
}

void loop(void) {
  // Process queued logs first
  BT_LOGGER::ProcessQueue();

#if BLE_ENABLED
  // Process BLE timed tasks (like sending categories after connection)
  if (bleHandler != nullptr) {
    bleHandler->processTimedTasks();
  }
#endif

  static unsigned long lastLogTime = 0;
  unsigned long currentTime = millis();

  // Log task and state machine status every 5 seconds
  if (currentTime - lastLogTime >= 5000) {
    lastLogTime = currentTime;

    const char* fotaStateStr[] = {"IDLE", "RECEIVING", "READY_TO_TRANSMIT"};
    const char* bootStepStr[] = {"", "BOOT", "PROGRAM", "TX", "VERIFY", "RESET"};
    const char* modeStr[] = {"BOOT", "CONFIG", "NORMAL", "BT"};

    int bootStep = ancitFotaSerialHandler.getCurrentBootloadStep();
    int lineNum = ancitFotaSerialHandler.getCurrentLine();

    DeviceMode_t currentMode = DeviceStateManager_GetMode();
    String modeInfo = "Mode:";
    modeInfo += modeStr[currentMode];

    // Add edit mode suffix for NORMAL mode
    if (currentMode == DEVICE_MODE_NORMAL && DeviceStateManager_GetEditEnabled()) {
      modeInfo += "(EDIT)";
    }

    // Build WiFi status string with IP address
    String wifiStatus = WiFi.getMode() == WIFI_STA ? "STA" : (WiFi.getMode() == WIFI_AP ? "AP" : "OFF");
    wifiStatus += "|";
    if (WiFi.status() == WL_CONNECTED) {
      wifiStatus += "Conn|";
      wifiStatus += WiFi.localIP().toString();
    } else {
      wifiStatus += "Disc";
    }

    // Build BLE status string
    String bleStatus = "BLE:";
#if BLE_ENABLED
    if (currentMode == DEVICE_MODE_BLUETOOTH) {
      BluetoothModeState_t btState = DeviceStateManager_GetBluetoothState();
      bleStatus += btState == BT_STATE_PAIRED ? "Paired" : "Unpaired";
    } else {
      bleStatus += "OFF";
    }
#else
    bleStatus += "Disabled";
#endif

    g_Logger.Write(LogLevel::Info, LogCategory::SETUP, "loop",
                     "%s | WiFi:%s | %s | MQTT:%s | FOTA:%s(%d/%d)|Serial:%s[%d]Line:%d | Heap:%d",
                     modeInfo.c_str(),
                     wifiStatus.c_str(),
                     bleStatus.c_str(),
                     MqttClient_IsConnected() ? "Conn" : "Disc",
                     fotaStateStr[fotaHandler.getState()],
                     fotaHandler.getReceivedChunks(),
                     fotaHandler.getExpectedChunks(),
                     bootStepStr[bootStep],
                     bootStep,
                     lineNum,
                     ESP.getFreeHeap());
  }

  delay(50);
  // Prevent watchdog timer from triggering
  esp_task_wdt_reset();
}
