/**
 * SMART_LOGGER.h - Smart Logger Library
 *
 * A lightweight logging system for ESP32 projects
 * with configurable levels and categories
 *
 * Author: Narayana Swamy
 * Date: September 2025
 */

#ifndef SMART_LOGGER_H
#define SMART_LOGGER_H

#include <Arduino.h>

// Log categories (bit flags for filtering)
enum class LogCategory : uint16_t {
    SYSTEM  = 0x0001,
    SETUP   = 0x0002,
    WIFI    = 0x0004,
    MODBUS  = 0x0008,
    SPIFFS  = 0x0010,
    FS      = 0x0020,
    SERVER  = 0x0040,
    LIVE    = 0x0080,
    MQTT    = 0x0100,
    CAN     = 0x0200,
    IO      = 0x0400,
    OTHERS  = 0x0800,
    DEVICE  = 0x1000
};

// Log levels
enum class LogLevel {
    Trace = 0,
    Debug = 1,
    Info = 2,
    Warn = 3,
    Error = 4,
    Fatal = 5
};


extern uint16_t gSetLogCategory;

class SMART_LOGGER {
private:
    bool m_bConsoleEnabled;
    LogLevel m_oLogLevel;

    String getTimeString();
    String getLevelString(LogLevel level);
    String getCategoryString(LogCategory logCategory);

public:
    SMART_LOGGER();
    SMART_LOGGER(LogLevel level);
    ~SMART_LOGGER();

    // Core logging functions
    void Write(LogLevel level, LogCategory logCategory, String method, String message);
    void Write(LogLevel level, LogCategory logCategory, String method, const char* format, ...);

    // Configuration
    void EnableConsole();
    void DisableConsole();

    // Status
    bool IsConsoleEnabled();
};

#endif // SMART_LOGGER_H