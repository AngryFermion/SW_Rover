/**
 * uart_manager.h — SmartWheels ESP32-WROOM Telematics
 *
 * Manages two UART channels:
 *   Logger Channel: System logging/debugging output (UART0 / USB-UART chip)
 *   Data Channel:   Receiving CAN data from MCU (UART1, GPIO16/17)
 *
 * Data source is selected at compile time via uart_config.h:
 *   #define USE_DUMMY_DATA  →  synthetic CAN signals, no UART hardware needed
 *   (undefined)            →  real data from MCU over UART1
 */

#ifndef UART_MANAGER_H
#define UART_MANAGER_H

#include <Arduino.h>
#include <HardwareSerial.h>
#include "../config/uart_config.h"

typedef enum {
    UART_CHANNEL_LOGGER = 0,
    UART_CHANNEL_DATA   = 1
} UARTChannel;

class UARTManager {
public:
    void init();
    void update();

    // Logger channel (UART0)
    void   loggerWrite(const char* message);
    void   loggerPrintf(const char* format, ...);
    void   loggerWriteLine(const char* message);
    int    loggerAvailable();
    int    loggerRead();
    size_t loggerReadBytes(char* buffer, size_t length);
    void   setLoggerEnabled(bool enable);
    bool   isLoggerEnabled();

    // Data channel (UART1) — only active when USE_DUMMY_DATA is NOT defined
    int    dataAvailable();
    int    dataRead();
    int    dataWrite(const char* buffer, size_t length);
    size_t dataReadBytes(uint8_t* buffer, size_t length);

    bool isChannelInitialized(UARTChannel channel);

private:
    HardwareSerial* loggerSerial;

    bool loggerInitialized;

    #if UART_LOGGER_RUNTIME_CONTROL
    bool loggerEnabled;
    #endif

    void initLoggerChannel();

#ifdef USE_DUMMY_DATA
    // Dummy data generator state
    unsigned long dummyLastPublish;
    int           dummySignalIndex;
    long          dummyValues[5];

    void generateDummyData();
#else
    // Real UART data channel
    HardwareSerial* dataSerial;
    bool            dataInitialized;

    char     dataLineBuffer[UART_DATA_LINE_BUFFER_SIZE];
    uint16_t dataLineBufferIndex;

    void initDataChannel();
    void processDataChannel();
    void parseCANDataLine(const char* line);
#endif
};

extern UARTManager uartManager;

// Compile-time logging macros
#if UART_LOGGER_ENABLED
    #define UART_LOG(msg)    uartManager.loggerWrite(msg)
    #define UART_LOG_LN(msg) uartManager.loggerWriteLine(msg)
    #define UART_LOG_F(...)  uartManager.loggerPrintf(__VA_ARGS__)
#else
    #define UART_LOG(msg)    ((void)0)
    #define UART_LOG_LN(msg) ((void)0)
    #define UART_LOG_F(...)  ((void)0)
#endif

#endif // UART_MANAGER_H
