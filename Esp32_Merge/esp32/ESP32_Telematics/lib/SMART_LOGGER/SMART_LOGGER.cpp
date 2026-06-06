/**
 * SMART_LOGGER.cpp - Smart Logger Library Implementation
 *
 * Author: Narayana Swamy
 * Date: September 2025
 */

#include "SMART_LOGGER.h"
#include <stdarg.h>
#include <time.h>

uint16_t gSetLogCategory = 0xFFFF; // Enable all categories by default

// Constructor implementations
SMART_LOGGER::SMART_LOGGER() {
    m_oLogLevel = LogLevel::Trace;
    m_bConsoleEnabled = true;
}

SMART_LOGGER::SMART_LOGGER(LogLevel level) {
    m_oLogLevel = level;
    m_bConsoleEnabled = true;
}

SMART_LOGGER::~SMART_LOGGER() {
    // Nothing to clean up for console-only logger
}

// Get current time as string (using time without brackets)
String SMART_LOGGER::getTimeString() {
    time_t now = time(0);
    struct tm* timeinfo = localtime(&now);
    char buffer[20];
    strftime(buffer, sizeof(buffer), "%H:%M:%S", timeinfo);
    return String(buffer);
}

// Convert log level to string
String SMART_LOGGER::getLevelString(LogLevel level) {
    switch (level) {
        case LogLevel::Trace: return "TRACE";
        case LogLevel::Debug: return "DEBUG";
        case LogLevel::Info:  return "INFO";
        case LogLevel::Warn:  return "WARN";
        case LogLevel::Error: return "ERROR";
        case LogLevel::Fatal: return "FATAL";
        default: return "UNKNOWN";
    }
}

// Convert log category to string
String SMART_LOGGER::getCategoryString(LogCategory logCategory) {
    switch (logCategory) {
        case LogCategory::SYSTEM: return "SYS";
        case LogCategory::SETUP:  return "SETUP";
        case LogCategory::WIFI:   return "WIFI";
        case LogCategory::MODBUS: return "MODBUS";
        case LogCategory::SPIFFS: return "SPIFFS";
        case LogCategory::FS:     return "FS";
        case LogCategory::SERVER: return "SERVER";
        case LogCategory::LIVE:   return "LIVE";
        case LogCategory::MQTT:   return "MQTT";
        case LogCategory::CAN:    return "CAN";
        case LogCategory::IO:     return "IO";
        case LogCategory::OTHERS: return "OTHER";
        case LogCategory::DEVICE: return "DEV";
        default: return "UNK";
    }
}

// Core logging function with string message
void SMART_LOGGER::Write(LogLevel level, LogCategory logCategory, String method, String message) {
    if (level < m_oLogLevel || !m_bConsoleEnabled) return;

    String timeStr = getTimeString();
    String levelStr = getLevelString(level);
    String categoryStr = getCategoryString(logCategory);

    // Check if this category should be logged
    if (level >= LogLevel::Warn || (gSetLogCategory & static_cast<uint16_t>(logCategory)) > 0) {
        Serial.print(timeStr);
        Serial.print("|");
        Serial.print(levelStr);
        Serial.print("|");
        Serial.print(categoryStr);
        Serial.print("|");
        Serial.print(method);
        Serial.print("|");
        Serial.println(message);
    }
}

// Core logging function with printf-style formatting
void SMART_LOGGER::Write(LogLevel level, LogCategory logCategory, String method, const char* format, ...) {
    if (level < m_oLogLevel || !m_bConsoleEnabled) return;

    char buffer[512];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    Write(level, logCategory, method, String(buffer));
}

// Configuration functions
void SMART_LOGGER::EnableConsole() {
    m_bConsoleEnabled = true;
}

void SMART_LOGGER::DisableConsole() {
    m_bConsoleEnabled = false;
}

// Status functions
bool SMART_LOGGER::IsConsoleEnabled() {
    return m_bConsoleEnabled;
}