/**
 * uart_manager.cpp — SmartWheels ESP32-WROOM Telematics
 */

#include "uart_manager.h"
#include "mqtt_manager.h"
#include "../config/system_config.h"
#include <stdarg.h>

UARTManager uartManager;

// ============================================================================
// Dummy signal table — used only when USE_DUMMY_DATA is defined
// ============================================================================
#ifdef USE_DUMMY_DATA

static const char* DUMMY_SIGNAL_NAMES[]  = {"Speed", "ENGINE_RPM", "THROTTLE", "BRAKE", "STEERING_ANGLE"};
static const long  DUMMY_SIGNAL_MIN[]    = {0,    800,  0,  0, -90};
static const long  DUMMY_SIGNAL_MAX[]    = {120, 6000, 100, 100, 90};
static const long  DUMMY_SIGNAL_STEP[]   = {5,   200,   5,   10, 15};
static const int   DUMMY_SIGNAL_COUNT    = 5;

#endif // USE_DUMMY_DATA

// ============================================================================
// init / update
// ============================================================================

void UARTManager::init() {
    loggerInitialized = false;
    loggerSerial      = nullptr;

    #if UART_LOGGER_RUNTIME_CONTROL
    loggerEnabled = true;
    #endif

    #if ENABLE_SERIAL_DEBUG
    Serial.println("[UART Manager] Initializing...");
    #endif

    initLoggerChannel();

#ifdef USE_DUMMY_DATA
    dummyLastPublish  = 0;
    dummySignalIndex  = 0;
    for (int i = 0; i < DUMMY_SIGNAL_COUNT; i++) {
        dummyValues[i] = DUMMY_SIGNAL_MIN[i];
    }
    #if ENABLE_SERIAL_DEBUG
    Serial.println("[UART Manager] Dummy data mode — UART1 not initialized");
    Serial.printf("[UART Manager] Publish interval: %d ms | Signals: %d\n",
                  DUMMY_DATA_INTERVAL_MS, DUMMY_SIGNAL_COUNT);
    #endif
#else
    dataInitialized     = false;
    dataSerial          = nullptr;
    dataLineBufferIndex = 0;
    memset(dataLineBuffer, 0, UART_DATA_LINE_BUFFER_SIZE);
    initDataChannel();
#endif

    #if ENABLE_SERIAL_DEBUG
    Serial.println("[UART Manager] Initialization complete");
    #endif
}

void UARTManager::update() {
#ifdef USE_DUMMY_DATA
    generateDummyData();
#else
    if (dataInitialized) {
        processDataChannel();
    }
#endif
}

// ============================================================================
// Logger Channel (UART0 / USB-UART chip)
// ============================================================================

void UARTManager::initLoggerChannel() {
    loggerSerial = &Serial;
    if (!Serial) {
        Serial.begin(UART_LOGGER_BAUD);
        delay(100);
    }
    loggerInitialized = true;

    #if ENABLE_SERIAL_DEBUG
    Serial.println("[UART Manager] Logger channel (UART0) initialized");
    Serial.printf("[UART Manager] Logger BAUD: %d\n", UART_LOGGER_BAUD);
    #endif
}

void UARTManager::loggerWrite(const char* message) {
    #if UART_LOGGER_RUNTIME_CONTROL
    if (!loggerEnabled) return;
    #endif
    if (loggerInitialized && loggerSerial) {
        loggerSerial->print(message);
    }
}

void UARTManager::loggerPrintf(const char* format, ...) {
    #if UART_LOGGER_RUNTIME_CONTROL
    if (!loggerEnabled) return;
    #endif
    if (!loggerInitialized || !loggerSerial) return;

    char buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    loggerSerial->print(buffer);
}

void UARTManager::loggerWriteLine(const char* message) {
    #if UART_LOGGER_RUNTIME_CONTROL
    if (!loggerEnabled) return;
    #endif
    if (loggerInitialized && loggerSerial) {
        loggerSerial->println(message);
    }
}

int UARTManager::loggerAvailable() {
    return (loggerInitialized && loggerSerial) ? loggerSerial->available() : 0;
}

int UARTManager::loggerRead() {
    return (loggerInitialized && loggerSerial) ? loggerSerial->read() : -1;
}

size_t UARTManager::loggerReadBytes(char* buffer, size_t length) {
    return (loggerInitialized && loggerSerial) ? loggerSerial->readBytes(buffer, length) : 0;
}

void UARTManager::setLoggerEnabled(bool enable) {
    #if UART_LOGGER_RUNTIME_CONTROL
    loggerEnabled = enable;
    #if ENABLE_SERIAL_DEBUG
    Serial.printf("[UART Manager] Logger %s\n", enable ? "ENABLED" : "DISABLED");
    #endif
    #endif
}

bool UARTManager::isLoggerEnabled() {
    #if UART_LOGGER_RUNTIME_CONTROL
    return loggerEnabled;
    #else
    return UART_LOGGER_ENABLED;
    #endif
}

bool UARTManager::isChannelInitialized(UARTChannel channel) {
    if (channel == UART_CHANNEL_LOGGER) return loggerInitialized;
#ifdef USE_DUMMY_DATA
    return false; // UART1 not initialized in dummy mode
#else
    return dataInitialized;
#endif
}

// ============================================================================
// Data Channel stubs — return safe no-ops in dummy mode
// ============================================================================

int UARTManager::dataAvailable() {
#ifdef USE_DUMMY_DATA
    return 0;
#else
    return (dataInitialized && dataSerial) ? dataSerial->available() : 0;
#endif
}

int UARTManager::dataRead() {
#ifdef USE_DUMMY_DATA
    return -1;
#else
    return (dataInitialized && dataSerial) ? dataSerial->read() : -1;
#endif
}

int UARTManager::dataWrite(const char* buffer, size_t length) {
#ifdef USE_DUMMY_DATA
    return 0;
#else
    return (dataInitialized && dataSerial) ? dataSerial->write(buffer, length) : 0;
#endif
}

size_t UARTManager::dataReadBytes(uint8_t* buffer, size_t length) {
#ifdef USE_DUMMY_DATA
    return 0;
#else
    return (dataInitialized && dataSerial) ? dataSerial->readBytes(buffer, length) : 0;
#endif
}

// ============================================================================
// USE_DUMMY_DATA path — synthetic CAN signal generator
// ============================================================================

#ifdef USE_DUMMY_DATA

void UARTManager::generateDummyData() {
    unsigned long now = millis();
    if (now - dummyLastPublish < DUMMY_DATA_INTERVAL_MS) return;
    dummyLastPublish = now;

    // Advance the current signal's value, wrap at max back to min
    dummyValues[dummySignalIndex] += DUMMY_SIGNAL_STEP[dummySignalIndex];
    if (dummyValues[dummySignalIndex] > DUMMY_SIGNAL_MAX[dummySignalIndex]) {
        dummyValues[dummySignalIndex] = DUMMY_SIGNAL_MIN[dummySignalIndex];
    }

    const char* name  = DUMMY_SIGNAL_NAMES[dummySignalIndex];
    long        value = dummyValues[dummySignalIndex];

    #if ENABLE_SERIAL_DEBUG
    Serial.printf("[UART Dummy] %s = %ld\n", name, value);
    #endif

    mqttManager.publishCANSignal(name, value);

    // Rotate to next signal on each tick
    dummySignalIndex = (dummySignalIndex + 1) % DUMMY_SIGNAL_COUNT;
}

// ============================================================================
// REAL UART path — UART1 data channel
// ============================================================================

#else

void UARTManager::initDataChannel() {
    dataSerial = new HardwareSerial(UART_DATA_NUM);

    #if ENABLE_SERIAL_DEBUG
    Serial.printf("[UART Manager] Initializing UART%d...\n", UART_DATA_NUM);
    Serial.printf("[UART Manager] RX: GPIO%d, TX: GPIO%d, BAUD: %d\n",
                  UART_DATA_RX_PIN, UART_DATA_TX_PIN, UART_DATA_BAUD);
    #endif

    dataSerial->begin(UART_DATA_BAUD, SERIAL_8N1, UART_DATA_RX_PIN, UART_DATA_TX_PIN);
    dataSerial->setTimeout(UART_READ_TIMEOUT_MS);
    delay(100);
    dataInitialized = true;

    #if ENABLE_SERIAL_DEBUG
    Serial.println("[UART Manager] Data channel (UART1) initialized successfully");
    #endif
}

void UARTManager::processDataChannel() {
    #if ENABLE_SERIAL_DEBUG
    static unsigned long lastDebugTime = 0;
    if (millis() - lastDebugTime >= 5000) {
        Serial.printf("[UART Debug] Data channel active, bytes available: %d\n",
                      dataSerial->available());
        lastDebugTime = millis();
    }
    #endif

    while (dataSerial->available() > 0) {
        char incomingByte = dataSerial->read();

        if (incomingByte == '\n') {
            if (dataLineBufferIndex > 0) {
                dataLineBuffer[dataLineBufferIndex] = '\0';
                parseCANDataLine(dataLineBuffer);
                dataLineBufferIndex = 0;
            }
        } else if (incomingByte == '\r') {
            // skip carriage return
        } else {
            if (dataLineBufferIndex < UART_DATA_LINE_BUFFER_SIZE - 1) {
                dataLineBuffer[dataLineBufferIndex++] = incomingByte;
            } else {
                #if ENABLE_SERIAL_DEBUG
                Serial.println("[UART Manager] ERROR: Data line buffer overflow!");
                #endif
                dataLineBufferIndex = 0;
            }
        }
    }
}

// Parse a complete line: SIGNAL_NAME,VALUE
// VALUE may be decimal (255) or hexadecimal (0xFF / FF)
void UARTManager::parseCANDataLine(const char* line) {
    const char* comma = strchr(line, ',');
    if (comma == nullptr) {
        #if ENABLE_SERIAL_DEBUG
        Serial.printf("[UART Manager] ERROR: Invalid format (no comma): %s\n", line);
        #endif
        return;
    }

    int nameLen = comma - line;
    if (nameLen <= 0 || nameLen >= 64) {
        #if ENABLE_SERIAL_DEBUG
        Serial.printf("[UART Manager] ERROR: Invalid signal name length: %s\n", line);
        #endif
        return;
    }

    char signalName[64];
    strncpy(signalName, line, nameLen);
    signalName[nameLen] = '\0';

    const char* dataStr = comma + 1;
    long dataValue;

    if (dataStr[0] == '0' && (dataStr[1] == 'x' || dataStr[1] == 'X')) {
        dataValue = strtol(dataStr, nullptr, 16);
    } else if (strchr(dataStr, 'x') != nullptr || strchr(dataStr, 'X') != nullptr) {
        dataValue = strtol(dataStr, nullptr, 16);
    } else {
        dataValue = strtol(dataStr, nullptr, 10);
    }

    #if ENABLE_SERIAL_DEBUG
    Serial.printf("[UART Data] %s = %ld (0x%lX)\n", signalName, dataValue, dataValue);
    #endif

    mqttManager.publishCANSignal(signalName, dataValue);
}

#endif // USE_DUMMY_DATA
