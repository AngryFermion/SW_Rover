/**
 * telematics_manager.h — SmartWheels ESP32-WROOM FOTAmatics
 *
 * Manages the CAN→MQTT telemetry pipeline.
 *
 * In UART mode (USE_DUMMY_DATA not defined):
 *   - Reads lines from Serial1 (shared with FOTA bootloader)
 *   - Automatically pauses while a FOTA transmission is in progress
 *   - Parses "SIGNAL_NAME,VALUE\n" format (decimal or 0xHEX)
 *   - Publishes JSON to SmartKit/Ultra via MQTT
 *
 * In dummy mode (USE_DUMMY_DATA defined):
 *   - Generates synthetic CAN signals at TELEMATICS_DUMMY_INTERVAL_MS
 *   - Serial1 is not touched by this module
 *
 * MQTT control path:
 *   - Incoming messages on SmartKit/Control are written to Serial1 TX by
 *     the MQTT client callback and do not pass through this manager.
 */

#ifndef TELEMATICS_MANAGER_H
#define TELEMATICS_MANAGER_H

#include <Arduino.h>
#include "telematics_config.h"

class TelematicsManager {
public:
    void init();
    void update();   // Call from FreeRTOS task every 50 ms

private:
    // Shared Serial1 — only used when USE_DUMMY_DATA is not defined
#ifndef USE_DUMMY_DATA
    char     lineBuffer[TELEMATICS_LINE_BUFFER_SIZE];
    uint16_t lineBufferIdx;

    void processSerial();
    void parseLine(const char* line);
#endif

#ifdef USE_DUMMY_DATA
    unsigned long lastDummyPublish;
    int           dummySignalIdx;
    long          dummyValues[6];

    void generateDummyData();
#endif

    void publishCANSignal(const char* name, long value);
};

extern TelematicsManager telematicsManager;

// FreeRTOS task entry point (defined in telematics_manager.cpp)
void TelematicsTask(void* pvParams);

#endif // TELEMATICS_MANAGER_H
