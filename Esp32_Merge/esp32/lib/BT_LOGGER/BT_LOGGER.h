/**
 * BT_LOGGER.h - Bluetooth Logger Library
 *
 * A lightweight logging system for ESP32 projects
 * with configurable levels and categories
 *
 * Author: Narayana Swamy
 * Date: September 2025
 * Version: 1.0.0
 */

#ifndef BT_LOGGER_H
#define BT_LOGGER_H

#include <Arduino.h>

// Version Information
#define BT_LOGGER_VERSION_MAJOR 1
#define BT_LOGGER_VERSION_MINOR 0
#define BT_LOGGER_VERSION_PATCH 0
#define BT_LOGGER_VERSION "1.0.0"

// Define all log categories in one place
// Format: LOG_CATEGORY(ENUM_NAME, bit_value, "string_name")
#define LOG_CATEGORIES_LIST \
    LOG_CATEGORY(SYSTEM,  0x0001, "system") \
    LOG_CATEGORY(SETUP,   0x0002, "setup") \
    LOG_CATEGORY(WIFI,    0x0004, "wifi") \
    LOG_CATEGORY(HTTP,    0x0008, "http") \
    LOG_CATEGORY(SPIFFS,  0x0010, "spiffs") \
    LOG_CATEGORY(FS,      0x0020, "fs") \
    LOG_CATEGORY(SERVER,  0x0040, "server") \
    LOG_CATEGORY(LIVE,    0x0080, "live") \
    LOG_CATEGORY(MQTT,    0x0100, "mqtt") \
    LOG_CATEGORY(CAN,     0x0200, "can") \
    LOG_CATEGORY(IO,      0x0400, "io") \
    LOG_CATEGORY(OTHERS,  0x0800, "others") \
    LOG_CATEGORY(DEVICE,  0x1000, "device") \
    LOG_CATEGORY(BLE,     0x2000, "ble") \
    LOG_CATEGORY(FOTA,    0x4000, "fota") \
    LOG_CATEGORY(SER,     0x8000, "serial") 

// Generate enum from the list
#define LOG_CATEGORY(name, value, str) name = value,
enum class LogCategory : uint16_t {
    LOG_CATEGORIES_LIST
};
#undef LOG_CATEGORY

// Define all log levels in one place
// Format: LOG_LEVEL(ENUM_NAME, value, "string_name")
#define LOG_LEVELS_LIST \
    LOG_LEVEL(Trace, 0, "trace") \
    LOG_LEVEL(Debug, 1, "debug") \
    LOG_LEVEL(Info,  2, "info") \
    LOG_LEVEL(Warn,  3, "warn") \
    LOG_LEVEL(Error, 4, "error") \
    LOG_LEVEL(Fatal, 5, "fatal")

// Generate enum from the list
#define LOG_LEVEL(name, value, str) name = value,
enum class LogLevel {
    LOG_LEVELS_LIST
};
#undef LOG_LEVEL


extern uint16_t gSetLogCategory;

// Log queue entry
struct LogEntry {
    LogLevel level;
    LogCategory category;
    char method[32];
    char message[160];
    uint32_t timestamp;
};

// Callback interface for log consumers (e.g., BLE)
class LogOutputCallback {
public:
    virtual ~LogOutputCallback() {}
    virtual void onLogOutput(const char* formattedLog) = 0;
};

class BT_LOGGER {
private:
    bool m_bConsoleEnabled;
    LogLevel m_oLogLevel;
    bool m_bUseRealTime;  // Use NTP synced time vs uptime

    // FIFO queue for async logging (static = shared across all instances)
    static const int LOG_QUEUE_SIZE = 10;
    static LogEntry logQueue[LOG_QUEUE_SIZE];
    static volatile int queueHead;
    static volatile int queueTail;
    static volatile int queueCount;

    // Log output callback (static = shared)
    static LogOutputCallback* logCallback;

    String getTimeString();
    String getUptimeString();
    String getRealTimeString();
    String getLevelString(LogLevel level);
    String getCategoryString(LogCategory logCategory);

    // Queue management
    bool pushLogEntry(LogLevel level, LogCategory category, const char* method, const char* message);

public:
    BT_LOGGER();
    BT_LOGGER(LogLevel level);
    ~BT_LOGGER();

    // Core logging functions
    void Write(LogLevel level, LogCategory logCategory, String method, String message);
    void Write(LogLevel level, LogCategory logCategory, String method, const char* format, ...);
    void WriteImmediate(LogLevel level, LogCategory logCategory, String method, const char* format, ...);

    // Configuration
    void EnableConsole();
    void DisableConsole();
    void SetLogLevel(LogLevel level);
    void SetLogCategory(uint16_t categoryMask);
    void EnableCategory(LogCategory category);
    void DisableCategory(LogCategory category);
    void EnableAllCategories();
    void DisableAllCategories();
    void UseRealTime(bool enabled);  // Switch between uptime and real-time

    // Status
    bool IsConsoleEnabled();
    LogLevel GetLogLevel();
    uint16_t GetLogCategory();
    bool IsCategoryEnabled(LogCategory category);

    // Version
    static const char* GetVersion();
    static void PrintVersion();

    // Category enumeration helper
    String GetAllCategoriesJson();

    // Process queued logs (call from loop)
    static void ProcessQueue();

    // Set log output callback (e.g., for BLE transmission)
    static void SetLogCallback(LogOutputCallback* callback);
};

// Global BT_LOGGER instance
extern BT_LOGGER g_Logger;

#endif // BT_LOGGER_H