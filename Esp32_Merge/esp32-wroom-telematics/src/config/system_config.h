/**
 * system_config.h — SmartWheels ESP32-WROOM Telematics
 */

#ifndef SYSTEM_CONFIG_H
#define SYSTEM_CONFIG_H

// ===== SCHEDULER =====
#define SCHEDULER_TICK_MS 10

// ===== RGB STATUS LED (WS2812) =====
// ESP32-WROOM DevKit: onboard addressable LED on GPIO 2
#define RGB_LED_PIN    2
#define NUM_LEDS       1
#define LED_BRIGHTNESS 50

// ===== DEBUG / LOGGING =====
#define ENABLE_SERIAL_DEBUG 1
#define SERIAL_BAUD_RATE    115200

#endif // SYSTEM_CONFIG_H
