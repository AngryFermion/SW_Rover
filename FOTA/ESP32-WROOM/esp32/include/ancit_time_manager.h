#ifndef _HEADER_FOTA_TIME_MANAGER_H_
#define _HEADER_FOTA_TIME_MANAGER_H_
#include <Arduino.h>
#include <stdint.h>
#include <time.h>

time_t TimeManager_GetUtcTimestamp();
time_t TimeManager_GetLocalTimestamp();
bool TimeManager_GetLocalTime(struct tm* timeinfo);  // replaces getLocalTimeNew
bool TimeManager_IsValid();

String TimeManager_GetTimeHHMMSS();      // local time
String TimeManager_GetDatetimeString();  // local time
void TimeManager_FormatTimestamp(time_t ts,
                                 char* outStr);  // yyyy-mm-dd HH:MM:SS

#endif  // _HEADER_FOTA_TIME_MANAGER_H_