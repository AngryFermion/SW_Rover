/*
 * ancit_PWM.h
 *
 * ANCIT FlexTimer PWM Driver
 *
 * This module provides a flexible PWM initialization system that bridges
 * S32 Design Studio auto-generated configurations with user-defined channel
 * configurations.
 *
 * Copyright (c) 2025 ANCIT
 */

#ifndef ANCIT_PWM_H
#define ANCIT_PWM_H

#include "sdk_project_config.h"
#include "peripherals_flexTimer_pwm_0.h"
#include "genx_config.h"

#ifdef PWM_CONFIGURED

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define PWM_HW_CHANNEL_0 0
#define PWM_HW_CHANNEL_1 1
#define PWM_HW_CHANNEL_2 2
#define PWM_HW_CHANNEL_3 3
#define PWM_HW_CHANNEL_4 4
#define PWM_HW_CHANNEL_5 5
#define PWM_HW_CHANNEL_6 6
#define PWM_HW_CHANNEL_7 7

#define TOTAL_PWM_INSTANCES 1 /* Total number of FTM instances used */

#define MAX_CHANNELS_PER_INSTANCE 8U  /* Maximum channels per FTM instance */

/*******************************************************************************
 * Types
 ******************************************************************************/

/**
 * @brief PWM Registration Parameters Structure
 *
 * This structure contains all configuration needed for a single FTM PWM instance,
 * including pointers to S32DS auto-generated configs and user-defined channel configs.
 */
typedef struct {
    uint8_t instanceIdx;		/*!< FTM instance index */
    uint8_t numChannels;        /*!< Number of channels for this instance */
	uint16_t uFrequencyHZ;      /*!< PWM signal frequency in Hz */
    ftm_independent_ch_param_t channels[MAX_CHANNELS_PER_INSTANCE]; /*!< Channel configurations */
    ftm_user_config_t* pInitConfig_s32ds;   /*!< Pointer to S32DS auto-generated init config */
    ftm_pwm_param_t* pPwmConfig_s32ds;      /*!< Pointer to S32DS auto-generated PWM config */
    ftm_state_t* pStateStruct_genx;         /*!< Pointer to FTM state structure */
} PWM_registration_params_t;

/*******************************************************************************
 * External Variables
 ******************************************************************************/

/*******************************************************************************
 * API Functions
 ******************************************************************************/

/**
 * @brief Initialize FlexTimer PWM module
 *
 * This function initializes all configured FTM PWM instances with their
 * channel configurations defined in genx_pwm_reg[].
 *
 * @return status_t - STATUS_SUCCESS if initialization is successful, error code otherwise
 */
status_t ancit_PWM_Init(void);

/**
 * @brief Update PWM channel duty cycle
 *
 * This function updates the duty cycle of a specific PWM channel.
 *
 * @param instanceIdx - FTM instance index (e.g., INST_FLEXTIMER_PWM_0)
 * @param hwChannel - Hardware channel ID (e.g., PWM_HW_CHANNEL_1)
 * @param dutyPercent - Duty cycle percentage (0-100)
 */
void ancit_PWM_UpdateDutyCycle(uint8_t instanceIdx, uint8_t hwChannel, uint8_t dutyPercent);

/**
 * @brief Enable PWM channel output
 *
 * This function enables the output of a specific PWM channel.
 *
 * @param instanceIdx - FTM instance index (e.g., INST_FLEXTIMER_PWM_0)
 * @param hwChannel - Hardware channel ID (e.g., PWM_HW_CHANNEL_1)
 */
void ancit_PWM_EnableChannel(uint8_t instanceIdx, uint8_t hwChannel);

/**
 * @brief Disable PWM channel output
 *
 * This function disables the output of a specific PWM channel.
 *
 * @param instanceIdx - FTM instance index (e.g., INST_FLEXTIMER_PWM_0)
 * @param hwChannel - Hardware channel ID (e.g., PWM_HW_CHANNEL_1)
 */
void ancit_PWM_DisableChannel(uint8_t instanceIdx, uint8_t hwChannel);

#endif /* PWM_CONFIGURED */

#endif /* ANCIT_PWM_H */
