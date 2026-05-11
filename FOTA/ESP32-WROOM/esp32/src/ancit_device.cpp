#include <Arduino.h>
#include <MD5Builder.h>
#include <SPIFFS.h>
#include <config_reader.h>
#include <BT_LOGGER.h>
#include <main.h>
#include <ancit_device.h>
#include <ancit_pins.h>
// #include <ancit_modbus_fixed.h>
#include <AncitRgbLed.h>
#include "BT_LOGGER.h"

globals_t globals;

MD5Builder _md5;

// bool g_valid_subscription = false;

//Handles all device related events
// like - device data ready event 
EventGroupHandle_t device_event_group;

// void setup_log_category(void) {
//   uint16 logCat;

//   logCat = 0;
//   logCat |= LogCategory::HTTP;
//   logCat |= LogCategory::SETUP;
//   logCat |= LogCategory::WIFI;
//   logCat |= LogCategory::SPIFFS;
//   logCat |= LogCategory::CAN;
//   logCat |= LogCategory::SERVER;
//   logCat |= LogCategory::FOTA;
//   logCat |= LogCategory::MQTT;
//   logCat |= LogCategory::NVS;
//   logCat |= LogCategory::DEBUG;
//   // logCat |= LogCategory::OTHERS;
//   logCat |= LogCategory::DEVICE;
//   logCat |= LogCategory::SER;
//   gSetLogCategory = logCat;
// }

void start_file_storage(void) {
  /* Initialize SPIFFS */
  if (!SPIFFS.begin()) {
    g_Logger.WriteImmediate(LogLevel::Fatal, LogCategory::SPIFFS, "start_file_storage",
                     "An Error has occurred while mounting SPIFFS !!");

    for (size_t i = 0; i < 10; i++) {
      digitalWrite(MP_NET_LED_RED, HIGH);
      delay(200);
      digitalWrite(MP_NET_LED_RED, LOW);
      delay(200);
      digitalWrite(MP_NET_LED_GREEN, HIGH);
      delay(200);
      digitalWrite(MP_NET_LED_GREEN, LOW);
      delay(200);
      digitalWrite(MP_NET_LED_BLUE, HIGH);
      delay(200);
      digitalWrite(MP_NET_LED_BLUE, LOW);
      delay(200);
    }
    ancit_device_restart();
  } else {
    g_Logger.WriteImmediate(LogLevel::Info, LogCategory::SPIFFS, "start_file_storage",
                     "SPIFFS mounted successfully");
  }
}

#if 0 // Narayan - this is not used
bool validateDeviceSubscription(void) {
  g_valid_subscription = false;

  bool hasRegistered = registration_doc["registered"].is<bool>();
  bool hasDevKey = registration_doc["dev_key"].is<String>();

  if (hasRegistered && hasDevKey) {
    String registeredKey = registration_doc["dev_key"].as<String>();
    bool isRegistered = registration_doc["registered"].as<bool>();

    if (registeredKey == globals.device.md5 && isRegistered) {
      g_valid_subscription = true;
    }
  }

  if (g_valid_subscription) {
    g_Logger.Write(LogLevel::Info, LogCategory::DEVICE,
                     "validateDeviceSubscription", "Registered Device..");
  } else {
    g_Logger.Write(LogLevel::Error, LogCategory::DEVICE,
                     "validateDeviceSubscription", "Device Registration Failed!! %s", globals.device.md5.c_str());
    // g_pIndicators[IndicatorType::LedRed2].Blink(
    //     g_pBlinkConfig[BlinkType::Slow]);
  }

  return g_valid_subscription;
  // Narayan - testing
  // return false;
}
#endif

#if 0  // ns
void	mandatory_restart_check(void) {
    uint32_t currentMillis = millis();
    static uint32_t previousMillis = 0;  // will store last time LED was updated
    // constants won't change:

    time_t now = CDate::Now().GetUnixTimestampNew(globals.rtc.timeOffset);
    struct tm *tmp = gmtime(&now);

    // Every midnight do a restart...
    // if time between 00:00:00 and 00:00:03
    // Note: Do not do check this for for 10 seconds...
    if (currentMillis > MANDATORY_RESTART_SKIP_INTERVAL) {
        if (tmp->tm_hour == 0 && tmp->tm_min == 0) {
            if (tmp->tm_sec < 3) {
                previousMillis = currentMillis;
                g_Logger.Write(LogLevel::Info, LogCategory::DEVICE, "loop",
                                 "Daily Midnight restart....");
                ancit_device_restart();
            }
        }
    }

    // this is for double safety... if midnight restart does not happen
    // after 90000 seconds, do a restart...
    if (currentMillis - previousMillis >= MANDATORY_RESTART_INTERVAL_BACKUP) {
        // save the last time you blinked the LED
        previousMillis = currentMillis;
        g_Logger.Write(LogLevel::Info, LogCategory::DEVICE, "loop",
                         "Daily restart (BACKUP)....");
        ancit_device_restart();
    }
}
#endif

void ancit_device_restart(void) {
  // Narayan - this is not implemented yet
  // net_led_set_state(LED_COLOR_RED, LED_MODE_ON);
  delay(100);
  ESP.restart();
}

String Device_GetMd5(const String& str) {
  _md5.begin();
  _md5.add(str);
  _md5.calculate();
  return _md5.toString();
}

void setup_device_params(void) {
  String dev_id;
  String mac;

  // remove colon and replace with -
  mac = WiFi.macAddress();
  // Remove all colons in device id and add MQ1....
  dev_id = "MQI1-" + mac;
  dev_id.replace(":", "-");
  device_doc["dev_mac_addr"] = mac;
  device_doc["dev_id"] = dev_id;
  device_doc["dev_vdd"] = 3.3;
  globals.device.md5 = Device_GetMd5(dev_id + "fota dev_id");
  g_Logger.WriteImmediate(LogLevel::Info, LogCategory::DEVICE, "setup_device_params",
                   "Device ID: %s", mac.c_str());

  // Create the event group for device events
  device_event_group = xEventGroupCreate();
}

#if 0
void	validate_iot_method(void)
{
    //If mqtt is enabled, disable live if enabled....
    if (globals.mqtt.enabled == true) {
        globals.live.enabled = false;
        globals.rtc.sync_server = SYNC_SERVER_NTP;
    }
}
#endif

void Device_ValidateRegistration(void) {
#ifdef ENABLE_DEVICE_REGISTRATION_CHECK
  bool isValid = false;

  JsonVariant reg = registration_doc["registered"];
  JsonVariant key = registration_doc["dev_key"];

  if (reg.is<bool>() && reg.as<bool>() && key.is<const char*>()) {
    const char* devKey = key;
    if (strcmp(devKey, globals.device.md5.c_str()) == 0) {
      isValid = true;
    }
  }

  if (isValid) {
    g_Logger.WriteImmediate(LogLevel::Info, LogCategory::DEVICE,
                     "Device_ValidateRegistration", "Registered Device..");
    // Set event bit to indicate valid registration
    // Narayan - testing
    xEventGroupSetBits(device_event_group, EVENT_BIT_DEVICE_REGISTERED);
  } else {
    g_Logger.WriteImmediate(LogLevel::Error, LogCategory::DEVICE,
                     "Device_ValidateRegistration", "Device Registration Failed!! ");
  }
#else
  xEventGroupSetBits(device_event_group, EVENT_BIT_DEVICE_REGISTERED);
  g_Logger.WriteImmediate(LogLevel::Info, LogCategory::DEVICE,
                   "Device_ValidateRegistration", "Registration check not enabled, assuming valid.");
#endif
}

void Device_WaitForDataReady(const char* caller) {
  g_Logger.Write(LogLevel::Info, LogCategory::DEVICE, caller,
                   "Waiting for device data ready event");

  // This is a blocking call that waits for the EVENT_BIT_DEVICE_DATA_READY
  // Does not clear the event bit so other tasks can also proceed
  xEventGroupWaitBits(device_event_group,
                      EVENT_BIT_DEVICE_DATA_READY,
                      pdFALSE,  // do not clear the bit
                      pdTRUE,   // wait for all bits (just one here)
                      portMAX_DELAY);
}

bool Device_IsRegistrationValid(void) {
  EventBits_t bits = xEventGroupGetBits(device_event_group);
  return (bits & EVENT_BIT_DEVICE_REGISTERED);
}