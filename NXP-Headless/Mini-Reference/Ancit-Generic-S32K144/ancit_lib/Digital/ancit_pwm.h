/*
 * ancit_others.h
 *
 *  Created on: 16-Jun-2024
 *      Author: Narayan
 */

#ifndef ANCIT_PWM_H_
#define ANCIT_PWM_H_
#include "sdk_project_config.h"

#define PWM_CHANNEL_MAX			   1

#define PWM_CHANNEL_IDX_GREEN_LED  0

typedef struct {
    uint8_t channel_idx;
    uint16_t duty_cycle;
} pwm_registration_t;

extern pwm_registration_t pwm[PWM_CHANNEL_MAX];

void ancit_pwm_start(void);
void ancit_pwm_main(void);
void ancit_pwm_update_duty(uint8_t pwm_idx, uint16_t duty_cycle);

#endif /* ANCIT_PWM_H_ */
