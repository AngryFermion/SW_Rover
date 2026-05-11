#ifndef _HEADER_ANCIT_CONFIG_SWITCH_H_
#define _HEADER_ANCIT_CONFIG_SWITCH_H_

#include <Arduino.h>

#ifdef __cplusplus
extern "C" {
#endif

// Timing Configuration (all in milliseconds)
// To customize: Simply change the values below and recompile
#define CFG_SW_TASK_DELAY_MS 25        // Task polling interval
#define CFG_SW_DEBOUNCE_TIME 50       // Debounce delay (must be > task delay for proper debouncing)
#define CFG_SW_SHORT_PRESS_MAX 750     // Short press: < 1 second
#define CFG_SW_LONG_PRESS_MAX 3000     // Long press: 1-3 seconds
#define CFG_SW_HOLD_MIN 5000           // Hold: > 5 seconds

// Switch Event Types
typedef enum {
  CFG_SW_EVENT_NONE = 0,
  CFG_SW_EVENT_SHORT_PRESS,      // < 1 second
  CFG_SW_EVENT_LONG_PRESS,       // 1-3 seconds
  CFG_SW_EVENT_HOLD              // > 5 seconds (triggered while held)
} ConfigSwitchEvent_t;

// Callback function type
typedef void (*ConfigSwitchCallback_t)(ConfigSwitchEvent_t event);

// Functions
void ConfigSwitch_Init(void);
void ConfigSwitch_Task(void *param);
void ConfigSwitch_RegisterCallback(ConfigSwitchCallback_t callback);
ConfigSwitchEvent_t ConfigSwitch_GetLastEvent(void);

// Boot-time detection (call before task starts)
bool ConfigSwitch_IsPressed(void);

// Integrated example functions
void ConfigSwitch_EventCallback(ConfigSwitchEvent_t event);
void ConfigSwitch_Start(void);

#ifdef __cplusplus
}
#endif

#endif  // _HEADER_ANCIT_CONFIG_SWITCH_H_
