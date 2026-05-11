#include "AncitRgbLed.h"
#include <ancit_pins.h>
#include <ancit_tasks.h>
#include <BT_LOGGER.h>
#include <esp_task_wdt.h>


// Global LED instances (pointers to avoid early initialization)
AncitRgbLed* netLed = nullptr;
// AncitRgbLed* statusLed = nullptr;  // Uncomment if using second LED

TaskHandle_t rgbLedTaskHandle = NULL;

// Constructor
AncitRgbLed::AncitRgbLed(uint8_t redPin, uint8_t greenPin, uint8_t bluePin, bool activeLow)
  : activeLow(activeLow),
    inBlinkOnce(false),
    fastCounter(0),
    slowCounter(0),
    blinkOnceCounter(0),
    blinkPhase(false),
    alternatePhase(false) {
  pins[0] = redPin;
  pins[1] = greenPin;
  pins[2] = bluePin;
}

// Initialize hardware
void AncitRgbLed::begin() {
  uint8_t offState = activeLow ? HIGH : LOW;
  for (int i = 0; i < 3; i++) {
    pinMode(pins[i], OUTPUT);
    digitalWrite(pins[i], offState);
  }
}

// Set LED state
void AncitRgbLed::setState(LedMode mode, LedColor color1, LedColor color2) {
  // If setting BLINK_ONCE, treat it as temporary (don't update base state)
  if (mode == LedMode::BLINK_ONCE) {
    if (!inBlinkOnce) {  // Only trigger if not already blinking once
      activeState.mode = mode;
      activeState.color1 = color1;
      activeState.color2 = color2;
      inBlinkOnce = true;
      blinkOnceCounter = LED_BLINK_ONCE_CYCLES;

      // Reset counters for BLINK_ONCE
      fastCounter = 0;
      slowCounter = 0;
      blinkPhase = false;
      alternatePhase = false;
    }
  } else {
    // Normal state - update base state
    baseState.mode = mode;
    baseState.color1 = color1;
    baseState.color2 = color2;

    // If not in BLINK_ONCE, also update active state
    if (!inBlinkOnce) {
      activeState = baseState;

      // Reset counters when state changes
      fastCounter = 0;
      slowCounter = 0;
      blinkPhase = false;
      alternatePhase = false;
    }
  }
}

// Main update function - call every 50ms
void AncitRgbLed::update() {
  // Handle BLINK_ONCE completion (auto-restore)
  if (inBlinkOnce) {
    if (blinkOnceCounter > 0) {
      blinkOnceCounter--;
      if (blinkOnceCounter == 0) {
        // BLINK_ONCE finished - restore to base state
        inBlinkOnce = false;
        activeState = baseState;
        fastCounter = 0;
        slowCounter = 0;
        blinkPhase = false;
        alternatePhase = false;
      }
    }
  }

  applyState(activeState);
}

// Apply current state to LEDs
void AncitRgbLed::applyState(const LedState& state) {
  switch (state.mode) {
    case LedMode::OFF:
      allOff();
      break;

    case LedMode::ON:
      allOff();
      setColor(state.color1, true);
      break;

    case LedMode::BLINK_FAST:
      // Toggle every cycle (50ms = 20Hz, so 10Hz blink)
      fastCounter++;
      if (fastCounter >= LED_FAST_CYCLES) {
        blinkPhase = !blinkPhase;
        fastCounter = 0;
      }
      allOff();
      if (blinkPhase) {
        setColor(state.color1, true);
      }
      break;

    case LedMode::BLINK_SLOW:
      // Toggle every 10 cycles (500ms = 1Hz blink)
      slowCounter++;
      if (slowCounter >= LED_SLOW_CYCLES) {
        blinkPhase = !blinkPhase;
        slowCounter = 0;
      }
      allOff();
      if (blinkPhase) {
        setColor(state.color1, true);
      }
      break;

    case LedMode::BLINK_ONCE:
      // Controlled by blinkOnceCounter
      allOff();
      if (blinkOnceCounter > 1 && blinkOnceCounter <= 4) {
        setColor(state.color1, true);
      }
      break;

    case LedMode::ALTERNATE_FAST:
      // Alternate colors every cycle (50ms)
      fastCounter++;
      if (fastCounter >= LED_ALTERNATE_FAST_CYCLES) {
        alternatePhase = !alternatePhase;
        fastCounter = 0;
      }
      allOff();
      if (alternatePhase) {
        setColor(state.color1, true);
      } else {
        setColor(state.color2, true);
      }
      break;

    case LedMode::ALTERNATE_SLOW:
      // Alternate colors every 10 cycles (500ms)
      slowCounter++;
      if (slowCounter >= LED_ALTERNATE_SLOW_CYCLES) {
        alternatePhase = !alternatePhase;
        slowCounter = 0;
      }
      allOff();
      if (alternatePhase) {
        setColor(state.color1, true);
      } else {
        setColor(state.color2, true);
      }
      break;

    default:
      allOff();
      break;
  }
}

// Set a specific color on/off
void AncitRgbLed::setColor(LedColor color, bool on) {
  if (color == LedColor::NONE) return;
  uint8_t value = on ? (activeLow ? LOW : HIGH) : (activeLow ? HIGH : LOW);
  digitalWrite(pins[static_cast<uint8_t>(color)], value);
}

// Turn all LEDs off
void AncitRgbLed::allOff() {
  uint8_t offState = activeLow ? HIGH : LOW;
  for (int i = 0; i < 3; i++) {
    digitalWrite(pins[i], offState);
  }
}

// LED control task - runs at 50ms interval
void RgbLed_Task(void* param) {
  const TickType_t xFrequency = pdMS_TO_TICKS(50);
  TickType_t xLastWakeTime = xTaskGetTickCount();

  g_Logger.Write(LogLevel::Debug, LogCategory::SETUP, "RgbLed_Task",
                 "Task started on Core %d", xPortGetCoreID());

  esp_task_wdt_add(NULL);

  for (;;) {
    vTaskDelayUntil(&xLastWakeTime, xFrequency);
    esp_task_wdt_reset();

    // Update all LED instances
    if (netLed) netLed->update();
    // if (statusLed) statusLed->update();  // If using second LED
  }
}

// Initialize and start the RGB LED task
void RgbLed_Start(void) {
  // Create LED instances
  netLed = new AncitRgbLed(MP_NET_LED_RED, MP_NET_LED_GREEN, MP_NET_LED_BLUE, false);  // false = active-high
  // statusLed = new AncitRgbLed(GPIO_NUM_12, GPIO_NUM_13, GPIO_NUM_14, true);  // true = active-low

  // Initialize hardware
  netLed->begin();
  // if (statusLed) statusLed->begin();  // If using second LED

  // Set initial states (examples)
  netLed->setState(LedMode::ON, LedColor::RED);
  // if (statusLed) statusLed->setState(LedMode::OFF, LedColor::NONE);

  // Create the FreeRTOS task
  BaseType_t result = xTaskCreatePinnedToCore(
      RgbLed_Task,                    // Task function
      "RgbLed_Task",                  // Task name
      ANCIT_LED_TASK_STACK_SIZE,      // Stack size
      NULL,                           // Parameters
      ANCIT_LED_TASK_PRIORITY,        // Priority
      &rgbLedTaskHandle,              // Task handle
      ANCIT_LED_TASK_CORE);           // Core affinity

  if (result != pdPASS) {
    g_Logger.WriteImmediate(LogLevel::Error, LogCategory::SETUP, "RgbLed_Start",
                            "FAILED to create task! Error=%d", result);
  }
}
