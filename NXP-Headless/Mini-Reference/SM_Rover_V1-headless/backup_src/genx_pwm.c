/*
 * genx_PWM.c
 *
 * FlexTimer PWM ANCIT Genx generated files
 *
 * This module provides a flexible PWM initialization system that bridges
 * S32 Design Studio auto-generated configurations with user-defined channel
 * configurations.
 *
 * Copyright (c) 2025 ANCIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <genx_pwm.h>

#ifdef PWM_CONFIGURED
/*******************************************************************************
 * Variables
 ******************************************************************************/

/* FTM state structures for each instance */
ftm_state_t genx_pwm_0_ftmStateStruct;

/* PWM instance configurations with integrated channel configurations */
PWM_registration_params_t genx_pwm_reg[TOTAL_PWM_INSTANCES] = {
	{
		.instanceIdx = INST_FLEXTIMER_PWM_0,            //From Genx_tool user input
		.numChannels = 4,                               //Genx_tool generated, Number of channels configured
		.pInitConfig_s32ds = &flexTimer_pwm_0_InitConfig,   //S32DS auto-generated
		.pPwmConfig_s32ds = &flexTimer_pwm_0_PwmConfig,     //S32DS auto-generated
		.pStateStruct_genx = &genx_pwm_0_ftmStateStruct,    //Genx_tool generated, Pointer to state structure
        .uFrequencyHZ = 1000,

		/* Channel configurations */
		.channels = {
			// Channel 2: LFM
			{
				.hwChannelId = PWM_HW_CHANNEL_0,
				.polarity = FTM_POLARITY_LOW,
				.uDutyCyclePercent = 0,
				.enableExternalTrigger = false,
				.safeState = FTM_HIGH_STATE,
				.enableSecondChannelOutput = false,
				.secondChannelPolarity = FTM_MAIN_INVERTED,
				.deadTime = false
			},
// Channel 2: LBM
			{
				.hwChannelId = PWM_HW_CHANNEL_1,
				.polarity = FTM_POLARITY_LOW,
				.uDutyCyclePercent = 0,
				.enableExternalTrigger = false,
				.safeState = FTM_HIGH_STATE,
				.enableSecondChannelOutput = false,
				.secondChannelPolarity = FTM_MAIN_INVERTED,
				.deadTime = false
			},
// Channel 2: RFM
			{
				.hwChannelId = PWM_HW_CHANNEL_4,
				.polarity = FTM_POLARITY_LOW,
				.uDutyCyclePercent = 0,
				.enableExternalTrigger = false,
				.safeState = FTM_HIGH_STATE,
				.enableSecondChannelOutput = false,
				.secondChannelPolarity = FTM_MAIN_INVERTED,
				.deadTime = false
			},
// Channel 2: RBM
			{
				.hwChannelId = PWM_HW_CHANNEL_5,
				.polarity = FTM_POLARITY_LOW,
				.uDutyCyclePercent = 0,
				.enableExternalTrigger = false,
				.safeState = FTM_HIGH_STATE,
				.enableSecondChannelOutput = false,
				.secondChannelPolarity = FTM_MAIN_INVERTED,
				.deadTime = false
			},

		}
	}
};

void genx_PWM_LFM_EnableOutput(void){
ancit_PWM_EnableChannel(INST_FLEXTIMER_PWM_0, PWM_HW_CHANNEL_0);
}

void genx_PWM_LFM_DisableOutput(void){
ancit_PWM_DisableChannel(INST_FLEXTIMER_PWM_0, PWM_HW_CHANNEL_0);
}

void genx_PWM_LFM_updateDutyCycle(uint8_t dutyPercent){
ancit_PWM_UpdateDutyCycle(INST_FLEXTIMER_PWM_0, PWM_HW_CHANNEL_0, dutyPercent);
}

void genx_PWM_LBM_EnableOutput(void){
ancit_PWM_EnableChannel(INST_FLEXTIMER_PWM_0, PWM_HW_CHANNEL_1);
}

void genx_PWM_LBM_DisableOutput(void){
ancit_PWM_DisableChannel(INST_FLEXTIMER_PWM_0, PWM_HW_CHANNEL_1);
}

void genx_PWM_LBM_updateDutyCycle(uint8_t dutyPercent){
ancit_PWM_UpdateDutyCycle(INST_FLEXTIMER_PWM_0, PWM_HW_CHANNEL_1, dutyPercent);
}

void genx_PWM_RFM_EnableOutput(void){
ancit_PWM_EnableChannel(INST_FLEXTIMER_PWM_0, PWM_HW_CHANNEL_4);
}

void genx_PWM_RFM_DisableOutput(void){
ancit_PWM_DisableChannel(INST_FLEXTIMER_PWM_0, PWM_HW_CHANNEL_4);
}

void genx_PWM_RFM_updateDutyCycle(uint8_t dutyPercent){
ancit_PWM_UpdateDutyCycle(INST_FLEXTIMER_PWM_0, PWM_HW_CHANNEL_4, dutyPercent);
}

void genx_PWM_RBM_EnableOutput(void){
ancit_PWM_EnableChannel(INST_FLEXTIMER_PWM_0, PWM_HW_CHANNEL_5);
}

void genx_PWM_RBM_DisableOutput(void){
ancit_PWM_DisableChannel(INST_FLEXTIMER_PWM_0, PWM_HW_CHANNEL_5);
}

void genx_PWM_RBM_updateDutyCycle(uint8_t dutyPercent){
ancit_PWM_UpdateDutyCycle(INST_FLEXTIMER_PWM_0, PWM_HW_CHANNEL_5, dutyPercent);
}



void genx_PWM_Init(void) {
	genx_PWM_LFM_updateDutyCycle(0);
genx_PWM_LFM_EnableOutput();
genx_PWM_LBM_updateDutyCycle(0);
genx_PWM_LBM_EnableOutput();
genx_PWM_RFM_updateDutyCycle(0);
genx_PWM_RFM_EnableOutput();
genx_PWM_RBM_updateDutyCycle(0);
genx_PWM_RBM_EnableOutput();


	//Call ANCIT PWM Init, which initializes FlexTimer PWM module
    ancit_PWM_Init();
}

#endif /* PWM_CONFIGURED */
