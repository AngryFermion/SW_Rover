#include "ancit_time.h"

#include <Arduino.h>
#include <BT_LOGGER.h>
#include <ancit_device.h>
#include <sys/time.h>
#include <time.h>
#include "config_reader.h"

void mtime_init(void) {
  // Narayan - this needs to be fixed
  globals.time.sync_server = SYNC_SERVER_NTP;
  globals.time.valid = false;

  // Get the time zone offset from the device configuration
  float tzHours = devComm_doc["user"]["timeZoneDiff"].as<float>();
  int32_t tzOffsetSec = static_cast<int32_t>(tzHours * 3600);

  if (tzHours < -12.0f || tzHours > 14.0f) {
    g_Logger.Write(LogLevel::Error, LogCategory::DEVICE, "ntp_setup",
                     "Invalid timeZoneDiff: %.2f", tzHours);
  } else {
    globals.time.zoneOffset = tzOffsetSec;
    globals.time.esp32_time.offset = tzOffsetSec;
    g_Logger.Write(LogLevel::Info, LogCategory::DEVICE, "mtime_init",
                     "Time zone offset set: %.2f hours (%d seconds)", tzHours,
                     tzOffsetSec);
  }
}

void mtime_get_time_format(time_t ts, char *retString) {
  struct tm *tmp = localtime(&ts);
  sprintf(retString, "%04d-%02d-%02d %02d:%02d:%02d", tmp->tm_year + 1900,
          tmp->tm_mon + 1, tmp->tm_mday, tmp->tm_hour, tmp->tm_min,
          tmp->tm_sec);

  g_Logger.Write(LogLevel::Trace, LogCategory::OTHERS, "live_get_time_format",
                   "%d-%d-%d %d:%d:%d", tmp->tm_year + 1900, tmp->tm_mon + 1,
                   tmp->tm_mday, tmp->tm_hour, tmp->tm_min, tmp->tm_sec);
}

bool getLocalTimeNew(struct tm *info) {
  uint32_t start = millis();

  // Attempt to get the local time, retrying for up to 1 second
  while ((millis() - start) < 1000) {
    // Get the current time adjusted by the zone offset
    time_t now = time(nullptr) + globals.time.zoneOffset;

    localtime_r(&now, info);
    if (info->tm_year > (2016 - 1900)) {
      return true;
    }
    delay(10);
  }
  return false;
}

uint32_t mtime_get_curr_timestamp() {
  time_t now;
  time(&now);
  return now;
}

uint32_t mtime_get_unix_timestamp() {
  return time(nullptr);
}

String mtime_get_hhmmss() {
  time_t now = time(nullptr);
  char buff[20];
  strftime(buff, sizeof(buff), "%H:%M:%S", localtime(&now));
  return String(buff);
}

String mtime_get_datetime() {
  time_t now = time(nullptr);
  char buff[25];
  strftime(buff, sizeof(buff), "%Y-%m-%d %H:%M:%S", localtime(&now));
  return String(buff);
}

bool mtime_check_valid(void) {
  struct tm timeinfo;
  return getLocalTimeNew(&timeinfo);
}

#if 0
void TimeManager_SetLocalTime(int year, int month, int day,
                              int hour, int minute, int second) {
  // Store current offset temporarily
  int tzOffset = globals.time.esp32_time.offset;

  // Temporarily clear offset so setTime() stores local time as UTC
  globals.time.esp32_time.offset = 0;
  globals.time.esp32_time.setTime(second, minute, hour, day, month, year);
  globals.time.esp32_time.offset = tzOffset;

  // Optional: mark time valid
  globals.time.valid = true;

  // g_Logger.Write(LogLevel::Info, LogCategory::TIME, "TimeManager_SetLocalTime",
  //                  "Local time set to: %s", globals.time.esp32_time.getDateTime().c_str());
}

#endif