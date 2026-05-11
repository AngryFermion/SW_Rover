#ifndef _HEADER_ANCIT_TASKS_H_
#define _HEADER_ANCIT_TASKS_H_
#include <Arduino.h>

// Task Priorities
// Order of priority is important, higher numbers indicate higher priority
// The priorities are used to determine the scheduling of tasks in FreeRTOS.
typedef enum {
  ANCIT_CONFIG_SWITCH_TASK_PRIORITY = 1,  // Low priority - user input
  ANCIT_DEVICE_STATE_MANAGER_TASK_PRIORITY,
  ANCIT_LED_TASK_PRIORITY,
  ANCIT_NEER_TASK_PRIORITY,
  ANCIT_MQTT_APP_PRIORITY,
  ANCIT_HTTP_APP_PRIORITY,
  ANCIT_NTP_TASK_PRIORITY,
  ANCIT_WIFI_TASK_PRIORITY,
  ANCIT_MODBUS_TASK_PRIORITY,
  ANCIT_TIMER_TASK_PRIORITY
} AncitTaskPriority_t;

// Task Stack Sizes
#define ANCIT_CONFIG_SWITCH_TASK_STACK_SIZE (2*1024)
#define ANCIT_DEVICE_STATE_MANAGER_TASK_STACK_SIZE (3*1024)  // Calls WiFi/BLE init
#define ANCIT_TIMER_TASK_STACK_SIZE (2*1024)
#define ANCIT_WIFI_TASK_STACK_SIZE (4*1024)  // WiFi stack needs 4KB (deep ESP-IDF calls)
#define ANCIT_NTP_TASK_STACK_SIZE (3*1024)   // Network + parsing
#define ANCIT_LED_TASK_STACK_SIZE (1*1024)
#define ANCIT_MODBUS_TASK_STACK_SIZE (2*1024)
#define ANCIT_NEER_TASK_STACK_SIZE (2*1024)
#define ANCIT_MQTT_APP_STACK_SIZE (4*1024)
#define ANCIT_HTTP_APP_STACK_SIZE (4*1024)

// Task Core Affinity
// Core 0: Arduino loop + system tasks + time-critical operations
// Core 1: Network I/O + communication + user input
#define ANCIT_CONFIG_SWITCH_TASK_CORE 1        // User input - Core 1
#define ANCIT_DEVICE_STATE_MANAGER_TASK_CORE 0 // State machine - Core 0 (with loop)
#define ANCIT_TIMER_TASK_CORE 0                // Timers - Core 0
#define ANCIT_WIFI_TASK_CORE 1                 // WiFi - Core 1 (network I/O)
#define ANCIT_NTP_TASK_CORE NULL               // NTP - ANY CORE
#define ANCIT_LED_TASK_CORE 0                  // LED - Core 0 (visual feedback)
#define ANCIT_MODBUS_TASK_CORE 0               // Modbus - Core 0 (serial/timing critical)
#define ANCIT_NEER_TASK_CORE 0                 // NEER - Core 0 (data processing)
#define ANCIT_MQTT_APP_TASK_CORE 1             // MQTT - Core 1 (network I/O)
#define ANCIT_HTTP_APP_TASK_CORE 1             // HTTP - Core 1 (network I/O)

void AncitTasks_setup(void);

#endif  //_HEADER_ANCIT_TASKS_H_