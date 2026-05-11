#ifndef _DEVICE_STATE_MANAGER_H_
#define _DEVICE_STATE_MANAGER_H_

#include <Arduino.h>

#ifdef __cplusplus
extern "C" {
#endif

// Device Operating Modes
typedef enum {
  DEVICE_MODE_BOOT = 0,          // Red LED Blinks Fast - Initial boot state
  DEVICE_MODE_CONFIG,            // WiFi AP mode - Configuration mode
  DEVICE_MODE_NORMAL,            // WiFi STA mode - Normal operation
  DEVICE_MODE_BLUETOOTH          // BLE mode - Bluetooth configuration
} DeviceMode_t;

// Config Mode Sub-states
typedef enum {
  CONFIG_STATE_UNCONNECTED = 0,  // Red LED Fast Blink - No client connected to AP
  CONFIG_STATE_CONNECTED         // Red LED Slow Blink - Client connected to AP
} ConfigModeState_t;

// Normal Mode Sub-states
typedef enum {
  NORMAL_STATE_WIFI_DISCONNECTED = 0,  // Blue LED Fast Blink - WiFi disconnected
  NORMAL_STATE_WIFI_CONNECTED,         // Blue LED Slow Blink - WiFi connected
  NORMAL_STATE_WIFI_GOT_IP             // Blue LED On (or Blue/Green alternate if edit enabled)
} NormalModeState_t;

// Bluetooth Mode Sub-states
typedef enum {
  BT_STATE_UNPAIRED = 0,         // Red/Green Fast Blink - Not paired
  BT_STATE_PAIRED                // Red/Green Slow Blink - Paired
} BluetoothModeState_t;

// Mode change callback function type
typedef void (*ModeChangeCallback_t)(DeviceMode_t oldMode, DeviceMode_t newMode);

// Functions - Main Mode Control
void DeviceStateManager_Init(void);
void DeviceStateManager_Start(void);
void DeviceStateManager_SetMode(DeviceMode_t mode);
DeviceMode_t DeviceStateManager_GetMode(void);
void DeviceStateManager_Task(void* param);

// Sub-state Control Functions
void DeviceStateManager_SetConfigState(ConfigModeState_t state);
ConfigModeState_t DeviceStateManager_GetConfigState(void);

void DeviceStateManager_SetNormalState(NormalModeState_t state);
NormalModeState_t DeviceStateManager_GetNormalState(void);

void DeviceStateManager_SetBluetoothState(BluetoothModeState_t state);
BluetoothModeState_t DeviceStateManager_GetBluetoothState(void);

// Edit Mode Control (only applicable in NORMAL mode)
void DeviceStateManager_SetEditEnabled(bool enabled);
bool DeviceStateManager_GetEditEnabled(void);

// Mode Change Callback Registration
void DeviceStateManager_RegisterModeCallback(ModeChangeCallback_t callback);

// Test/Debug function - manually trigger LED pattern for a mode
void DeviceStateManager_TestLedPattern(DeviceMode_t mode);

// Internal helper - determines mode from wifi.json on startup
DeviceMode_t DeviceStateManager_DetermineBootMode(void);

// Get the appropriate mode to restore when exiting Bluetooth mode
DeviceMode_t DeviceStateManager_GetBluetoothRestoreMode(void);

#ifdef __cplusplus
}
#endif

#endif // _DEVICE_STATE_MANAGER_H_
