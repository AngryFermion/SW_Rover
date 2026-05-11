/**
 * BT_LOGGER.cpp - Bluetooth Logger Library Implementation
 *
 * Author: Narayana Swamy
 * Date: September 2025
 */

#include "BT_LOGGER.h"
#include <stdarg.h>
#include <time.h>

uint16_t gSetLogCategory = 0xFFFF; // Enable all categories by default

// Global BT_LOGGER instance
BT_LOGGER g_Logger;

// Static queue storage
LogEntry BT_LOGGER::logQueue[BT_LOGGER::LOG_QUEUE_SIZE];
volatile int BT_LOGGER::queueHead = 0;
volatile int BT_LOGGER::queueTail = 0;
volatile int BT_LOGGER::queueCount = 0;

// Static callback storage
LogOutputCallback* BT_LOGGER::logCallback = nullptr;

// Constructor implementations
BT_LOGGER::BT_LOGGER() {
    m_oLogLevel = LogLevel::Trace;
    m_bConsoleEnabled = true;
    m_bUseRealTime = false;  // Default to uptime
}

BT_LOGGER::BT_LOGGER(LogLevel level) {
    m_oLogLevel = level;
    m_bConsoleEnabled = true;
    m_bUseRealTime = false;  // Default to uptime
}

BT_LOGGER::~BT_LOGGER() {
    // Nothing to clean up for console-only logger
}

// Get uptime string from millis()
String BT_LOGGER::getUptimeString() {
    unsigned long ms = millis();
    unsigned long seconds = ms / 1000;
    unsigned long minutes = seconds / 60;
    unsigned long hours = minutes / 60;

    seconds %= 60;
    minutes %= 60;
    hours %= 24;  // Wrap at 24 hours

    char buffer[20];
    snprintf(buffer, sizeof(buffer), "%02lu:%02lu:%02lu", hours, minutes, seconds);
    return String(buffer);
}

// Get real-time string from NTP synced time
String BT_LOGGER::getRealTimeString() {
    time_t now = time(0);
    struct tm timeinfo;
    localtime_r(&now, &timeinfo);  // Thread-safe version
    char buffer[20];
    strftime(buffer, sizeof(buffer), "%H:%M:%S", &timeinfo);
    return String(buffer);
}

// Get current time as string (respects m_bUseRealTime flag)
String BT_LOGGER::getTimeString() {
    return m_bUseRealTime ? getRealTimeString() : getUptimeString();
}

// Convert log level to string (auto-generated from LOG_LEVELS_LIST)
String BT_LOGGER::getLevelString(LogLevel level) {
    switch (level) {
        #define LOG_LEVEL(name, value, str) case LogLevel::name: { \
            String s = String(str); \
            s.toUpperCase(); \
            return s; \
        }
        LOG_LEVELS_LIST
        #undef LOG_LEVEL
        default: return "UNKNOWN";
    }
}

// Convert log category to string (auto-generated from LOG_CATEGORIES_LIST)
String BT_LOGGER::getCategoryString(LogCategory logCategory) {
    switch (logCategory) {
        #define LOG_CATEGORY(name, value, str) case LogCategory::name: return String(str).substring(0, 6);
        LOG_CATEGORIES_LIST
        #undef LOG_CATEGORY
        default: return "UNK";
    }
}

// Core logging function with string message
void BT_LOGGER::Write(LogLevel level, LogCategory logCategory, String method, String message) {
    if (level < m_oLogLevel || !m_bConsoleEnabled) return;

    String timeStr = getTimeString();
    String levelStr = getLevelString(level);
    String categoryStr = getCategoryString(logCategory);

    // Check if this category should be logged
    // Only log if the category is enabled in the mask
    if ((gSetLogCategory & static_cast<uint16_t>(logCategory)) > 0) {
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

// Push log entry to queue (safe for callback/ISR context)
bool BT_LOGGER::pushLogEntry(LogLevel level, LogCategory category, const char* method, const char* message) {
    if (queueCount >= LOG_QUEUE_SIZE) {
        return false; // Queue full, drop message
    }

    LogEntry* entry = &logQueue[queueTail];
    entry->level = level;
    entry->category = category;
    entry->timestamp = millis();

    // Copy strings safely
    strncpy(entry->method, method, sizeof(entry->method) - 1);
    entry->method[sizeof(entry->method) - 1] = '\0';

    strncpy(entry->message, message, sizeof(entry->message) - 1);
    entry->message[sizeof(entry->message) - 1] = '\0';

    queueTail = (queueTail + 1) % LOG_QUEUE_SIZE;
    queueCount++;

    return true;
}

// Process queued logs (call from main loop)
void BT_LOGGER::ProcessQueue() {
    while (queueCount > 0) {
        LogEntry* entry = &logQueue[queueHead];

        // Format time
        time_t now = entry->timestamp / 1000;
        struct tm timeinfo;
        localtime_r(&now, &timeinfo);  // Thread-safe version
        char timeStr[20];
        strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &timeinfo);

        // Get level and category strings
        String levelStr;
        String categoryStr;

        // Generate level string
        switch (entry->level) {
            #define LOG_LEVEL(name, value, str) case LogLevel::name: levelStr = String(str); levelStr.toUpperCase(); break;
            LOG_LEVELS_LIST
            #undef LOG_LEVEL
            default: levelStr = "UNKNOWN";
        }

        // Generate category string
        switch (entry->category) {
            #define LOG_CATEGORY(name, value, str) case LogCategory::name: categoryStr = String(str).substring(0, 6); break;
            LOG_CATEGORIES_LIST
            #undef LOG_CATEGORY
            default: categoryStr = "UNK";
        }

        // Check if this category should be logged
        if ((gSetLogCategory & static_cast<uint16_t>(entry->category)) > 0) {
            Serial.print(timeStr);
            Serial.print("|");
            Serial.print(levelStr);
            Serial.print("|");
            Serial.print(categoryStr);
            Serial.print("|");
            Serial.print(entry->method);
            Serial.print("|");
            Serial.println(entry->message);

            // Send to BLE callback if registered (avoiding recursion)
            // TODO: Enable BLE output when needed
            if (logCallback != nullptr) {
                static char logBuffer[200];
                snprintf(logBuffer, sizeof(logBuffer), "%s|%s|%s|%s|%s",
                        timeStr, levelStr.c_str(), categoryStr.c_str(),
                        entry->method, entry->message);
                logCallback->onLogOutput(logBuffer);
            }
        }

        queueHead = (queueHead + 1) % LOG_QUEUE_SIZE;
        queueCount--;
    }
}

// Core logging function with printf-style formatting
void BT_LOGGER::Write(LogLevel level, LogCategory logCategory, String method, const char* format, ...) {
    if (level < m_oLogLevel || !m_bConsoleEnabled) return;

    // Use global buffer for formatting (not on stack)
    static char buffer[160];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    // Push to queue instead of immediate print
    pushLogEntry(level, logCategory, method.c_str(), buffer);
}

// Immediate logging function (queues message then immediately processes queue)
void BT_LOGGER::WriteImmediate(LogLevel level, LogCategory logCategory, String method, const char* format, ...) {
    if (level < m_oLogLevel || !m_bConsoleEnabled) return;

    // Format message
    static char buffer[160];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    // Queue the message (with timestamp)
    pushLogEntry(level, logCategory, method.c_str(), buffer);

    // Immediately process the queue to output everything
    ProcessQueue();
}

// Configuration functions
void BT_LOGGER::EnableConsole() {
    m_bConsoleEnabled = true;
}

void BT_LOGGER::DisableConsole() {
    m_bConsoleEnabled = false;
}

void BT_LOGGER::SetLogLevel(LogLevel level) {
    m_oLogLevel = level;
}

void BT_LOGGER::SetLogCategory(uint16_t categoryMask) {
    gSetLogCategory = categoryMask;
}

void BT_LOGGER::EnableCategory(LogCategory category) {
    gSetLogCategory |= static_cast<uint16_t>(category);
}

void BT_LOGGER::DisableCategory(LogCategory category) {
    gSetLogCategory &= ~static_cast<uint16_t>(category);
}

void BT_LOGGER::EnableAllCategories() {
    gSetLogCategory = 0xFFFF;
}

void BT_LOGGER::DisableAllCategories() {
    gSetLogCategory = 0x0000;
}

void BT_LOGGER::UseRealTime(bool enabled) {
    m_bUseRealTime = enabled;
}

// Status functions
bool BT_LOGGER::IsConsoleEnabled() {
    return m_bConsoleEnabled;
}

LogLevel BT_LOGGER::GetLogLevel() {
    return m_oLogLevel;
}

uint16_t BT_LOGGER::GetLogCategory() {
    return gSetLogCategory;
}

bool BT_LOGGER::IsCategoryEnabled(LogCategory category) {
    return (gSetLogCategory & static_cast<uint16_t>(category)) != 0;
}

// Version functions
const char* BT_LOGGER::GetVersion() {
    return BT_LOGGER_VERSION;
}

void BT_LOGGER::PrintVersion() {
    // Use direct Serial output to avoid stack issues - no buffer needed
    Serial.print("00:00:00|INFO|SYSTEM|BT_LOGGER|Library version: ");
    Serial.print(BT_LOGGER_VERSION);
    Serial.print(" (Major: ");
    Serial.print(BT_LOGGER_VERSION_MAJOR);
    Serial.print(", Minor: ");
    Serial.print(BT_LOGGER_VERSION_MINOR);
    Serial.print(", Patch: ");
    Serial.print(BT_LOGGER_VERSION_PATCH);
    Serial.println(")");
}

// Set log output callback
void BT_LOGGER::SetLogCallback(LogOutputCallback* callback) {
    logCallback = callback;
}

// Get all available log categories as JSON array
String BT_LOGGER::GetAllCategoriesJson() {
    // Pre-calculate size to avoid reallocations
    // Each entry: {"value":XXXXX,"name":"XXXXXX"} ~ 35 bytes average
    // 15 categories × 35 bytes = 525 bytes + brackets
    String json;
    json.reserve(550);  // Pre-allocate memory

    json = "[";
    bool first = true;

    // Generate JSON from the LOG_CATEGORIES_LIST macro
    #define LOG_CATEGORY(name, value, str) \
        if (!first) json += ","; \
        json += "{\"value\":"; \
        json += String(value); \
        json += ",\"name\":\""; \
        json += str; \
        json += "\"}"; \
        first = false;

    LOG_CATEGORIES_LIST

    #undef LOG_CATEGORY

    json += "]";
    return json;
}