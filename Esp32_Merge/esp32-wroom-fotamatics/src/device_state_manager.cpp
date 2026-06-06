/*
 * Device State Manager - Hierarchical State Machine
 *
 * OVERVIEW:
 * =========
 * This module manages the device's operating modes and their sub-states using a
 * hierarchical state machine pattern. It coordinates LED patterns and notifies
 * other modules of mode changes via callbacks.
 *
 * STATE HIERARCHY:
 * ================
 *
 * DEVICE_MODE_BOOT (top-level)
 *   └─ No sub-states
 *   └─ LED: Red Fast Blink
 *
 * DEVICE_MODE_CONFIG (WiFi AP mode)
 *   ├─ CONFIG_STATE_UNCONNECTED → Red Fast Blink
 *   └─ CONFIG_STATE_CONNECTED → Red Slow Blink
 *
 * DEVICE_MODE_NORMAL (WiFi STA mode)
 *   ├─ NORMAL_STATE_WIFI_DISCONNECTED → Blue Fast Blink
 *   ├─ NORMAL_STATE_WIFI_CONNECTED → Blue Slow Blink
 *   └─ NORMAL_STATE_WIFI_GOT_IP
 *       ├─ Edit Disabled → Blue Solid
 *       └─ Edit Enabled → Blue/Green Alternate
 *
 * DEVICE_MODE_BLUETOOTH
 *   ├─ BT_STATE_UNPAIRED → Red/Green Fast Blink
 *   └─ BT_STATE_PAIRED → Red/Green Slow Blink
 *
 *
 * FLOW 1: SWITCH EVENT → MODE CHANGE
 * ====================================
 *
 * 1. User presses switch physically
 * 2. Config Switch module detects press type (SHORT/LONG/HOLD)
 * 3. Config Switch calls onSwitchEvent(event)
 *
 * 4. onSwitchEvent() processes event:
 *
 *    CFG_SW_EVENT_HOLD:
 *      └─ DeviceStateManager_SetMode(DEVICE_MODE_CONFIG)
 *          └─ Sets requestedMode = CONFIG, modeChangeRequested = true
 *          └─ Main task picks up change request
 *          └─ Calls handleModeTransition(CONFIG)
 *              ├─ Sets currentMode = CONFIG
 *              ├─ Initializes configState = UNCONNECTED
 *              ├─ Calls applyLedPattern()
 *              │   └─ Sets LED to Red Fast Blink
 *              └─ Calls modeChangeCallback(oldMode, CONFIG)
 *                  └─ main.cpp starts WiFi AP mode
 *
 *    CFG_SW_EVENT_LONG_PRESS:
 *      └─ DeviceStateManager_SetMode(DEVICE_MODE_BLUETOOTH)
 *          └─ Similar flow, initializes btState = UNPAIRED
 *          └─ LED: Red/Green Fast Blink
 *          └─ Callback starts BLE mode
 *
 *    CFG_SW_EVENT_SHORT_PRESS (only in NORMAL mode):
 *      └─ Toggles editEnabled flag
 *      └─ Calls applyLedPattern()
 *          └─ If WIFI_GOT_IP: Switches LED between Blue Solid ↔ Blue/Green Alternate
 *      └─ NO mode change, NO callback invoked
 *
 *
 * FLOW 2: WIFI EVENT → SUB-STATE CHANGE
 * =======================================
 *
 * 1. WiFi hardware generates event (e.g., got IP address)
 * 2. WiFi manager receives event via WifiManager_Event()
 * 3. WiFi manager calls DeviceStateManager_SetNormalState(WIFI_GOT_IP)
 *
 * 4. DeviceStateManager_SetNormalState():
 *    ├─ Validates currentMode == NORMAL
 *    ├─ Checks if state actually changed
 *    ├─ Updates normalState = WIFI_GOT_IP
 *    ├─ Logs state change
 *    └─ Calls applyLedPattern()
 *        └─ Checks normalState and editEnabled
 *            ├─ If editEnabled: Blue/Green Alternate
 *            └─ Else: Blue Solid
 *
 * 5. NO mode change occurred, so NO callback invoked
 *
 *
 * FLOW 3: LED PATTERN APPLICATION
 * =================================
 *
 * applyLedPattern():
 *
 * 1. Acquires LED mutex for thread safety
 * 2. Checks currentMode
 * 3. Within each mode, checks sub-state:
 *
 *    DEVICE_MODE_CONFIG:
 *      ├─ If configState == UNCONNECTED → Red Fast Blink
 *      └─ If configState == CONNECTED → Red Slow Blink
 *
 *    DEVICE_MODE_NORMAL:
 *      ├─ If normalState == WIFI_DISCONNECTED → Blue Fast Blink
 *      ├─ If normalState == WIFI_CONNECTED → Blue Slow Blink
 *      └─ If normalState == WIFI_GOT_IP:
 *          ├─ If editEnabled → Blue/Green Alternate
 *          └─ Else → Blue Solid
 *
 *    DEVICE_MODE_BLUETOOTH:
 *      ├─ If btState == UNPAIRED → Red/Green Fast Blink
 *      └─ If btState == PAIRED → Red/Green Slow Blink
 *
 * 4. Sets LED hardware via netLed->setState()
 * 5. Releases mutex
 *
 *
 * FLOW 4: BOOT SEQUENCE
 * ======================
 *
 * 1. main.cpp setup():
 *    ├─ ConfigSwitch_Start() - Initialize switch hardware
 *    ├─ RgbLed_Start() - Initialize LED hardware
 *    └─ DeviceStateManager_Start()
 *
 * 2. DeviceStateManager_Start():
 *    ├─ DeviceStateManager_Init()
 *    │   ├─ Sets currentMode = BOOT
 *    │   └─ Creates LED mutex
 *    ├─ Registers internalModeChangeHandler (WiFi/BLE orchestration)
 *    ├─ Registers onSwitchEvent (button press handler)
 *    └─ Creates DeviceStateManager_Task
 *
 * 3. DeviceStateManager_Task() starts in background:
 *    ├─ Waits 500ms for LED init
 *    ├─ Shows BOOT LED (Red Fast Blink) for 2 seconds
 *    ├─ Calls DeviceStateManager_DetermineBootMode()
 *    │   ├─ Checks ConfigSwitch_IsPressed() - HIGHEST PRIORITY
 *    │   │   └─ If pressed: Force CONFIG mode
 *    │   └─ Else: Read wifi.json sta_enabled flag
 *    │       └─ Returns CONFIG or NORMAL mode
 *    └─ Calls handleModeTransition(bootMode)
 *        ├─ Initializes appropriate sub-state
 *        ├─ Applies LED pattern
 *        └─ Calls internalModeChangeHandler(BOOT → CONFIG/NORMAL)
 *            └─ Starts WiFi AP or STA mode
 *
 * 4. Main loop runs continuously:
 *    ├─ Checks modeChangeRequested flag
 *    ├─ If switch pressed: handleModeTransition()
 *    │   └─ Calls internalModeChangeHandler → WiFi mode switch
 *    └─ Sleeps 100ms, repeats
 *
 * 5. Meanwhile, ancit_tasks.cpp calls WifiManager_Start():
 *    ├─ Initializes WiFi globals
 *    ├─ Registers WiFi event handler
 *    ├─ Creates WiFi monitor task
 *    └─ Does NOT start WiFi (handled by state manager)
 *
 *
 * THREAD SAFETY:
 * ==============
 * - LED access protected by ledMutex
 * - Mode changes happen in single task (DeviceStateManager_Task)
 * - Sub-state setters can be called from any task (WiFi, BLE, etc.)
 * - All state setters re-apply LED pattern atomically
 *
 *
 * KEY DESIGN PRINCIPLES:
 * ======================
 * 1. Single Responsibility: State manager only tracks state and controls LED
 * 2. Separation of Concerns: WiFi/BLE init handled via callbacks, not here
 * 3. Observer Pattern: Callback notifies main.cpp of mode changes
 * 4. Hierarchical States: Mode + Sub-state for clean organization
 * 5. Thread Safety: Mutex protects shared LED resource
 */

#include <Arduino.h>
#include <device_state_manager.h>
#include <ancit_config_switch.h>
#include <AncitRgbLed.h>
#include <config_reader.h>
#include <ancit_tasks.h>
#include <BT_LOGGER.h>
#include <esp_task_wdt.h>
#include <ancit_wifi_manager.h>
#include <ancit_mqtt_client.h>
#include "ancit_device.h"

#if BLE_ENABLED
#include <ble_handler.h>
#include <ble_app.h>
extern BLEHandler* bleHandler;
extern BLEApp* bleApp;
#endif

// External dependencies
extern AncitRgbLed* netLed;

// Internal state variables
static DeviceMode_t currentMode = DEVICE_MODE_BOOT;
static DeviceMode_t previousMode = DEVICE_MODE_BOOT;
static bool modeChangeRequested = false;
static DeviceMode_t requestedMode = DEVICE_MODE_BOOT;

// Sub-state variables
static ConfigModeState_t configState = CONFIG_STATE_UNCONNECTED;
static NormalModeState_t normalState = NORMAL_STATE_WIFI_DISCONNECTED;
static BluetoothModeState_t btState = BT_STATE_UNPAIRED;
static bool editEnabled = false;

// Mode change callback
static ModeChangeCallback_t modeChangeCallback = NULL;

// Task handle
TaskHandle_t DeviceStateManager_task_handle = NULL;

// Mutex for thread-safe LED access
static SemaphoreHandle_t ledMutex = NULL;

// Forward declarations
static void applyLedPattern(void);
static void handleModeTransition(DeviceMode_t newMode);
static void onSwitchEvent(ConfigSwitchEvent_t event);
static void internalModeChangeHandler(DeviceMode_t oldMode, DeviceMode_t newMode);
static DeviceMode_t getRestoreModeFromBluetooth(void);

// Initialize the device state manager
void DeviceStateManager_Init(void) {
  currentMode = DEVICE_MODE_BOOT;
  previousMode = DEVICE_MODE_BOOT;
  modeChangeRequested = false;

  // Create mutex for LED access
  if (ledMutex == NULL) {
    ledMutex = xSemaphoreCreateMutex();
  }

  g_Logger.WriteImmediate(LogLevel::Debug, LogCategory::SETUP, "DeviceStateManager_Init",
                          "Initialized in BOOT mode");
}

// Determine boot mode based on wifi.json and switch state
DeviceMode_t DeviceStateManager_DetermineBootMode(void) {
#ifdef ENABLE_CONFIG_SWITCH
  // Check if config switch is held during boot (highest priority)
  bool switchPressed = ConfigSwitch_IsPressed();

  g_Logger.Write(LogLevel::Info, LogCategory::SETUP, "DeviceStateManager_DetermineBootMode",
                 "Config switch pressed: %s", switchPressed ? "YES" : "NO");

  if (switchPressed) {
    g_Logger.Write(LogLevel::Info, LogCategory::SETUP, "DeviceStateManager_DetermineBootMode",
                   "Switch held during boot -> Forcing CONFIG mode");
    return DEVICE_MODE_CONFIG;
  }
#else
  g_Logger.Write(LogLevel::Info, LogCategory::SETUP, "DeviceStateManager_DetermineBootMode",
                 "Config switch disabled - determining mode from wifi.json only");
#endif

  // Read wifi.json to check sta_enabled flag
  // Note: wifi_doc should already be loaded by read_wifi_file() in main setup
  bool staEnabled = wifi_doc["sta_enabled"] | false;

  g_Logger.Write(LogLevel::Info, LogCategory::SETUP, "DeviceStateManager_DetermineBootMode",
                 "wifi.json sta_enabled: %s", staEnabled ? "true" : "false");

  // Determine mode: NORMAL if sta_enabled=true, CONFIG if false
  return staEnabled ? DEVICE_MODE_NORMAL : DEVICE_MODE_CONFIG;
}

// Set device mode
void DeviceStateManager_SetMode(DeviceMode_t mode) {
  requestedMode = mode;
  modeChangeRequested = true;

  g_Logger.Write(LogLevel::Info, LogCategory::SETUP, "DeviceStateManager",
                 "Mode change requested: %d", mode);
}

// Get current device mode
DeviceMode_t DeviceStateManager_GetMode(void) {
  return currentMode;
}

// Test function - directly apply LED pattern without changing mode
void DeviceStateManager_TestLedPattern(DeviceMode_t mode) {
  currentMode = mode;
  applyLedPattern();

  const char* modeNames[] = {"BOOT", "CONFIG", "NORMAL", "BLUETOOTH"};
  g_Logger.Write(LogLevel::Info, LogCategory::SETUP, "DeviceStateManager_Test",
                 "Testing LED pattern for mode: %s", modeNames[mode]);
}

// Apply LED pattern based on mode and sub-state
static void applyLedPattern(void) {
  // Take mutex before accessing LED (thread-safe)
  if (ledMutex != NULL && xSemaphoreTake(ledMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
    if (!netLed) {
      g_Logger.Write(LogLevel::Warn, LogCategory::SETUP, "DeviceStateManager",
                     "netLed not initialized, cannot set LED pattern");
      xSemaphoreGive(ledMutex);
      return;
    }

    switch (currentMode) {
      case DEVICE_MODE_BOOT:
        // Red LED Fast Blink
        netLed->setState(LedMode::BLINK_FAST, LedColor::RED);
        g_Logger.Write(LogLevel::Debug, LogCategory::SETUP, "DeviceStateManager",
                       "LED: BOOT - Red Fast Blink");
        break;

      case DEVICE_MODE_CONFIG:
        // Sub-state dependent LED
        if (configState == CONFIG_STATE_UNCONNECTED) {
          // Red LED Fast Blink - No client connected
          netLed->setState(LedMode::BLINK_FAST, LedColor::RED);
          g_Logger.Write(LogLevel::Debug, LogCategory::SETUP, "DeviceStateManager",
                         "LED: CONFIG UNCONNECTED - Red Fast Blink");
        } else {
          // Red LED Slow Blink - Client connected
          netLed->setState(LedMode::BLINK_SLOW, LedColor::RED);
          g_Logger.Write(LogLevel::Debug, LogCategory::SETUP, "DeviceStateManager",
                         "LED: CONFIG CONNECTED - Red Slow Blink");
        }
        break;

      case DEVICE_MODE_NORMAL:
        // Sub-state and edit mode dependent LED
        if (normalState == NORMAL_STATE_WIFI_DISCONNECTED) {
          // Blue LED Fast Blink - WiFi disconnected
          netLed->setState(LedMode::BLINK_FAST, LedColor::BLUE);
          g_Logger.Write(LogLevel::Debug, LogCategory::SETUP, "DeviceStateManager",
                         "LED: NORMAL WIFI_DISCONNECTED - Blue Fast Blink");
        } else if (normalState == NORMAL_STATE_WIFI_CONNECTED) {
          // Blue LED Slow Blink - WiFi connected
          netLed->setState(LedMode::BLINK_SLOW, LedColor::BLUE);
          g_Logger.Write(LogLevel::Debug, LogCategory::SETUP, "DeviceStateManager",
                         "LED: NORMAL WIFI_CONNECTED - Blue Slow Blink");
        } else if (normalState == NORMAL_STATE_WIFI_GOT_IP) {
          if (editEnabled) {
            // Blue/Green Alternate - WiFi got IP + edit enabled
            netLed->setState(LedMode::ALTERNATE_SLOW, LedColor::BLUE, LedColor::GREEN);
            g_Logger.Write(LogLevel::Debug, LogCategory::SETUP, "DeviceStateManager",
                           "LED: NORMAL WIFI_GOT_IP EDIT - Blue/Green Alternate");
          } else {
            // Blue LED On - WiFi got IP
            netLed->setState(LedMode::ON, LedColor::BLUE);
            g_Logger.Write(LogLevel::Debug, LogCategory::SETUP, "DeviceStateManager",
                           "LED: NORMAL WIFI_GOT_IP - Blue On");
          }
        }
        break;

      case DEVICE_MODE_BLUETOOTH:
        // Sub-state dependent LED
        if (btState == BT_STATE_UNPAIRED) {
          // Red/Green Fast Blink - Unpaired
          netLed->setState(LedMode::BLINK_FAST, LedColor::GREEN);
          g_Logger.Write(LogLevel::Debug, LogCategory::SETUP, "DeviceStateManager",
                         "LED: BLUETOOTH UNPAIRED - Red/Green Fast Blink");
        } else {
          // Red/Green Slow Blink - Paired
          netLed->setState(LedMode::BLINK_SLOW, LedColor::GREEN);
          g_Logger.Write(LogLevel::Debug, LogCategory::SETUP, "DeviceStateManager",
                         "LED: BLUETOOTH PAIRED - Red/Green Slow Blink");
        }
        break;

      default:
        netLed->setState(LedMode::OFF, LedColor::NONE);
        break;
    }

    xSemaphoreGive(ledMutex);
  } else {
    g_Logger.Write(LogLevel::Warn, LogCategory::SETUP, "DeviceStateManager",
                   "Failed to acquire LED mutex");
  }
}

// Handle mode transitions
static void handleModeTransition(DeviceMode_t newMode) {
  if (newMode == currentMode) {
    g_Logger.Write(LogLevel::Debug, LogCategory::SETUP, "DeviceStateManager",
                   "Already in mode %d, no transition needed", newMode);
    return;
  }

  previousMode = currentMode;
  currentMode = newMode;

  const char* modeNames[] = {"BOOT", "CONFIG", "NORMAL", "BLUETOOTH"};
  g_Logger.Write(LogLevel::Info, LogCategory::SETUP, "DeviceStateManager",
                 "Mode transition: %s -> %s",
                 modeNames[previousMode], modeNames[currentMode]);

  // Initialize sub-states when entering a new mode
  switch (currentMode) {
    case DEVICE_MODE_CONFIG:
      // Initialize to unconnected state
      configState = CONFIG_STATE_UNCONNECTED;
      g_Logger.Write(LogLevel::Info, LogCategory::SETUP, "DeviceStateManager",
                     "Initialized CONFIG mode - TODO: Start WiFi AP mode");
      break;

    case DEVICE_MODE_NORMAL:
      // Initialize to disconnected state, edit disabled
      normalState = NORMAL_STATE_WIFI_DISCONNECTED;
      editEnabled = false;
      g_Logger.Write(LogLevel::Info, LogCategory::SETUP, "DeviceStateManager",
                     "Initialized NORMAL mode - TODO: Start WiFi STA mode");
      break;

    case DEVICE_MODE_BLUETOOTH:
      // Initialize to unpaired state
      btState = BT_STATE_UNPAIRED;
      g_Logger.Write(LogLevel::Info, LogCategory::SETUP, "DeviceStateManager",
                     "Initialized BLUETOOTH mode - TODO: Start BLE mode");
      break;

    default:
      break;
  }

  // Apply LED pattern for new mode and initial sub-state
  applyLedPattern();

  // Notify registered callback of mode change
  if (modeChangeCallback != NULL) {
    modeChangeCallback(previousMode, currentMode);
  }
}

// Switch event callback
static void onSwitchEvent(ConfigSwitchEvent_t event) {
  switch (event) {
    case CFG_SW_EVENT_HOLD:
      // HOLD -> CONFIG mode (from any mode)
      g_Logger.Write(LogLevel::Info, LogCategory::SETUP, "DeviceStateManager",
                     "Switch HOLD detected -> CONFIG mode");
      DeviceStateManager_SetMode(DEVICE_MODE_CONFIG);
      break;

    case CFG_SW_EVENT_SHORT_PRESS:
      // SHORT_PRESS -> Toggle edit mode (only in NORMAL mode)
      if (currentMode == DEVICE_MODE_NORMAL) {
        editEnabled = !editEnabled;
        g_Logger.Write(LogLevel::Info, LogCategory::SETUP, "DeviceStateManager",
                       "Switch SHORT_PRESS detected -> Edit mode %s",
                       editEnabled ? "ENABLED" : "DISABLED");
        // Re-apply LED pattern to reflect edit state change
        applyLedPattern();
      } else {
        g_Logger.Write(LogLevel::Debug, LogCategory::SETUP, "DeviceStateManager",
                       "Switch SHORT_PRESS ignored - not in NORMAL mode");
      }
      break;

    case CFG_SW_EVENT_LONG_PRESS:
      // LONG_PRESS -> Toggle Bluetooth mode
      if (currentMode == DEVICE_MODE_BLUETOOTH) {
        // Exit Bluetooth - restore previous mode
        DeviceMode_t restoreMode = getRestoreModeFromBluetooth();
        g_Logger.Write(LogLevel::Info, LogCategory::SETUP, "DeviceStateManager",
                       "Switch LONG_PRESS - Exit BLUETOOTH, restore to mode %d", restoreMode);
        DeviceStateManager_SetMode(restoreMode);
      } else {
        // Enter Bluetooth mode
        g_Logger.Write(LogLevel::Info, LogCategory::SETUP, "DeviceStateManager",
                       "Switch LONG_PRESS - Enter BLUETOOTH mode");
        DeviceStateManager_SetMode(DEVICE_MODE_BLUETOOTH);
      }
      break;

    default:
      return;  // No action
  }
}

// ========== Public API Functions for Sub-states ==========

// Config state management
void DeviceStateManager_SetConfigState(ConfigModeState_t state) {
  if (currentMode != DEVICE_MODE_CONFIG) {
    g_Logger.Write(LogLevel::Warn, LogCategory::SETUP, "DeviceStateManager",
                   "Cannot set config state - not in CONFIG mode");
    return;
  }

  if (configState != state) {
    configState = state;
    g_Logger.Write(LogLevel::Info, LogCategory::SETUP, "DeviceStateManager",
                   "Config state changed to: %s",
                   state == CONFIG_STATE_CONNECTED ? "CONNECTED" : "UNCONNECTED");
    applyLedPattern();
  }
}

ConfigModeState_t DeviceStateManager_GetConfigState(void) {
  return configState;
}

// Normal state management
void DeviceStateManager_SetNormalState(NormalModeState_t state) {
  if (currentMode != DEVICE_MODE_NORMAL) {
    g_Logger.Write(LogLevel::Warn, LogCategory::SETUP, "DeviceStateManager",
                   "Cannot set normal state - not in NORMAL mode");
    return;
  }

  if (normalState != state) {
    normalState = state;
    const char* stateNames[] = {"WIFI_DISCONNECTED", "WIFI_CONNECTED", "WIFI_GOT_IP"};
    g_Logger.Write(LogLevel::Info, LogCategory::SETUP, "DeviceStateManager",
                   "Normal state changed to: %s", stateNames[state]);
    applyLedPattern();
  }
}

NormalModeState_t DeviceStateManager_GetNormalState(void) {
  return normalState;
}

// Bluetooth state management
void DeviceStateManager_SetBluetoothState(BluetoothModeState_t state) {
  if (currentMode != DEVICE_MODE_BLUETOOTH) {
    g_Logger.Write(LogLevel::Warn, LogCategory::SETUP, "DeviceStateManager",
                   "Cannot set bluetooth state - not in BLUETOOTH mode");
    return;
  }

  if (btState != state) {
    btState = state;
    g_Logger.Write(LogLevel::Info, LogCategory::SETUP, "DeviceStateManager",
                   "Bluetooth state changed to: %s",
                   state == BT_STATE_PAIRED ? "PAIRED" : "UNPAIRED");
    applyLedPattern();
  }
}

BluetoothModeState_t DeviceStateManager_GetBluetoothState(void) {
  return btState;
}

// Edit mode management
void DeviceStateManager_SetEditEnabled(bool enabled) {
  if (currentMode != DEVICE_MODE_NORMAL) {
    g_Logger.Write(LogLevel::Warn, LogCategory::SETUP, "DeviceStateManager",
                   "Cannot set edit mode - not in NORMAL mode");
    return;
  }

  if (editEnabled != enabled) {
    editEnabled = enabled;
    g_Logger.Write(LogLevel::Info, LogCategory::SETUP, "DeviceStateManager",
                   "Edit mode %s", editEnabled ? "ENABLED" : "DISABLED");
    applyLedPattern();
  }
}

bool DeviceStateManager_GetEditEnabled(void) {
  return editEnabled;
}

// Mode change callback registration
void DeviceStateManager_RegisterModeCallback(ModeChangeCallback_t callback) {
  modeChangeCallback = callback;
  g_Logger.Write(LogLevel::Debug, LogCategory::SETUP, "DeviceStateManager",
                 "Callback registered");
}

// ========== Task Functions ==========

// Main state manager task
void DeviceStateManager_Task(void* param) {
  g_Logger.Write(LogLevel::Debug, LogCategory::SETUP, "DeviceStateManager_Task",
                 "Task started on Core %d", xPortGetCoreID());

  // Wait for LED task to be fully initialized
  vTaskDelay(pdMS_TO_TICKS(500));

  // Start in BOOT mode briefly
  applyLedPattern();
  vTaskDelay(pdMS_TO_TICKS(2000));  // Show BOOT LED for 2 seconds

  // Determine and transition to initial operating mode
  DeviceMode_t bootMode = DeviceStateManager_DetermineBootMode();
  handleModeTransition(bootMode);

  // Main loop
  for (;;) {
    // Check for mode change requests
    if (modeChangeRequested) {
      modeChangeRequested = false;
      handleModeTransition(requestedMode);
    }

    // TODO: Add state-specific periodic tasks here
    // For example: Check WiFi connection status, BLE connection status, etc.

    // Feed watchdog
    esp_task_wdt_reset();

    vTaskDelay(pdMS_TO_TICKS(100));  // 100ms cycle
  }
}

// Start the device state manager
void DeviceStateManager_Start(void) {
  // Initialize
  DeviceStateManager_Init();

  // Register internal mode change handler
  DeviceStateManager_RegisterModeCallback(internalModeChangeHandler);

#ifdef ENABLE_CONFIG_SWITCH
  // Register switch callback
  ConfigSwitch_RegisterCallback(onSwitchEvent);
#endif

  // Create task
  BaseType_t result = xTaskCreatePinnedToCore(
      DeviceStateManager_Task,                     // Task function
      "DeviceStateManager_Task",                   // Task name
      ANCIT_DEVICE_STATE_MANAGER_TASK_STACK_SIZE, // Stack size
      NULL,                                        // Parameters
      ANCIT_DEVICE_STATE_MANAGER_TASK_PRIORITY,   // Priority
      &DeviceStateManager_task_handle,             // Task handle
      ANCIT_DEVICE_STATE_MANAGER_TASK_CORE);      // Core affinity

  if (result != pdPASS) {
    g_Logger.WriteImmediate(LogLevel::Error, LogCategory::SETUP, "DeviceStateManager_Start",
                            "FAILED to create task! Error=%d", result);
  }
}

// ========== Helper Functions ==========

// Get the appropriate mode to restore when exiting Bluetooth
static DeviceMode_t getRestoreModeFromBluetooth(void) {
  // Exit Bluetooth - restore previous mode
  DeviceMode_t restoreMode = (previousMode == DEVICE_MODE_BOOT ||
                              previousMode == DEVICE_MODE_BLUETOOTH)
                             ? DEVICE_MODE_NORMAL
                             : previousMode;
  return restoreMode;
}

// Public API to get restore mode (for BLE timeout)
DeviceMode_t DeviceStateManager_GetBluetoothRestoreMode(void) {
  return getRestoreModeFromBluetooth();
}

// ========== Internal Mode Change Handler ==========

// Internal mode change handler - called when device mode changes
// This function orchestrates WiFi and BLE module initialization based on mode
static void internalModeChangeHandler(DeviceMode_t oldMode, DeviceMode_t newMode) {
  const char* modeNames[] = {"BOOT", "CONFIG", "NORMAL", "BLUETOOTH"};
  g_Logger.Write(LogLevel::Info, LogCategory::SETUP, "internalModeChangeHandler",
                 "Handling mode change: %s -> %s",
                 modeNames[oldMode], modeNames[newMode]);

  // Stop old mode services
  switch (oldMode) {
    case DEVICE_MODE_CONFIG:
      g_Logger.Write(LogLevel::Debug, LogCategory::SETUP, "internalModeChangeHandler",
                     "Stopping CONFIG mode services");
      WifiManager_StopSoftAp();
      break;

    case DEVICE_MODE_NORMAL:
      g_Logger.Write(LogLevel::Debug, LogCategory::SETUP, "internalModeChangeHandler",
                     "Stopping NORMAL mode services");
#ifdef ENABLE_MQTT
      MqttClient_Stop();
#endif
      WifiManager_StopStation();
      break;

    case DEVICE_MODE_BLUETOOTH:
#if BLE_ENABLED
      // Stop BLE services
      g_Logger.Write(LogLevel::Debug, LogCategory::SETUP, "internalModeChangeHandler",
                     "Stopping BLUETOOTH mode services");
      if (bleHandler != nullptr) {
        bleHandler->stop();
      }
#endif
      break;

    default:
      break;
  }

  // Start new mode services
  switch (newMode) {
    case DEVICE_MODE_CONFIG:
      g_Logger.Write(LogLevel::Info, LogCategory::SETUP, "internalModeChangeHandler",
                     "Starting WiFi AP mode");
      WifiManager_SetupSoftAp(true);
      break;

    case DEVICE_MODE_NORMAL:
      g_Logger.Write(LogLevel::Info, LogCategory::SETUP, "internalModeChangeHandler",
                     "Starting WiFi STA mode");
      WifiManager_SetupStation(true);

      // Start application tasks after WiFi is ready (only on first boot transition)
      if (oldMode == DEVICE_MODE_BOOT) {
        g_Logger.Write(LogLevel::Info, LogCategory::SETUP, "internalModeChangeHandler",
                       "First boot complete - starting application tasks");
        // Note: Application tasks should be started here after WiFi mode is set
        // For now, this is called from main.cpp after DeviceStateManager_Start()
        // TODO: Move AncitTasks_setup() call here for proper sequencing
      } else {
        // Restart MQTT when re-entering NORMAL mode from other modes
#ifdef ENABLE_MQTT
        if (Device_IsRegistrationValid()) {
          g_Logger.Write(LogLevel::Info, LogCategory::SETUP, "internalModeChangeHandler",
                         "Restarting MQTT for NORMAL mode");
          MqttClient_Restart();
        }
#endif
      }
      break;

    case DEVICE_MODE_BLUETOOTH:
#if BLE_ENABLED
      g_Logger.Write(LogLevel::Info, LogCategory::SETUP, "internalModeChangeHandler",
                     "Starting BLE mode");
      // Start BLE services
      if (bleHandler != nullptr) {
        bleHandler->begin();
      }
#endif
      break;

    default:
      break;
  }
}
