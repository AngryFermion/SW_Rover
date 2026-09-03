#include <ancit_driver_timer.h>
#include "ancit_timer.h"
#include "ancit_ultrasonic.h"
#include "ancit_scheduler.h"

#ifdef CAN_TX_CONFIGURED
#include "ancit_can_tx.h"
#endif

#ifdef ADC_CONFIGURED
#include "ancit_adc.h"
#endif

// Global variable to store the tick count
volatile uint32_t g_tickCount = 0;

void ancit_base_timer_start(void) {
	/* Init LPTMR as timer
	 *  -   interrupt after 1 second
	 *  -   SIRC as clock source
	 *  -   counter disabled
	 */
	ancit_dd_timer_init(INST_LPTMR_1, &lptmr_1_config0, false);
	/* Enable IRQ for LPTMR */
	ancit_dd_timer_EnableIRQ(LPTMR0_IRQn);

	/* Start LPTMR counter */
	ancit_dd_timer_StartCounter(INST_LPTMR_1);
}

void ancit_one_ms_handler(void) {
	g_tickCount++;

#ifdef CAN_TX_CONFIGURED
	ancit_can_tx_one_ms();
#endif //End of CAN_TX_CONFIGURED

#ifdef ADC_CONFIGURED
	ancit_adc_one_ms();
#endif //End of ADC_CONFIGURED

#ifdef SCHEDULER_CONFIGURED
	ancit_scheduler_one_ms();
#endif //SCHEDULER_CONFIGURED

#ifdef ULTRASONIC_CONFIGURED
    ancit_ultrasonic_one_ms();
#endif
}

uint32_t ancit_GetTick(void) {
	return g_tickCount;
}

// Function to initialize the timeout timer
void ancit_init_timeout(uint32_t *timer, uint32_t timeout) {
	*timer = ancit_GetTick() + timeout;
}

// Function to check if the timeout period has expired
uint8_t is_timeout_expired(uint32_t *timer) {
	uint32_t current_tick = ancit_GetTick();

	// Check if the current tick count has reached or passed the timer value
	// This works correctly even if current_tick has overflowed
	if ((current_tick - *timer) < (UINT32_MAX / 2)) {
		return 1; // Timeout expired
	}
	return 0; // Timeout not expired
}
