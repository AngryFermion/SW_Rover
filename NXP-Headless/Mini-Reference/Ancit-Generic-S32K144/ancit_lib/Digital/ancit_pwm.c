/*
 * ancit_others.c
 *
 *  Created on: 16-Jun-2024
 *      Author: Narayan
 */
#include "ancit_pwm.h"
#include "ancit_timer.h"

//pwm_registration_t pwm[PWM_CHANNEL_MAX] = {
////0
//		{ 7, 0 },
////End
//		};
//
//static status_t status = 0;
//
//void ancit_pwm_start(void) {
//	status = PWM_Init(&pwm_pal_1_instance, &pwm_pal_1_configs);
//}
//
//void ancit_pwm_update_duty(uint8_t pwm_idx, uint16_t duty_cycle) {
//	pwm[pwm_idx].duty_cycle = duty_cycle;
//
//	status = PWM_UpdateDuty(&pwm_pal_1_instance, pwm[pwm_idx].channel_idx,
//			pwm[pwm_idx].duty_cycle);
//}
//
//void ancit_pwm_main(void) {
//	//Nothing here as of now
//}
