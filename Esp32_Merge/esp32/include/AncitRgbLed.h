#ifndef _ANCIT_RGB_LED_H_
#define _ANCIT_RGB_LED_H_

#include <Arduino.h>

// LED timing configuration (in 50ms cycles)
#define LED_FAST_CYCLES     1    // 1 * 50ms = 50ms toggle (10Hz blink)
#define LED_SLOW_CYCLES     14   // 14 * 50ms = 700ms toggle (1Hz blink)
#define LED_BLINK_ONCE_CYCLES  7 // 5 * 50ms = 250ms blink
#define LED_ALTERNATE_FAST_CYCLES 3  // 3 * 50ms = 150ms toggle (3.33Hz alternate)
#define LED_ALTERNATE_SLOW_CYCLES 14 // 14 * 50ms

// LED Colors
enum class LedColor {
  RED = 0,
  GREEN = 1,
  BLUE = 2,
  NONE = 255  // For OFF state
};

// LED Modes
enum class LedMode {
  OFF = 0,
  ON,
  BLINK_FAST,
  BLINK_SLOW,
  BLINK_ONCE,
  ALTERNATE_FAST,  // Alternates between two colors (fast)
  ALTERNATE_SLOW   // Alternates between two colors (slow)
};

// LED State structure
struct LedState {
  LedMode mode;
  LedColor color1;      // Primary color
  LedColor color2;      // Secondary color (for alternate modes)

  LedState() : mode(LedMode::OFF), color1(LedColor::NONE), color2(LedColor::NONE) {}
  LedState(LedMode m, LedColor c1, LedColor c2 = LedColor::NONE)
    : mode(m), color1(c1), color2(c2) {}
};

class AncitRgbLed {
public:
  // Constructor: Initialize with GPIO pins for R, G, B
  // activeLow: true = LED on when pin is LOW, false = LED on when pin is HIGH
  AncitRgbLed(uint8_t redPin, uint8_t greenPin, uint8_t bluePin, bool activeLow = false);

  // Initialize the LED hardware
  void begin();

  // Set LED state
  // Note: BLINK_ONCE is automatically handled as temporary and restores to previous state
  void setState(LedMode mode, LedColor color1, LedColor color2 = LedColor::NONE);

  // Update function - call this periodically (e.g., every 50ms)
  void update();

  // Get current base state
  LedState getBaseState() const { return baseState; }

private:
  // Pin assignments
  uint8_t pins[3];  // RED, GREEN, BLUE
  bool activeLow;   // true = LED on when pin LOW, false = LED on when pin HIGH

  // State management
  LedState baseState;      // Persistent state
  LedState activeState;    // Currently executing state (may be BLINK_ONCE)
  bool inBlinkOnce;        // True if BLINK_ONCE is active

  // Timing variables
  uint8_t fastCounter;      // Fast blink/alternate counter
  uint8_t slowCounter;      // Slow blink/alternate counter
  uint8_t blinkOnceCounter; // BLINK_ONCE timer
  bool blinkPhase;          // Current blink phase (on/off)
  bool alternatePhase;      // Current alternate phase (color1/color2)

  // Internal methods
  void applyState(const LedState& state);
  void setColor(LedColor color, bool on);
  void allOff();
};

// Global LED instances (pointers)
extern AncitRgbLed* netLed;
// extern AncitRgbLed* statusLed;  // Uncomment if using second LED

// Task functions
void RgbLed_Task(void* param);
void RgbLed_Start(void);

#endif // _ANCIT_RGB_LED_H_
