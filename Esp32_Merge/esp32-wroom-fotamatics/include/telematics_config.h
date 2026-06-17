/**
 * telematics_config.h — SmartWheels ESP32-WROOM FOTAmatics
 *
 * Configuration for the telematics (CAN→MQTT) subsystem.
 *
 * Data source selection:
 *   Define USE_DUMMY_DATA  →  synthetic CAN signals, no MCU wiring required.
 *   Leave undefined        →  real SIGNAL_NAME,VALUE lines from MCU via Serial1.
 *
 * NOTE: Serial1 (GPIO 16 RX / GPIO 17 TX) is shared with the FOTA bootloader.
 *       The telematics task automatically pauses while a FOTA transmission is
 *       in progress and resumes when it completes.
 */

#ifndef TELEMATICS_CONFIG_H
#define TELEMATICS_CONFIG_H

// ===== DATA SOURCE =====
// Uncomment to generate synthetic CAN signals internally (no MCU / UART wiring needed).
// Comment out to receive real data from the MCU over Serial1.
// #define USE_DUMMY_DATA

// Interval between dummy signal emissions (ms) — only used when USE_DUMMY_DATA is defined.
#define TELEMATICS_DUMMY_INTERVAL_MS   1000

// ===== SERIAL DATA CHANNEL (shared with FOTA bootloader on Serial1) =====
#define TELEMATICS_SERIAL          Serial1
#define TELEMATICS_SERIAL_BAUD     115200
#define TELEMATICS_SERIAL_RX_PIN   16
#define TELEMATICS_SERIAL_TX_PIN   17
#define TELEMATICS_LINE_BUFFER_SIZE 128

// ===== MQTT TOPICS =====
#define TELEMATICS_TOPIC_CAN_SIGNALS  "SmartKit/Ultra"
#define TELEMATICS_TOPIC_DEVICE_HEALTH "SmartKit/device_health"
#define TELEMATICS_TOPIC_CONTROL       "SmartKit/Control"

// ===== DEVICE ID (used in JSON payloads) =====
#define TELEMATICS_DEVICE_ID  "MA3ZZ20G0T1234567"

// ===== JSON BUFFER =====
#define TELEMATICS_JSON_BUFFER_SIZE  256

// ===== HEALTH PUBLISH =====
// Uncomment to enable periodic device health publishing to TELEMATICS_TOPIC_DEVICE_HEALTH
// #define TELEMATICS_PUBLISH_HEALTH
#define TELEMATICS_HEALTH_INTERVAL_MS  5000

#endif // TELEMATICS_CONFIG_H
