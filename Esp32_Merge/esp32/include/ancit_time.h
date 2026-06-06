#ifndef _ANCIT_TIME_H
#define _ANCIT_TIME_H 1

#include <Arduino.h>
#include <stdint.h>
#include <time.h>
#include <ESP32Time.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    bool valid;
    uint8_t sync_server;
    uint32_t zoneOffset;
    ESP32Time esp32_time;    
} global_time_t;

void mtime_init(void);
void mtime_get_time_format(time_t ts, char *retString);
bool getLocalTimeNew(struct tm *info);
bool mtime_check_valid(void);
uint32_t mtime_get_curr_timestamp(void);
uint32_t mtime_get_unix_timestamp(void);
String mtime_get_hhmmss(void);
String mtime_get_datetime(void);
void TimeManager_SetLocalTime(int year, int month, int day,
                              int hour, int minute, int second);
                              
#ifdef __cplusplus
}
#endif

#endif