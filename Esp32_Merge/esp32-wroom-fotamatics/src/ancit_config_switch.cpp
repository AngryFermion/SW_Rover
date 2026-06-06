#include <Arduino.h>
#include <ancit_config_switch.h>
#include <ancit_pins.h>
#include <ancit_tasks.h>
#include <BT_LOGGER.h>
#include <esp_task_wdt.h>

// Internal variables
static ConfigSwitchCallback_t eventCallback = NULL;
static ConfigSwitchEvent_t lastEvent = CFG_SW_EVENT_NONE;

// Task handle for config switch
TaskHandle_t ConfigSwitch_task_handle = NULL;

// Simple duration-based approach
static unsigned long lastDebounceTime = 0;
static unsigned long pressStartTime = 0;
static bool lastButtonState = HIGH;  // Assuming active LOW with pullup
static bool buttonState = HIGH;
static bool buttonPressed = false;
static bool holdTriggered = false;
static bool firstRead = true;

// Initialize the config switch
void ConfigSwitch_Init(void) {
  pinMode(MP_CFG_SW, INPUT);  // External pullup exists
  lastEvent = CFG_SW_EVENT_NONE;
  buttonPressed = false;
  holdTriggered = false;
  firstRead = true;

  g_Logger.WriteImmediate(LogLevel::Debug, LogCategory::SETUP, "ConfigSwitch_Init",
                 "Initialized on GPIO %d", MP_CFG_SW);
}

// Register callback for events
void ConfigSwitch_RegisterCallback(ConfigSwitchCallback_t callback) {
  eventCallback = callback;
}

// Get last event
ConfigSwitchEvent_t ConfigSwitch_GetLastEvent(void) {
  return lastEvent;
}

// Check if switch is currently pressed (for boot-time detection)
// This can be called before the task starts to determine boot mode
bool ConfigSwitch_IsPressed(void) {
  return (digitalRead(MP_CFG_SW) == LOW);
}

// Trigger event (internal)
static void triggerEvent(ConfigSwitchEvent_t event) {
  lastEvent = event;

  const char* eventName[] = {
    "NONE",
    "SHORT_PRESS",
    "LONG_PRESS",
    "HOLD"
  };

  g_Logger.Write(LogLevel::Info, LogCategory::SETUP, "ConfigSwitch",
                 "Event: %s", eventName[event]);

  if (eventCallback != NULL) {
    eventCallback(event);
  }
}

// Main task
void ConfigSwitch_Task(void *param) {
  g_Logger.Write(LogLevel::Debug, LogCategory::SETUP, "ConfigSwitch_Task",
                 "Task started on Core %d", xPortGetCoreID());

  // Add periodic debug logging
  unsigned long lastDebugLog = 0;

  for (;;) {
    // Read current button state with debouncing
    int reading = digitalRead(MP_CFG_SW);
    unsigned long currentTime = millis();

    // On first read, initialize state without triggering events
    if (firstRead) {
      lastButtonState = reading;
      buttonState = reading;
      firstRead = false;
      g_Logger.Write(LogLevel::Debug, LogCategory::SETUP, "ConfigSwitch_Task",
                     "Initial state: %s", reading == LOW ? "PRESSED" : "RELEASED");
      vTaskDelay(pdMS_TO_TICKS(CFG_SW_TASK_DELAY_MS));
      continue;
    }

#if 0    
    // Debug: Log button state every 5 seconds
    if (currentTime - lastDebugLog > 5000) {
      lastDebugLog = currentTime;
      g_Logger.Write(LogLevel::Debug, LogCategory::SETUP, "ConfigSwitch_Task",
                     "Running... Pin=%d, State=%s, Pressed=%d", MP_CFG_SW,
                     reading == LOW ? "LOW" : "HIGH", buttonPressed);
    }
#endif

    // Debounce logic
    if (reading != lastButtonState) {
      lastDebounceTime = currentTime;
    }

    if ((currentTime - lastDebounceTime) > CFG_SW_DEBOUNCE_TIME) {
      // Button state is stable
      if (reading != buttonState) {
        buttonState = reading;

        // Debug: Log state change
        g_Logger.Write(LogLevel::Debug, LogCategory::SETUP, "ConfigSwitch_Task",
                       "Button state changed to: %s", buttonState == LOW ? "PRESSED" : "RELEASED");

        // Button pressed (active LOW)
        if (buttonState == LOW) {
          // Start tracking press duration
          pressStartTime = currentTime;
          buttonPressed = true;
          holdTriggered = false;
        }
        // Button released - determine event based on duration (only if hold not already triggered)
        else if (buttonPressed) {
          if (!holdTriggered) {
            unsigned long pressDuration = currentTime - pressStartTime;

            g_Logger.Write(LogLevel::Debug, LogCategory::SETUP, "ConfigSwitch_Task",
                           "Press duration: %lu ms", pressDuration);

            // Determine event type based on duration
            if (pressDuration < CFG_SW_SHORT_PRESS_MAX) {
              triggerEvent(CFG_SW_EVENT_SHORT_PRESS);
            } else if (pressDuration < CFG_SW_LONG_PRESS_MAX) {
              triggerEvent(CFG_SW_EVENT_LONG_PRESS);
            }
            // Hold would have been triggered while button was held
          }

          buttonPressed = false;
          holdTriggered = false;
        }
      }
    }

    // Check for hold while button is still held
    if (buttonPressed && !holdTriggered) {
      unsigned long pressDuration = currentTime - pressStartTime;
      if (pressDuration >= CFG_SW_HOLD_MIN) {
        g_Logger.Write(LogLevel::Debug, LogCategory::SETUP, "ConfigSwitch_Task",
                       "Hold detected: %lu ms", pressDuration);
        triggerEvent(CFG_SW_EVENT_HOLD);
        holdTriggered = true;
      }
    }

    lastButtonState = reading;

    // Feed watchdog timer
    esp_task_wdt_reset();

    vTaskDelay(pdMS_TO_TICKS(CFG_SW_TASK_DELAY_MS));
  }
}

// Example callback function - customize this for your application
void ConfigSwitch_EventCallback(ConfigSwitchEvent_t event) {
  switch (event) {
    case CFG_SW_EVENT_SHORT_PRESS:
      g_Logger.Write(LogLevel::Info, LogCategory::SETUP, "ConfigSwitch",
                     "Short Press (< 1s) - Example: Toggle mode");
      // Add your short press action here
      // Example: toggle_mode();
      break;

    case CFG_SW_EVENT_LONG_PRESS:
      g_Logger.Write(LogLevel::Info, LogCategory::SETUP, "ConfigSwitch",
                     "Long Press (1-3s) - Example: Toggle Bluetooth mode");
      // Add your long press action here
      // Example: toggle_bluetooth_mode();
      break;

    case CFG_SW_EVENT_HOLD:
      g_Logger.Write(LogLevel::Info, LogCategory::SETUP, "ConfigSwitch",
                     "Hold (>5s) - Example: Factory reset");
      // Add your hold action here
      // Example: factory_reset();
      break;

    default:
      break;
  }
}

// Initialize and start the config switch task
void ConfigSwitch_Start(void) {
  // Initialize the config switch hardware
  ConfigSwitch_Init();

  // Register the callback function
  ConfigSwitch_RegisterCallback(ConfigSwitch_EventCallback);

  // Create the FreeRTOS task
  BaseType_t result = xTaskCreatePinnedToCore(
      ConfigSwitch_Task,                    // Task function
      "ConfigSwitch_Task",                  // Task name
      ANCIT_CONFIG_SWITCH_TASK_STACK_SIZE, // Stack size
      NULL,                                 // Parameters
      ANCIT_CONFIG_SWITCH_TASK_PRIORITY,   // Priority
      &ConfigSwitch_task_handle,           // Task handle
      ANCIT_CONFIG_SWITCH_TASK_CORE);      // Core affinity (Core 0)

  if (result != pdPASS) {
    g_Logger.WriteImmediate(LogLevel::Error, LogCategory::SETUP, "ConfigSwitch_Start",
                   "FAILED to create task! Error=%d", result);
  }
}
