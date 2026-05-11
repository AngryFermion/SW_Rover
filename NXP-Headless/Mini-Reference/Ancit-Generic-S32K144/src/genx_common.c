/*
 * genx_common.c
 *
 *  Created on: 19-Jun-2024
 *      Author: Narayan
 */
#include "genx_config.h"

void genx_clock_init(void) {
	/* Write your local variable definition here */
	status_t status;

	/* Initialize clock module */
	status = CLOCK_DRV_Init(&clockMan1_InitConfig0);
	DEV_ASSERT(status == STATUS_SUCCESS);

	/* Initialize pins
	 *  -   Init FlexCAN and GPIO pins
	 *  -   See PinSettings component for more info
	 */
	PINS_DRV_Init(NUM_OF_CONFIGURED_PINS0, g_pin_mux_InitConfigArr0);

	/* Initialize and configure clocks
	 *  -   Setup system clocks, dividers
	 *  -   Configure FlexCAN clock, GPIO
	 *  -   see clock manager component for more details
	 */
	CLOCK_DRV_Init(&clockMan1_InitConfig0);

	CLOCK_SYS_Init(g_clockManConfigsArr, CLOCK_MANAGER_CONFIG_CNT,
			g_clockManCallbacksArr, CLOCK_MANAGER_CALLBACK_CNT);
	CLOCK_SYS_UpdateConfiguration(0U, CLOCK_MANAGER_POLICY_AGREEMENT);
}
