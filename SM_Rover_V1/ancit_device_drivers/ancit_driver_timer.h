/*
 * ancit_dd_timer.h
 *
 *  Created on: 20-Jun-2024
 *      Author: Narayan
 */

#ifndef ANCIT_DRIVER_TIMER_H_
#define ANCIT_DRIVER_TIMER_H_
#include "sdk_project_config.h"

void ancit_dd_timer_init(const uint32_t instance,
		const lptmr_config_t *const config, const bool startCounter);

void ancit_dd_timer_EnableIRQ(IRQn_Type irqNumber);

void ancit_dd_timer_StartCounter(const uint32_t instance);

#endif /* ANCIT_DRIVER_TIMER_H_ */
