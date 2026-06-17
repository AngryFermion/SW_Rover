#ifndef _HEADER_ANCIT_TASKS_H_
#define _HEADER_ANCIT_TASKS_H_

#include <Arduino.h>

// Task Priorities (higher number = higher priority)
typedef enum {
    ANCIT_CONFIG_SWITCH_TASK_PRIORITY = 1,  // User input — low priority
    ANCIT_DEVICE_STATE_MANAGER_TASK_PRIORITY,
    ANCIT_LED_TASK_PRIORITY,
    ANCIT_NEER_TASK_PRIORITY,
    ANCIT_TELEMATICS_TASK_PRIORITY,         // Telematics CAN→MQTT
    ANCIT_MQTT_APP_PRIORITY,
    ANCIT_HTTP_APP_PRIORITY,
    ANCIT_NTP_TASK_PRIORITY,
    ANCIT_WIFI_TASK_PRIORITY,
    ANCIT_MODBUS_TASK_PRIORITY,
    ANCIT_TIMER_TASK_PRIORITY
} AncitTaskPriority_t;

// Task Stack Sizes
#define ANCIT_CONFIG_SWITCH_TASK_STACK_SIZE       (2*1024)
#define ANCIT_DEVICE_STATE_MANAGER_TASK_STACK_SIZE (3*1024)
#define ANCIT_TIMER_TASK_STACK_SIZE               (2*1024)
#define ANCIT_WIFI_TASK_STACK_SIZE                (4*1024)
#define ANCIT_NTP_TASK_STACK_SIZE                 (3*1024)
#define ANCIT_LED_TASK_STACK_SIZE                 (1*1024)
#define ANCIT_MODBUS_TASK_STACK_SIZE              (2*1024)
#define ANCIT_NEER_TASK_STACK_SIZE                (2*1024)
#define ANCIT_MQTT_APP_STACK_SIZE                 (8*1024)
#define ANCIT_HTTP_APP_STACK_SIZE                 (4*1024)
#define ANCIT_TELEMATICS_TASK_STACK_SIZE          (3*1024)  // UART parsing + JSON + MQTT publish

// Task Core Affinity
#define ANCIT_CONFIG_SWITCH_TASK_CORE        1   // User input — Core 1
#define ANCIT_DEVICE_STATE_MANAGER_TASK_CORE 0   // State machine — Core 0
#define ANCIT_TIMER_TASK_CORE                0   // Timers — Core 0
#define ANCIT_WIFI_TASK_CORE                 1   // WiFi — Core 1 (network I/O)
#define ANCIT_NTP_TASK_CORE                  1   // NTP — Core 1
#define ANCIT_LED_TASK_CORE                  0   // LED — Core 0
#define ANCIT_MODBUS_TASK_CORE               0   // Modbus — Core 0
#define ANCIT_NEER_TASK_CORE                 0   // NEER — Core 0
#define ANCIT_MQTT_APP_TASK_CORE             1   // MQTT — Core 1 (network I/O)
#define ANCIT_HTTP_APP_TASK_CORE             1   // HTTP — Core 1 (network I/O)
#define ANCIT_TELEMATICS_TASK_CORE           1   // Telematics — Core 1 (publishes over network)

void AncitTasks_setup(void);

#endif  // _HEADER_ANCIT_TASKS_H_
