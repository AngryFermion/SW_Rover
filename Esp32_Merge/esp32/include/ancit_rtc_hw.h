#ifndef _ANCIT_RTC_HW_H_INCLUDED_
#define _ANCIT_RTC_HW_H_INCLUDED_ 1
#include <Arduino.h>
#include <ancit_time.h>
#include <ESP32Time.h>

// Constatnts
#define SYNC_SERVER_CTR_MAX 10
#define LIVE_SERVER_TIME_ZONE_OFFSET (5.5 * 3600)

// rtc_sync_server options
#define SYNC_SERVER_NONE 0
#define SYNC_SERVER_LIVE 1
#define SYNC_SERVER_NTP 2

// Variables
// Narayan: many of these may not be needed in the future, but keeping them for now
typedef struct {
    bool valid;
    bool hw_ready;
    uint8_t sync_server;
    uint16_t sync_server_ctr;
    uint32_t timeOffset;
} global_rtc_t;

// Functions
void rtc_hw_setup(void);
void rtc_hw_set_time(uint32_t ts);
void rtc_hw_sync_to_internet_time(void);
void rtc_hw_update_esp32_time(void);
void rtc_update_with_live_time(char *response);
#endif