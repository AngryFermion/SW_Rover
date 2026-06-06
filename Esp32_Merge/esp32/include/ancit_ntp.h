#ifndef _ANCIT_NTP_H
#define _ANCIT_NTP_H 1

#include <Arduino.h>
#include <stdint.h>
#include <time.h>
#include <ESP32Time.h>

#ifdef __cplusplus
extern "C" {
#endif

#define NTP_EVENT_START_BIT (1 << 0)

extern EventGroupHandle_t ntp_event_group;

void ntp_setup(void);
void ntp_start(void);
void ntp_task(void *param);

#ifdef __cplusplus
}
#endif

#endif