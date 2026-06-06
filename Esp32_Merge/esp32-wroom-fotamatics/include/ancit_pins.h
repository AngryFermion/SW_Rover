#ifndef _HEADER_ANCIT_PINS_H_
#define _HEADER_ANCIT_PINS_H_

#include <Arduino.h>

// ===== STATUS LED (3 discrete R/G/B LEDs) =====
#define MP_NET_LED_RED   GPIO_NUM_26
#define MP_NET_LED_GREEN GPIO_NUM_25
#define MP_NET_LED_BLUE  GPIO_NUM_27

// ===== FOTA / TELEMATICS SERIAL (Serial1 — shared) =====
// Used by FOTA bootloader to send S-records to the NXP MCU.
// Also used by TelematicsManager to receive CAN data from the NXP MCU.
// The two roles are mutually exclusive: telematics pauses while FOTA is active.
#define FOTA_SERIAL_BAUD_RATE 115200
#define FOTA_SERIAL_RX_PIN    16
#define FOTA_SERIAL_TX_PIN    17
#define FOTA_SERIAL_HW        Serial1

// ===== DEBUG / LOGGER SERIAL (Serial0 — USB-UART chip) =====
#define DEBUG_SERIAL_BAUD_RATE 921600
#define DEBUG_SERIAL_RX_PIN    3
#define DEBUG_SERIAL_TX_PIN    1
#define DEBUG_SERIAL_HW        Serial0

// ===== CONFIG SWITCH =====
#define MP_CFG_SW GPIO_NUM_34

#endif // _HEADER_ANCIT_PINS_H_
