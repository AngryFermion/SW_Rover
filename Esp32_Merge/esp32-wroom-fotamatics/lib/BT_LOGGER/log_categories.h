/**
 * log_categories.h - Common Log Category Definitions
 *
 * Centralized definitions for all log categories used across the project.
 * Include this file wherever you need to use log categories.
 *
 * Author: Narayana Swamy
 * Date: September 2025
 */

#ifndef LOG_CATEGORIES_H
#define LOG_CATEGORIES_H

#include "BT_LOGGER.h"

// Log category aliases for easier use
#define LOG_CAT_SYSTEM   LogCategory::SYSTEM
#define LOG_CAT_SETUP    LogCategory::SETUP
#define LOG_CAT_WIFI     LogCategory::WIFI
#define LOG_CAT_MODBUS   LogCategory::MODBUS
#define LOG_CAT_SPIFFS   LogCategory::SPIFFS
#define LOG_CAT_FS       LogCategory::FS
#define LOG_CAT_SERVER   LogCategory::SERVER
#define LOG_CAT_LIVE     LogCategory::LIVE
#define LOG_CAT_MQTT     LogCategory::MQTT
#define LOG_CAT_CAN      LogCategory::CAN
#define LOG_CAT_IO       LogCategory::IO
#define LOG_CAT_OTHERS   LogCategory::OTHERS
#define LOG_CAT_DEVICE   LogCategory::DEVICE
#define LOG_CAT_BLE      LogCategory::BLE

// Convenience alias for debug messages
#define LOG_CAT_DEBUG    LogCategory::OTHERS

#endif // LOG_CATEGORIES_H