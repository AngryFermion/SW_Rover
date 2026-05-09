/*
 * ancit_timer.h
 *
 *  Created on: 17-May-2024
 *      Author: Narayan
 */

#ifndef ANCIT_TIMER_H_
#define ANCIT_TIMER_H_
#include "genx_config.h"

typedef struct {
    uint32_t startTick;
    uint32_t timeout;
} timeout_counter_t;

void ancit_one_ms_handler(void);
void ancit_delay(uint32_t milliseconds);
void ancit_init_timeout(uint32_t *timer, uint32_t timeout);
uint8_t is_timeout_expired(uint32_t *timer);
void ancit_base_timer_start(void);

#endif
