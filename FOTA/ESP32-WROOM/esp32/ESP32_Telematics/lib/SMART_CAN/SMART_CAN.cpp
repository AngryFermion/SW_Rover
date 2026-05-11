/**
 * SMART_CAN.cpp - ESP32 TWAI (CAN) Library Implementation
 *
 * Author: Narayana Swamy
 * Date: September 2025
 */

#include "SMART_CAN.h"

#if ENABLE_CAN
// Static variable for error throttling
static unsigned long lastTxErrorLogTime = 0;
static const unsigned long TX_ERROR_LOG_INTERVAL = 5000; // 5 seconds

// Constructor
SMART_CAN::SMART_CAN(int tx_pin, int rx_pin) {
    _tx_pin = tx_pin;
    _rx_pin = rx_pin;
    _speed = CAN_SPEED_500KBPS;
    _receive_callback = nullptr;
    _initialized = false;
    _filter_enabled = false;
    _filter_id = 0;
    _filter_mask = 0;
    _filter_extended = false;
}

// Destructor
SMART_CAN::~SMART_CAN() {
    end();
}

// Get timing configuration based on speed
twai_timing_config_t SMART_CAN::getTimingConfig(can_speed_t speed) {
    switch (speed) {
        case CAN_SPEED_125KBPS:
            return TWAI_TIMING_CONFIG_125KBITS();
        case CAN_SPEED_250KBPS:
            return TWAI_TIMING_CONFIG_250KBITS();
        case CAN_SPEED_500KBPS:
            return TWAI_TIMING_CONFIG_500KBITS();
        case CAN_SPEED_1MBPS:
            return TWAI_TIMING_CONFIG_1MBITS();
        default:
            return TWAI_TIMING_CONFIG_500KBITS();
    }
}

// Get filter configuration
twai_filter_config_t SMART_CAN::getFilterConfig() {
    if (_filter_enabled) {
        twai_filter_config_t filter_config;
        filter_config.acceptance_code = _filter_id << 21;  // Standard ID format
        filter_config.acceptance_mask = ~(_filter_mask << 21);
        filter_config.single_filter = true;

        if (_filter_extended) {
            filter_config.acceptance_code = _filter_id << 3;  // Extended ID format
            filter_config.acceptance_mask = ~(_filter_mask << 3);
        }

        return filter_config;
    } else {
        return TWAI_FILTER_CONFIG_ACCEPT_ALL();
    }
}

// Initialize CAN bus
bool SMART_CAN::begin(can_speed_t speed) {
    if (_initialized) {
        end(); // Stop current instance if running
    }

    _speed = speed;

    // Configure TWAI driver
    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT((gpio_num_t)_tx_pin, (gpio_num_t)_rx_pin, TWAI_MODE_NORMAL);
    twai_timing_config_t t_config = getTimingConfig(_speed);
    twai_filter_config_t f_config = getFilterConfig();

    // Install TWAI driver
    esp_err_t result = twai_driver_install(&g_config, &t_config, &f_config);
    if (result != ESP_OK) {
        myLogger->Write(LogLevel::Error, LOG_CAT_SYSTEM, "SMART_CAN::begin",
                       "TWAI driver install failed: %s", errorToString(result));
        return false;
    }

    // Start TWAI driver
    result = twai_start();
    if (result != ESP_OK) {
        myLogger->Write(LogLevel::Error, LOG_CAT_SYSTEM, "SMART_CAN::begin",
                       "TWAI start failed: %s", errorToString(result));
        twai_driver_uninstall();
        return false;
    }

    _initialized = true;
    myLogger->Write(LogLevel::Info, LOG_CAT_SYSTEM, "SMART_CAN::begin",
                   "SMART_CAN initialized successfully at %s on pins TX:%d RX:%d",
                   speedToString(_speed), _tx_pin, _rx_pin);

    return true;
}

// Stop CAN bus
bool SMART_CAN::end() {
    if (!_initialized) {
        return true;
    }

    esp_err_t result = twai_stop();
    if (result != ESP_OK) {
        myLogger->Write(LogLevel::Error, LOG_CAT_SYSTEM, "SMART_CAN::end",
                       "TWAI stop failed: %s", errorToString(result));
    }

    result = twai_driver_uninstall();
    if (result != ESP_OK) {
        myLogger->Write(LogLevel::Error, LOG_CAT_SYSTEM, "SMART_CAN::end",
                       "TWAI driver uninstall failed: %s", errorToString(result));
        return false;
    }

    _initialized = false;
    myLogger->Write(LogLevel::Info, LOG_CAT_SYSTEM, "SMART_CAN::end", "SMART_CAN stopped");
    return true;
}

// Check if initialized
bool SMART_CAN::isInitialized() const {
    return _initialized;
}

// Send message with parameters
bool SMART_CAN::sendMessage(uint32_t id, const uint8_t* data, uint8_t length, bool extended) {
    if (!_initialized) {
        myLogger->Write(LogLevel::Warn, LOG_CAT_SYSTEM, "SMART_CAN::sendMessage",
                       "CAN not initialized");
        return false;
    }

    if (length > 8) {
        myLogger->Write(LogLevel::Warn, LOG_CAT_SYSTEM, "SMART_CAN::sendMessage",
                       "Data length exceeds 8 bytes: %d", length);
        return false;
    }

    twai_message_t message;
    message.identifier = id;
    message.data_length_code = length;
    message.flags = extended ? TWAI_MSG_FLAG_EXTD : TWAI_MSG_FLAG_NONE;

    for (int i = 0; i < length; i++) {
        message.data[i] = data[i];
    }

    esp_err_t result = twai_transmit(&message, pdMS_TO_TICKS(1000));
    if (result == ESP_OK) {
        // Only log detailed TX info at trace level to reduce verbosity
        char hexData[32] = "";
        for (int i = 0; i < length; i++) {
            char temp[4];
            sprintf(temp, "%s%02X", (i > 0) ? " " : "", data[i]);
            strcat(hexData, temp);
        }
        myLogger->Write(LogLevel::Trace, LOG_CAT_DEVICE, "SMART_CAN::sendMessage",
                       "TX ID: 0x%03X [%s]", id, hexData);
        return true;
    } else {
        // Only log TX failures every 5 seconds to avoid spam
        unsigned long currentTime = millis();
        if ((currentTime - lastTxErrorLogTime) >= TX_ERROR_LOG_INTERVAL) {
            myLogger->Write(LogLevel::Error, LOG_CAT_SYSTEM, "SMART_CAN::sendMessage",
                           "CAN TX failed: %s", errorToString(result));
            lastTxErrorLogTime = currentTime;
        }
        return false;
    }
}

// Send message using can_message_t structure
bool SMART_CAN::sendMessage(const can_message_t* message) {
    return sendMessage(message->id, message->data, message->length, message->extended);
}

// Send standard frame
bool SMART_CAN::sendStandardFrame(uint32_t id, const uint8_t* data, uint8_t length) {
    return sendMessage(id, data, length, false);
}

// Send extended frame
bool SMART_CAN::sendExtendedFrame(uint32_t id, const uint8_t* data, uint8_t length) {
    return sendMessage(id, data, length, true);
}

// Receive message with timeout
bool SMART_CAN::receiveMessage(can_message_t* message, uint32_t timeout_ms) {
    if (!_initialized) {
        return false;
    }

    twai_message_t twai_msg;
    esp_err_t result = twai_receive(&twai_msg, pdMS_TO_TICKS(timeout_ms));

    if (result == ESP_OK) {
        message->id = twai_msg.identifier;
        message->length = twai_msg.data_length_code;
        message->extended = (twai_msg.flags & TWAI_MSG_FLAG_EXTD) != 0;
        message->rtr = (twai_msg.flags & TWAI_MSG_FLAG_RTR) != 0;

        for (int i = 0; i < message->length; i++) {
            message->data[i] = twai_msg.data[i];
        }

        return true;
    }

    return false;
}

// Check if message is available
bool SMART_CAN::hasMessage() {
    if (!_initialized) {
        return false;
    }

    twai_status_info_t status_info;
    esp_err_t result = twai_get_status_info(&status_info);

    return (result == ESP_OK && status_info.msgs_to_rx > 0);
}

// Set receive callback
void SMART_CAN::setReceiveCallback(can_receive_callback_t callback) {
    _receive_callback = callback;
}

// Check for messages and call callback if set
void SMART_CAN::checkMessages() {
    if (!_initialized || !_receive_callback) {
        return;
    }

    can_message_t message;
    while (receiveMessage(&message, 0)) { // Non-blocking receive
        _receive_callback(&message);
    }
}

// Set GPIO pins
bool SMART_CAN::setPins(int tx_pin, int rx_pin) {
    if (_initialized) {
        myLogger->Write(LogLevel::Warn, LOG_CAT_SYSTEM, "SMART_CAN::setPins",
                       "Cannot change pins while CAN is running. Call end() first.");
        return false;
    }

    _tx_pin = tx_pin;
    _rx_pin = rx_pin;
    return true;
}

// Set CAN speed
bool SMART_CAN::setSpeed(can_speed_t speed) {
    if (_initialized) {
        myLogger->Write(LogLevel::Warn, LOG_CAT_SYSTEM, "SMART_CAN::setSpeed",
                       "Cannot change speed while CAN is running. Call end() first.");
        return false;
    }

    _speed = speed;
    return true;
}

// Set message filter
bool SMART_CAN::setFilter(uint32_t filter_id, uint32_t filter_mask, bool extended) {
    _filter_enabled = true;
    _filter_id = filter_id;
    _filter_mask = filter_mask;
    _filter_extended = extended;

    if (_initialized) {
        myLogger->Write(LogLevel::Info, LOG_CAT_SYSTEM, "SMART_CAN::setFilter",
                       "Filter will be applied after driver restart");
        // Store filter settings, apply on next begin()
        can_speed_t current_speed = _speed;
        end();
        return begin(current_speed);
    }

    myLogger->Write(LogLevel::Info, LOG_CAT_SYSTEM, "SMART_CAN::setFilter",
                   "Filter set - ID: 0x%03X, Mask: 0x%03X, Extended: %s",
                   filter_id, filter_mask, extended ? "Yes" : "No");
    return true;
}

// Clear message filter
bool SMART_CAN::clearFilter() {
    _filter_enabled = false;
    _filter_id = 0;
    _filter_mask = 0;
    _filter_extended = false;

    if (_initialized) {
        myLogger->Write(LogLevel::Info, LOG_CAT_SYSTEM, "SMART_CAN::clearFilter",
                       "Filter cleared - accepting all messages after restart");
        can_speed_t current_speed = _speed;
        end();
        return begin(current_speed);
    }

    myLogger->Write(LogLevel::Info, LOG_CAT_SYSTEM, "SMART_CAN::clearFilter",
                   "Filter cleared - will accept all messages");
    return true;
}

// Set standard filter (convenience method)
bool SMART_CAN::setStandardFilter(uint32_t filter_id, uint32_t filter_mask) {
    return setFilter(filter_id, filter_mask, false);
}

// Set extended filter (convenience method)
bool SMART_CAN::setExtendedFilter(uint32_t filter_id, uint32_t filter_mask) {
    return setFilter(filter_id, filter_mask, true);
}

// Set range filter (accept IDs within a range)
bool SMART_CAN::setRangeFilter(uint32_t id_start, uint32_t id_end, bool extended) {
    if (id_start > id_end) {
        myLogger->Write(LogLevel::Warn, LOG_CAT_SYSTEM, "SMART_CAN::setRangeFilter",
                       "Invalid range: start ID must be <= end ID");
        return false;
    }

    // Calculate mask to cover the range
    uint32_t range = id_end - id_start;
    uint32_t mask = 0;

    // Find the appropriate mask that covers the range
    if (extended) {
        mask = 0x1FFFFFFF; // Start with all bits set for extended
        uint32_t test_mask = 0x1FFFFFFF;
        while (test_mask && ((id_start & test_mask) != (id_end & test_mask))) {
            test_mask <<= 1;
            mask = test_mask;
        }
    } else {
        mask = 0x7FF; // Start with all bits set for standard
        uint32_t test_mask = 0x7FF;
        while (test_mask && ((id_start & test_mask) != (id_end & test_mask))) {
            test_mask <<= 1;
            mask = test_mask;
        }
    }

    myLogger->Write(LogLevel::Info, LOG_CAT_SYSTEM, "SMART_CAN::setRangeFilter",
                   "Range filter - Start: 0x%03X, End: 0x%03X, Calculated mask: 0x%03X",
                   id_start, id_end, mask);

    return setFilter(id_start, mask, extended);
}

// Set multiple ID filter (accept specific list of IDs)
bool SMART_CAN::setMultipleFilter(const uint32_t* ids, uint8_t count, bool extended) {
    if (count == 0 || ids == nullptr) {
        myLogger->Write(LogLevel::Warn, LOG_CAT_SYSTEM, "SMART_CAN::setMultipleFilter",
                       "Invalid multiple filter parameters");
        return false;
    }

    if (count == 1) {
        // Single ID - use exact match
        myLogger->Write(LogLevel::Debug, LOG_CAT_SYSTEM, "SMART_CAN::setMultipleFilter",
                       "Single ID filter: 0x%03X", ids[0]);
        return setFilter(ids[0], extended ? 0x1FFFFFFF : 0x7FF, extended);
    }

    // For multiple IDs, find common bits and create mask
    uint32_t common_bits = ids[0];
    for (uint8_t i = 1; i < count; i++) {
        common_bits &= ids[i]; // Find bits that are same in all IDs
    }

    // Create mask - bits that differ between IDs should be masked out
    uint32_t mask = extended ? 0x1FFFFFFF : 0x7FF;
    for (uint8_t i = 0; i < count; i++) {
        mask &= ~(ids[0] ^ ids[i]); // Clear bits that differ
    }

    myLogger->Write(LogLevel::Info, LOG_CAT_SYSTEM, "SMART_CAN::setMultipleFilter",
                   "Multiple ID filter - Count: %d, Base: 0x%03X, Mask: 0x%03X",
                   count, common_bits, mask);
    myLogger->Write(LogLevel::Debug, LOG_CAT_SYSTEM, "SMART_CAN::setMultipleFilter",
                   "Warning: Hardware filter may accept additional IDs within mask range");

    return setFilter(common_bits, mask, extended);
}

// Check if filter is enabled
bool SMART_CAN::isFilterEnabled() const {
    return _filter_enabled;
}

// Get CAN status
bool SMART_CAN::getStatus() {
    if (!_initialized) {
        return false;
    }

    twai_status_info_t status_info;
    esp_err_t result = twai_get_status_info(&status_info);

    return (result == ESP_OK && status_info.state == TWAI_STATE_RUNNING);
}

// Get error count
uint32_t SMART_CAN::getErrorCount() {
    if (!_initialized) {
        return 0;
    }

    twai_status_info_t status_info;
    esp_err_t result = twai_get_status_info(&status_info);

    if (result == ESP_OK) {
        return status_info.tx_error_counter + status_info.rx_error_counter;
    }

    return 0;
}

// Print status information
void SMART_CAN::printStatus() {
    if (!_initialized) {
        myLogger->Write(LogLevel::Warn, LOG_CAT_SYSTEM, "SMART_CAN::printStatus",
                       "CAN not initialized");
        return;
    }

    twai_status_info_t status_info;
    esp_err_t result = twai_get_status_info(&status_info);

    if (result == ESP_OK) {
        const char* state_str;
        switch (status_info.state) {
            case TWAI_STATE_STOPPED: state_str = "STOPPED"; break;
            case TWAI_STATE_RUNNING: state_str = "RUNNING"; break;
            case TWAI_STATE_BUS_OFF: state_str = "BUS_OFF"; break;
            case TWAI_STATE_RECOVERING: state_str = "RECOVERING"; break;
            default: state_str = "UNKNOWN"; break;
        }

        myLogger->Write(LogLevel::Info, LOG_CAT_SYSTEM, "SMART_CAN::printStatus",
                       "=== CAN Status ===");
        myLogger->Write(LogLevel::Info, LOG_CAT_SYSTEM, "SMART_CAN::printStatus",
                       "State: %s, TX Queue: %d, RX Queue: %d",
                       state_str, status_info.msgs_to_tx, status_info.msgs_to_rx);
        myLogger->Write(LogLevel::Info, LOG_CAT_SYSTEM, "SMART_CAN::printStatus",
                       "Errors - TX: %d, RX: %d, TX Failed: %d, RX Missed: %d",
                       status_info.tx_error_counter, status_info.rx_error_counter,
                       status_info.tx_failed_count, status_info.rx_missed_count);
        myLogger->Write(LogLevel::Info, LOG_CAT_SYSTEM, "SMART_CAN::printStatus",
                       "Counters - RX Overrun: %d, Arb Lost: %d, Bus Error: %d",
                       status_info.rx_overrun_count, status_info.arb_lost_count,
                       status_info.bus_error_count);
    } else {
        myLogger->Write(LogLevel::Error, LOG_CAT_SYSTEM, "SMART_CAN::printStatus",
                       "Failed to get status: %s", errorToString(result));
    }
}

// Convert speed enum to string
const char* SMART_CAN::speedToString(can_speed_t speed) {
    switch (speed) {
        case CAN_SPEED_125KBPS: return "125 kbps";
        case CAN_SPEED_250KBPS: return "250 kbps";
        case CAN_SPEED_500KBPS: return "500 kbps";
        case CAN_SPEED_1MBPS: return "1 Mbps";
        default: return "Unknown";
    }
}

// Convert ESP error to string
const char* SMART_CAN::errorToString(esp_err_t error) {
    return esp_err_to_name(error);
}

#endif