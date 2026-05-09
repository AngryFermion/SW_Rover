/**
 * SMART_CAN.h - ESP32 TWAI (CAN) Library
 *
 * A reusable library for ESP32 CAN bus communication using the built-in TWAI driver.
 * Provides an easy-to-use interface for sending and receiving CAN messages.
 *
 * Features:
 * - Configurable CAN speed (125k, 250k, 500k, 1M bps)
 * - Configurable GPIO pins for TX/RX
 * - Non-blocking message reception
 * - Multiple message filtering options:
 *   • Single ID filtering (exact match)
 *   • Range filtering (accept ID range)
 *   • Multiple ID filtering (accept specific IDs)
 *   • Custom mask filtering (advanced users)
 * - Error handling and status reporting
 * - Compatible with all ESP32 variants
 *
 * Multi-Message Reception Examples:
 *
 * 1. Accept specific IDs:
 *    uint32_t ids[] = {0x123, 0x124, 0x125};
 *    can_bus.setMultipleFilter(ids, 3);
 *
 * 2. Accept ID range:
 *    can_bus.setRangeFilter(0x100, 0x1FF);  // Accept 0x100-0x1FF
 *
 * 3. Accept all messages (no filter):
 *    can_bus.clearFilter();
 *
 * 4. Custom mask for pattern matching:
 *    can_bus.setFilter(0x120, 0x7F0);  // Accept 0x120-0x12F
 *
 * Note: ESP32 hardware filter limitations mean some methods may accept
 * additional IDs beyond those specified. Use callback function to further
 * filter messages in software if needed.
 *
 * Author: Narayana Swamy
 * Date: September 2025
 */

#ifndef SMART_CAN_H
#define SMART_CAN_H

#include <Arduino.h>
#include "driver/twai.h"
#include "SMART_LOGGER.h"
#include "log_categories.h"

// CAN Speed definitions
typedef enum {
    CAN_SPEED_125KBPS = 0,
    CAN_SPEED_250KBPS,
    CAN_SPEED_500KBPS,
    CAN_SPEED_1MBPS
} can_speed_t;

// CAN Message structure
typedef struct {
    uint32_t id;                    // CAN ID
    uint8_t data[8];               // Data bytes (max 8)
    uint8_t length;                // Data length (0-8)
    bool extended;                 // Extended frame format
    bool rtr;                      // Remote transmission request
} can_message_t;

// Callback function type for received messages
typedef void (*can_receive_callback_t)(const can_message_t* message);

// External logger instance
extern SMART_LOGGER* myLogger;

class SMART_CAN {
private:
    int _tx_pin;
    int _rx_pin;
    can_speed_t _speed;
    can_receive_callback_t _receive_callback;
    bool _initialized;
    bool _filter_enabled;
    uint32_t _filter_id;
    uint32_t _filter_mask;
    bool _filter_extended;

    twai_timing_config_t getTimingConfig(can_speed_t speed);
    twai_filter_config_t getFilterConfig();

public:
    // Constructor
    SMART_CAN(int tx_pin = 5, int rx_pin = 4);

    // Destructor
    ~SMART_CAN();

    // Initialization and configuration
    bool begin(can_speed_t speed = CAN_SPEED_500KBPS);
    bool end();
    bool isInitialized() const;

    // Message transmission
    bool sendMessage(uint32_t id, const uint8_t* data, uint8_t length, bool extended = false);
    bool sendMessage(const can_message_t* message);
    bool sendStandardFrame(uint32_t id, const uint8_t* data, uint8_t length);
    bool sendExtendedFrame(uint32_t id, const uint8_t* data, uint8_t length);

    // Message reception
    bool receiveMessage(can_message_t* message, uint32_t timeout_ms = 0);
    bool hasMessage();
    void setReceiveCallback(can_receive_callback_t callback);
    void checkMessages(); // Call this in loop() to process incoming messages

    // Configuration
    bool setPins(int tx_pin, int rx_pin);
    bool setSpeed(can_speed_t speed);
    bool setFilter(uint32_t filter_id, uint32_t filter_mask, bool extended = false);
    bool setStandardFilter(uint32_t filter_id, uint32_t filter_mask = 0x7FF);
    bool setExtendedFilter(uint32_t filter_id, uint32_t filter_mask = 0x1FFFFFFF);
    bool setRangeFilter(uint32_t id_start, uint32_t id_end, bool extended = false);
    bool setMultipleFilter(const uint32_t* ids, uint8_t count, bool extended = false);
    bool clearFilter();
    bool isFilterEnabled() const;

    // Status and diagnostics
    bool getStatus();
    uint32_t getErrorCount();
    void printStatus();

    // Utility functions
    static const char* speedToString(can_speed_t speed);
    static const char* errorToString(esp_err_t error);
};

#endif // SMART_CAN_H