/**
 * uart_config.h — SmartWheels ESP32-WROOM Telematics
 *
 * Logger channel (UART0/USB-UART chip): system debug output
 * Data channel   (UART1):              CAN data from MCU
 *
 * Data source selection:
 *   Define USE_DUMMY_DATA to inject synthetic CAN signals without any UART hardware.
 *   Comment it out (or leave undefined) to receive real data from the MCU via UART1.
 */

#ifndef UART_CONFIG_H
#define UART_CONFIG_H

// ===== DATA SOURCE =====
// Uncomment to enable internal dummy data generation (no MCU / UART wiring required).
// Comment out to use real UART data from the connected MCU.
// #define USE_DUMMY_DATA

// How often to emit one dummy CAN signal (ms) — only used when USE_DUMMY_DATA is defined.
#define DUMMY_DATA_INTERVAL_MS  1000

// ===== LOGGER CHANNEL (UART0 / USB-UART chip) =====
#define UART_LOGGER_NUM      0
#define UART_LOGGER_BAUD     115200
#define UART_LOGGER_TX_PIN   1   // Default UART0 TX on WROOM (driven by USB-UART chip)
#define UART_LOGGER_RX_PIN   3   // Default UART0 RX on WROOM (driven by USB-UART chip)

// ===== DATA CHANNEL (UART1) — not used when USE_DUMMY_DATA is defined =====
#define UART_DATA_NUM        1
#define UART_DATA_BAUD       115200
#define UART_DATA_TX_PIN     17
#define UART_DATA_RX_PIN     16

// ===== BUFFER SIZES =====
#define UART_LOGGER_RX_BUFFER_SIZE  256
#define UART_LOGGER_TX_BUFFER_SIZE  256
#define UART_DATA_RX_BUFFER_SIZE    1024
#define UART_DATA_TX_BUFFER_SIZE    256
#define UART_DATA_LINE_BUFFER_SIZE  128

// ===== TIMEOUT =====
#define UART_READ_TIMEOUT_MS  100

// ===== LOGGER CONTROL =====
#define UART_LOGGER_ENABLED         1
#define UART_LOGGER_RUNTIME_CONTROL 1

#endif // UART_CONFIG_H
