/**
 * telematics_manager.cpp — SmartWheels ESP32-WROOM FOTAmatics
 */

#include "telematics_manager.h"
#include <ancit_fota_handler.h>
#include <ancit_fota_serial_handler.h>
#include <ancit_mqtt_client.h>
#include <BT_LOGGER.h>
#include <ArduinoJson.h>
#include <esp_task_wdt.h>
#include "app_config.h"

TelematicsManager telematicsManager;

// ============================================================================
// Dummy signal table
// ============================================================================
#ifdef USE_DUMMY_DATA

static const char* DUMMY_NAMES[]  = {"Speed", "ENGINE_RPM", "THROTTLE", "BRAKE", "STEERING_ANGLE", "Distance"};
static const long  DUMMY_MIN[]    = {0,    800,  0,   0, -90,  20};
static const long  DUMMY_MAX[]    = {120, 6000, 100, 100,  90, 400};
static const long  DUMMY_STEP[]   = {5,    200,   5,   10,  15,  20};
static const int   DUMMY_COUNT    = 6;

#endif // USE_DUMMY_DATA

// ============================================================================
// init
// ============================================================================

void TelematicsManager::init() {
#ifdef USE_DUMMY_DATA
    lastDummyPublish = 0;
    dummySignalIdx   = 0;
    for (int i = 0; i < DUMMY_COUNT; i++) dummyValues[i] = DUMMY_MIN[i];

    g_Logger.Write(LogLevel::Info, LogCategory::OTHERS, "TelematicsManager::init",
                   "Dummy data mode — Serial1 not used by telematics");
    g_Logger.Write(LogLevel::Info, LogCategory::OTHERS, "TelematicsManager::init",
                   "Interval: %d ms | Signals: %d", TELEMATICS_DUMMY_INTERVAL_MS, DUMMY_COUNT);
#else
    lineBufferIdx = 0;
    memset(lineBuffer, 0, TELEMATICS_LINE_BUFFER_SIZE);

    // Serial1 is also used by the FOTA serial handler (same baud/pins).
    // Begin here so telematics is ready at boot; FOTA handler will also call
    // begin() when it initialises — calling begin() twice at identical settings
    // is safe on ESP-IDF.
    TELEMATICS_SERIAL.begin(TELEMATICS_SERIAL_BAUD, SERIAL_8N1,
                            TELEMATICS_SERIAL_RX_PIN, TELEMATICS_SERIAL_TX_PIN);

    g_Logger.Write(LogLevel::Info, LogCategory::OTHERS, "TelematicsManager::init",
                   "UART mode — Serial1 RX:GPIO%d TX:GPIO%d @ %d baud",
                   TELEMATICS_SERIAL_RX_PIN, TELEMATICS_SERIAL_TX_PIN,
                   TELEMATICS_SERIAL_BAUD);
#endif

    g_Logger.Write(LogLevel::Info, LogCategory::OTHERS, "TelematicsManager::init",
                   "Telematics manager initialized. Pub: %s | Sub: %s",
                   TELEMATICS_TOPIC_CAN_SIGNALS, TELEMATICS_TOPIC_CONTROL);
}

// ============================================================================
// update — called every 50 ms from the FreeRTOS task
// ============================================================================

void TelematicsManager::update() {
#if FOTAMATICS_MODE == APP_MODE_BOTH
    // Pause telematics only while FOTA chunks are downloading — MQTT bandwidth is
    // needed for chunk ACKs. Once the hex lands on SPIFFS (READY_TO_TRANSMIT),
    // MQTT and Serial1 are both free so telematics can resume.
    // Serial1 contention during the bootload flash is handled below by
    // ancitFotaSerialHandler.isTransmissionInProgress().
    if (fotaHandler.getState() == FOTA_RECEIVING) return;
#endif

#ifdef USE_DUMMY_DATA
    generateDummyData();
#else
    // Do not read Serial1 while FOTA bootloader is using it
    if (ancitFotaSerialHandler.isTransmissionInProgress()) return;
    processSerial();
#endif
}

// ============================================================================
// UART path — parse SIGNAL_NAME,VALUE lines from MCU
// ============================================================================

#ifndef USE_DUMMY_DATA

void TelematicsManager::processSerial() {
    // Process at most ONE complete line per update() tick.
    // Draining all available bytes in one shot triggers back-to-back
    // MqttClient_Publish() calls without yielding, starving MqttClient_Loop()
    // of the mutex and causing the broker to drop the connection.
    // The 50 ms task delay between ticks keeps the UART buffer clear.
    while (TELEMATICS_SERIAL.available() > 0) {
        char c = TELEMATICS_SERIAL.read();

        if (c == '\n') {
            if (lineBufferIdx > 0) {
                lineBuffer[lineBufferIdx] = '\0';
                parseLine(lineBuffer);
                lineBufferIdx = 0;
                break;  // yield — next line handled on next tick
            }
        } else if (c == '\r') {
            // skip
        } else {
            if (lineBufferIdx < TELEMATICS_LINE_BUFFER_SIZE - 1) {
                lineBuffer[lineBufferIdx++] = c;
            } else {
                g_Logger.Write(LogLevel::Warn, LogCategory::OTHERS, "TelematicsManager",
                               "Line buffer overflow — resetting");
                lineBufferIdx = 0;
            }
        }
    }
}

// Parse "SIGNAL_NAME,VALUE\0" — VALUE may be decimal or 0xHEX / HEX
void TelematicsManager::parseLine(const char* line) {
    const char* comma = strchr(line, ',');
    if (!comma) {
        g_Logger.Write(LogLevel::Warn, LogCategory::OTHERS, "TelematicsManager",
                       "Invalid line (no comma): %s", line);
        return;
    }

    int nameLen = comma - line;
    if (nameLen <= 0 || nameLen >= 64) return;

    char signalName[64];
    strncpy(signalName, line, nameLen);
    signalName[nameLen] = '\0';

    const char* valStr = comma + 1;
    long value;
    if ((valStr[0] == '0' && (valStr[1] == 'x' || valStr[1] == 'X')) ||
        strchr(valStr, 'x') || strchr(valStr, 'X')) {
        value = strtol(valStr, nullptr, 16);
    } else {
        value = strtol(valStr, nullptr, 10);
    }

    publishCANSignal(signalName, value);
}

#endif // !USE_DUMMY_DATA

// ============================================================================
// Dummy data path
// ============================================================================

#ifdef USE_DUMMY_DATA

void TelematicsManager::generateDummyData() {
    unsigned long now = millis();
    if (now - lastDummyPublish < TELEMATICS_DUMMY_INTERVAL_MS) return;
    lastDummyPublish = now;

    dummyValues[dummySignalIdx] += DUMMY_STEP[dummySignalIdx];
    if (dummyValues[dummySignalIdx] > DUMMY_MAX[dummySignalIdx]) {
        dummyValues[dummySignalIdx] = DUMMY_MIN[dummySignalIdx];
    }

    g_Logger.Write(LogLevel::Debug, LogCategory::OTHERS, "TelematicsManager",
                   "Dummy: %s = %ld", DUMMY_NAMES[dummySignalIdx], dummyValues[dummySignalIdx]);

    publishCANSignal(DUMMY_NAMES[dummySignalIdx], dummyValues[dummySignalIdx]);
    dummySignalIdx = (dummySignalIdx + 1) % DUMMY_COUNT;
}

#endif // USE_DUMMY_DATA

// ============================================================================
// Publish a CAN signal as JSON to SmartKit/Ultra
// ============================================================================

void TelematicsManager::publishCANSignal(const char* name, long value) {
    if (!MqttClient_IsConnected()) return;

    StaticJsonDocument<TELEMATICS_JSON_BUFFER_SIZE> doc;
    doc["device_id"] = TELEMATICS_DEVICE_ID;
    doc["signal"]    = name;
    doc["value"]     = value;
    doc["timestamp"] = millis();

    char payload[TELEMATICS_JSON_BUFFER_SIZE];
    serializeJson(doc, payload, sizeof(payload));

    MqttClient_Publish(TELEMATICS_TOPIC_CAN_SIGNALS, payload, false);
}

// ============================================================================
// FreeRTOS task
// ============================================================================

void TelematicsTask(void* pvParams) {
    esp_task_wdt_add(NULL);

    telematicsManager.init();

    const TickType_t xDelay = pdMS_TO_TICKS(50);

    while (true) {
        esp_task_wdt_reset();
        telematicsManager.update();
        vTaskDelay(xDelay);
    }
}
