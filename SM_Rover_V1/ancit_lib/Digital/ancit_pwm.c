/*
 * ancit_PWM.c
 *
 * ANCIT FlexTimer PWM Driver Implementation
 *
 * This module provides a flexible PWM initialization system that bridges
 * S32 Design Studio auto-generated configurations with user-defined channel
 * configurations.A
 *
 * Copyright (c) 2025 ANCIT
 */

#include "ancit_pwm.h"

#ifdef PWM_CONFIGURED

/*******************************************************************************
 * External Variables (Weak Declarations)
 ******************************************************************************/

/* Weak declarations - should be defined in genx_PWM.c */
__attribute__((weak)) extern PWM_registration_params_t genx_pwm_reg[];
__attribute__((weak)) extern ftm_state_t genx_pwm_0_ftmStateStruct;

/*******************************************************************************
 * Private Functions
 ******************************************************************************/

/**
 * @brief Initialize a single FTM PWM instance
 *
 * This function initializes a specific FTM instance with its configured channels.
 * All configuration pointers are contained within the pInstanceConfig_genx structure.
 *
 * @param pInstanceConfig_genx - Pointer to the instance configuration structure (contains all configs)
 * @return status_t - STATUS_SUCCESS if initialization is successful, error code otherwise
 */
static status_t ancit_PWM_InitInstance(PWM_registration_params_t* pInstanceConfig_genx) {
	status_t error;

	/* Initialize the FTM hardware instance */
	error = FTM_DRV_Init(pInstanceConfig_genx->instanceIdx,
	                      pInstanceConfig_genx->pInitConfig_s32ds,
	                      pInstanceConfig_genx->pStateStruct_genx);
	if (error != STATUS_SUCCESS) {
		return error;
	}

	/* Modify the S32DS PWM config to use instance channel configuration */
	pInstanceConfig_genx->pPwmConfig_s32ds->nNumIndependentPwmChannels = pInstanceConfig_genx->numChannels;
	pInstanceConfig_genx->pPwmConfig_s32ds->uFrequencyHZ = pInstanceConfig_genx->uFrequencyHZ;
	pInstanceConfig_genx->pPwmConfig_s32ds->pwmIndependentChannelConfig = pInstanceConfig_genx->channels;

	/* Initialize the FTM PWM channels */
	error = FTM_DRV_InitPwm(pInstanceConfig_genx->instanceIdx,
	                         pInstanceConfig_genx->pPwmConfig_s32ds);
	if (error != STATUS_SUCCESS) {
		return error;
	}

	return STATUS_SUCCESS;
}

/*******************************************************************************
 * Public Functions
 ******************************************************************************/

/**
 * @brief Initialize FlexTimer PWM module
 *
 * This function initializes all configured FTM PWM instances with their
 * channel configurations defined in genx_pwm_reg[].
 *
 * @return status_t - STATUS_SUCCESS if initialization is successful, error code otherwise
 */
status_t ancit_PWM_Init(void) {
	status_t error;
	uint8_t i;

	/* Initialize each FTM instance */
	for (i = 0; i < TOTAL_PWM_INSTANCES; i++) {
		if (genx_pwm_reg[i].numChannels == 0) {
			continue;  /* Skip instances with no channels */
		}

		/* Initialize the FTM instance - all configs are in the structure */
		error = ancit_PWM_InitInstance(&genx_pwm_reg[i]);
		if (error != STATUS_SUCCESS) {
			return error;
		}
	}

	return STATUS_SUCCESS;
}

/**
 * @brief Update PWM channel duty cycle
 *
 * This function updates the duty cycle of a specific PWM channel.
 *
 * @param instanceIdx - FTM instance index (e.g., INST_FLEXTIMER_PWM_0)
 * @param hwChannel - Hardware channel ID (e.g., PWM_HW_CHANNEL_1)
 * @param dutyPercent - Duty cycle percentage (0-100)
 */
void ancit_PWM_UpdateDutyCycle(uint8_t instanceIdx, uint8_t hwChannel, uint8_t dutyPercent) {
	uint16_t dutyValue;

	/* Limit duty cycle to 0-100% */
	if (dutyPercent > 100) {
		dutyPercent = 100;
	}

	/* Scale percentage (0-100) to FTM duty value (0-32767) */
	dutyValue = (uint16_t)((dutyPercent * 32767) / 100);

	/* Update the PWM channel */
	FTM_DRV_UpdatePwmChannel(instanceIdx,
	                          hwChannel,
	                          FTM_PWM_UPDATE_IN_DUTY_CYCLE,
	                          dutyValue,
	                          0,    /* secondEdge not used in edge-aligned PWM */
	                          true);
}

void ancit_PWM_EnableChannel(uint8_t instanceIdx, uint8_t hwChannel) {
	FTM_DRV_ControlChannelOutput(instanceIdx, hwChannel, true);
}

void ancit_PWM_DisableChannel(uint8_t instanceIdx, uint8_t hwChannel) {
	FTM_DRV_ControlChannelOutput(instanceIdx, hwChannel, false);
}

#endif /* PWM_CONFIGURED */
