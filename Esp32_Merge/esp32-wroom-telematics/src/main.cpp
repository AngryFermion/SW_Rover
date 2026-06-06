#include <Arduino.h>

// Configuration
#include "config/system_config.h"
#include "config/wifi_config.h"
#include "config/uart_config.h"

// Core modules
#include "scheduler/scheduler.h"
#include "managers/wifi_manager.h"
#include "managers/status_manager.h"
#include "managers/uart_manager.h"
#include "managers/mqtt_manager.h"

// Task wrappers for scheduler
void wifiManagerTask()   { wifiManager.update();   }
void statusManagerTask() { statusManager.update();  }
void uartManagerTask()   { uartManager.update();    }
void mqttManagerTask()   { mqttManager.update();    }

void setup() {
    #if ENABLE_SERIAL_DEBUG
        Serial.begin(SERIAL_BAUD_RATE);
        while (!Serial) { ; }
        Serial.println("\n=====================================");
        Serial.println("  SmartWheels ESP32-WROOM Telematics");
        Serial.println("=====================================");
        #ifdef USE_DUMMY_DATA
        Serial.println("  [MODE] Dummy data — no UART needed");
        #else
        Serial.println("  [MODE] Live UART data from MCU");
        #endif
        Serial.println("=====================================\n");
    #endif

    scheduler.init();

    // Initialize managers (order matters)
    uartManager.init();
    statusManager.init();
    wifiManager.init();
    mqttManager.init();

    // Register tasks at 50 ms interval
    scheduler.addTask(uartManagerTask,   50);
    scheduler.addTask(wifiManagerTask,   50);
    scheduler.addTask(mqttManagerTask,   50);
    scheduler.addTask(statusManagerTask, 50);

    #if ENABLE_SERIAL_DEBUG
        uartManager.loggerWriteLine("\n[SYSTEM] All managers initialized");
        uartManager.loggerWriteLine("[SYSTEM] Scheduler started\n");
        uartManager.loggerWriteLine("=====================================\n");
    #endif
}

void loop() {
    scheduler.update();
    delay(1);
}
