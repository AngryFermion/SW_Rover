#include "ancit_time_manager.h"
#include <ArduinoJson.h>
#include <BT_LOGGER.h>
#include "ancit_device.h"

time_t TimeManager_GetUtcTimestamp() {
  return time(nullptr);
}

time_t TimeManager_GetLocalTimestamp() {
  return time(nullptr) + globals.time.zoneOffset;
}

void TimeManager_SetUtcTimestamp(time_t utc_ts) {
  globals.time.esp32_time.setTime(utc_ts);
  globals.time.valid = true;
}

void TimeManager_SetLocalTimestamp(time_t local_ts) {
  TimeManager_SetUtcTimestamp(local_ts - globals.time.zoneOffset);
}

void TimeManager_SetLocalTime(int year, int month, int day,
                              int hour, int minute, int second) {
  struct tm local_tm = {0};
  local_tm.tm_year = year - 1900;
  local_tm.tm_mon = month - 1;
  local_tm.tm_mday = day;
  local_tm.tm_hour = hour;
  local_tm.tm_min = minute;
  local_tm.tm_sec = second;

  time_t local_ts = mktime(&local_tm);  // Local timestamp
  TimeManager_SetLocalTimestamp(local_ts);  // Convert and store as UTC
}

bool TimeManager_GetLocalTime(struct tm* info) {
  time_t now = TimeManager_GetLocalTimestamp();
  localtime_r(&now, info);
  return info->tm_year > (2016 - 1900);
}

String TimeManager_GetTimeHHMMSS() {
  time_t now = TimeManager_GetLocalTimestamp();
  char buff[20];
  strftime(buff, sizeof(buff), "%H:%M:%S", localtime(&now));
  return String(buff);
}

String TimeManager_GetDatetimeString() {
  time_t now = TimeManager_GetLocalTimestamp();
  char buff[25];
  strftime(buff, sizeof(buff), "%Y-%m-%d %H:%M:%S", localtime(&now));
  return String(buff);
}

void TimeManager_FormatTimestamp(time_t ts, char* outStr) {
  struct tm* tmp = localtime(&ts);
  sprintf(outStr, "%04d-%02d-%02d %02d:%02d:%02d",
          tmp->tm_year + 1900, tmp->tm_mon + 1, tmp->tm_mday,
          tmp->tm_hour, tmp->tm_min, tmp->tm_sec);
}

bool TimeManager_IsValid() {
  struct tm timeinfo;
  return TimeManager_GetLocalTime(&timeinfo);
}   