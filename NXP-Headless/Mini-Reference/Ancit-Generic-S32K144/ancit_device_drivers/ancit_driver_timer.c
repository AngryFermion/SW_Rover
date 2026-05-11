/*
 * ancit_dd_ISR.c
 *
 *  Created on: 19-Jun-2024
 *      Author: Narayan
 */
#include <ancit_driver_timer.h>
#include "sdk_project_config.h"
#include "ancit_timer.h"

/*!
 * @brief: LPTMR IRQ handler.
 */
void LPTMR0_IRQHandler(void) {
	/* Clear compare flag */
	LPTMR_DRV_ClearCompareFlag(INST_LPTMR_1);

	ancit_one_ms_handler();
}

void ancit_dd_timer_init(const uint32_t instance,
		const lptmr_config_t *const config, const bool startCounter) {
	LPTMR_DRV_Init(instance, config, startCounter);
}

void ancit_dd_timer_EnableIRQ(IRQn_Type irqNumber) {
	/* Enable IRQ for LPTMR */
	INT_SYS_EnableIRQ(irqNumber);
}

void ancit_dd_timer_StartCounter(const uint32_t instance) {
	/* Start LPTMR counter */
	LPTMR_DRV_StartCounter(instance);
}
