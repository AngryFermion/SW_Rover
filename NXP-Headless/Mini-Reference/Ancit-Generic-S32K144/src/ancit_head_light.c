/*
 * ancit_orvm.c
 *
 * Copyright (c) 2024-2025 ANCIT Consulting Pvt Ltd
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.

 * Created on: 25-03-2025
 *     Author: Narayan
 *
 */

#include <ancit_head_light.h>
#include "ancit_common.h"
#include "ancit_timer.h"
#include "ancit_driver_digital_io.h"
#include "genx_di.h"
#include "genx_adc.h"
#include "ancit_head_light.h"

#ifdef HEAD_LIGHT_CONFIGURED

volatile headlightSwitches_t headlightSwitches;
volatile finalLightStatus_t finalLightStatus_left;
volatile finalLightStatus_t finalLightStatus_right;



volatile finalLightStatus_t finalLightStatus_left_sdv;
volatile finalLightStatus_t finalLightStatus_right_sdv;

volatile HeadLightFeatureStatus_t HeadLightFeatureStatus;

volatile DigitalInputStatus_t di_status;

int count = 0;
uint8_t button_status = 0;

const uint16_t pwm_duty_lookup[LEVELLER_POS_MAX] = { //
		169,  // 1.8V
				338,  // 3.6V
				507,  // 5.4V
				676,  // 7.2V
				837   // 8.9V
		};

// This function reads the debounced status of all digital input channels.
// It retrieves the debounced status of each channel using the respective getter functions.
void ancit_hl_ReadAllDebouncedStatus(void) {
	di_status.ch1 = DI_PD_CH1_get_debounced_status();
	di_status.ch2 = DI_PD_CH2_get_debounced_status();
	di_status.ch3 = DI_PD_CH3_get_debounced_status();
	di_status.ch4 = DI_PD_CH4_get_debounced_status();
	di_status.ch5 = DI_PD_CH5_get_debounced_status();
	di_status.ch6 = DI_PD_CH6_get_debounced_status();
}

void ancit_hl_ProcessIndicatorSwitch(bool pin1, bool pin2) {
	IndicatorSwitch_t newIndicatorState = INDICATOR_SWITCH_NONE;

	if (pin1 && pin2) {
		newIndicatorState = INDICATOR_SWITCH_HAZZARD;
	} else if (pin1) {
		newIndicatorState = INDICATOR_SWITCH_LEFT;
	} else if (pin2) {
		newIndicatorState = INDICATOR_SWITCH_RIGHT;
	}

	headlightSwitches.indicatorSwitch = newIndicatorState;
}

void ancit_hl_ProcessHeadLightState(bool pin3, bool pin5, bool pin6) {
	LightStateSwitch_t newLightState = LIGHT_SWITCH_OFF;

	if (pin6) {
		newLightState = LIGHT_SWITCH_HIGH_BEAM;
	} else if (pin5) {
		newLightState = LIGHT_SWITCH_LOW_BEAM;
	} else if (pin3) {
		newLightState = LIGHT_SWITCH_PARKING;
	}

	headlightSwitches.lightSwitch = newLightState;
}

void ancit_hl_ProcessRotarySwitchPosition(uint16_t adc_counts) {
	uint32_t mv = (adc_counts * 3300) / 1000; // Convert ADC counts to millivolts

	LevellerPosition_t newPosition = LEVELLER_POS_4;

	if (mv > 8600) {
		newPosition = LEVELLER_POS_0;
	} else if (mv > 6900) {
		newPosition = LEVELLER_POS_1;
	} else if (mv > 5100) {
		newPosition = LEVELLER_POS_2;
	} else if (mv > 3300) {
		newPosition = LEVELLER_POS_3;
	}

	headlightSwitches.rotaryLevellerSwitch = newPosition;
}

void ancit_hl_UpdateHeadLightStatus(void) {
	HeadLightBeam_t beam = HEAD_LIGHT_BEAM_OFF;
	LightStatus_t parking = LIGHT_OFF;

	switch (headlightSwitches.lightSwitch) {
	case LIGHT_SWITCH_PARKING:
		parking = LIGHT_ON;
		break;

	case LIGHT_SWITCH_LOW_BEAM:
		beam = HEAD_LIGHT_BEAM_LOW;
		break;

	case LIGHT_SWITCH_HIGH_BEAM:
		beam = HEAD_LIGHT_BEAM_HIGH;
		break;

		// LIGHT_SWITCH_OFF and unknown values are already covered by initial values
	default:
		break;
	}

	finalLightStatus_left.head_light_beam = beam;
	finalLightStatus_right.head_light_beam = beam;

	finalLightStatus_left.parking_light = parking;
	finalLightStatus_right.parking_light = parking;
}

void ancit_hl_UpdateIndicatorEnabled(void) {
	switch (headlightSwitches.indicatorSwitch) {
	case INDICATOR_SWITCH_LEFT:
		finalLightStatus_left.indicator_enabled = LIGHT_ON;
		finalLightStatus_right.indicator_enabled = LIGHT_OFF;
		break;
	case INDICATOR_SWITCH_RIGHT:
		finalLightStatus_left.indicator_enabled = LIGHT_OFF;
		finalLightStatus_right.indicator_enabled = LIGHT_ON;
		break;

	case INDICATOR_SWITCH_HAZZARD:
		finalLightStatus_left.indicator_enabled = LIGHT_ON;
		finalLightStatus_right.indicator_enabled = LIGHT_ON;
		break;

	case INDICATOR_SWITCH_NONE:
	default:
		finalLightStatus_left.indicator_enabled = LIGHT_OFF;
		finalLightStatus_right.indicator_enabled = LIGHT_OFF;
		break;
	}
}

static void ancit_hl_ToggleIndicatorLight(volatile finalLightStatus_t *status) {
//	status->indicator_light = status->indicator_enabled;

#if 1
    if (status->indicator_enabled == LIGHT_ON)
    {
        if (status->indicator_light == LIGHT_ON)
        {
            status->indicator_light = LIGHT_OFF;
        }
        else
        {
            status->indicator_light = LIGHT_ON;
        }
    }
    else
    {
        status->indicator_light = LIGHT_OFF;
    }
#endif
}

void ancit_hl_ToggleIndicators(void) {
	// Called every 500ms to toggle
	ancit_hl_ToggleIndicatorLight(&finalLightStatus_left);
	ancit_hl_ToggleIndicatorLight(&finalLightStatus_right);
	ancit_hl_ToggleIndicatorLight(&finalLightStatus_right_sdv);
	ancit_hl_ToggleIndicatorLight(&finalLightStatus_left_sdv);
}

void ancit_hl_UpdateHeadLightLevelStatus(void) {
	finalLightStatus_left.head_light_level =
			headlightSwitches.rotaryLevellerSwitch;
	finalLightStatus_right.head_light_level =
			headlightSwitches.rotaryLevellerSwitch;
}

ftm_state_t ftmStateStruct;

void ancit_hl_pwm_update_duty(uint16_t dutyCycle) {
	FTM_DRV_UpdatePwmChannel(INST_FLEXTIMER_PWM_1,
			flexTimer_pwm_1_IndependentChannelsConfig[0].hwChannelId,
			FTM_PWM_UPDATE_IN_TICKS, (uint16_t) dutyCycle, 0, // secondEdge not used in edge-aligned PWM
			true);
}

void ancit_tunnel_crossed(void){
	HeadLightFeatureStatus.head_light_feature_high_beam = HEAD_LIGHT_FEATURE_ENABLED;
	HeadLightFeatureStatus.head_light_feature_low_beam = HEAD_LIGHT_FEATURE_ENABLED;
}
void ancit_tunnel_started(void){
	HeadLightFeatureStatus.head_light_feature_high_beam = HEAD_LIGHT_FEATURE_DISABLED;
	HeadLightFeatureStatus.head_light_feature_low_beam = HEAD_LIGHT_FEATURE_ENABLED;
}


void ancit_hl_pwm_start(void) {

#ifdef AUTO_DIP_FEATURE_ENABLED

	HeadLightFeatureStatus.head_light_feature_high_beam = HEAD_LIGHT_FEATURE_ENABLED;
	HeadLightFeatureStatus.head_light_feature_low_beam = HEAD_LIGHT_FEATURE_ENABLED;

#endif

	/* Initialize FTM instance */


	FTM_DRV_Init(INST_FLEXTIMER_PWM_1, &flexTimer_pwm_1_InitConfig,
			&ftmStateStruct);

	/* Initialize FTM PWM */
	FTM_DRV_InitPwm(INST_FLEXTIMER_PWM_1, &flexTimer_pwm_1_PwmConfig);

}

#if 0
void ancit_pwm_step_level(void) {
	static uint8_t level = 0;

	ancit_hl_pwm_update_duty(pwm_duty_lookup[level]);

	level++;
	if (level >= LEVELLER_POS_MAX) {
		level = 0;
	}
}
#endif

void ancit_hl_UpdatePwmFromLeveller(void)
{
    static LevellerPosition_t prevLevel = LEVELLER_POS_0;
    LevellerPosition_t currentLevel = finalLightStatus_right.head_light_level;

    if (currentLevel != prevLevel && currentLevel < LEVELLER_POS_MAX)
    {
        ancit_hl_pwm_update_duty(pwm_duty_lookup[currentLevel]);
        prevLevel = currentLevel;
    }
}

void ancit_hl_SetHighBeamState(uint8_t state)
{
    PINS_DRV_WritePin(DO_HS_PWR_CH1_PORT, DO_HS_PWR_CH1_PIN, state);
}

void ancit_hl_SetLowBeamState(uint8_t state)
{
    PINS_DRV_WritePin(DO_HS_PWR_CH2_PORT, DO_HS_PWR_CH2_PIN, state);
}

void ancit_hl_SetIndicatorState(uint8_t state)
{
    PINS_DRV_WritePin(DO_HS_PWR_CH3_PORT, DO_HS_PWR_CH3_PIN, state);
}

void ancit_hl_SetParkingLightState(uint8_t state)
{
    PINS_DRV_WritePin(DO_HS_PWR_CH4_PORT, DO_HS_PWR_CH4_PIN, state);
}

void ancit_hl_ApplyRightLightStatus(void)
{


#ifdef AUTO_DIP_FEATURE_ENABLED
    if (finalLightStatus_right.head_light_beam == HEAD_LIGHT_BEAM_HIGH && HeadLightFeatureStatus.head_light_feature_high_beam == HEAD_LIGHT_FEATURE_ENABLED)
	{
		ancit_hl_SetHighBeamState(1);
		ancit_hl_SetLowBeamState(0);
	}
	else if (finalLightStatus_right.head_light_beam == HEAD_LIGHT_BEAM_HIGH && HeadLightFeatureStatus.head_light_feature_high_beam == HEAD_LIGHT_FEATURE_DISABLED && finalLightStatus_right.head_light_beam != HEAD_LIGHT_BEAM_LOW)
	{
		ancit_hl_SetHighBeamState(0);
		ancit_hl_SetLowBeamState(1);
	}
	else if (finalLightStatus_right.head_light_beam == HEAD_LIGHT_BEAM_LOW)
	{
		ancit_hl_SetHighBeamState(0);
		ancit_hl_SetLowBeamState(1);
	}

	else
    {
/*    	if(finalLightStatus_right.head_light_beam == HEAD_LIGHT_BEAM_HIGH){
    		ancit_hl_SetHighBeamState(0);
			ancit_hl_SetLowBeamState(1);
    	}
    	else{

    		ancit_hl_SetHighBeamState(0);
			ancit_hl_SetLowBeamState(0);
    	}*/

	if (finalLightStatus_right_sdv.head_light_beam == HEAD_LIGHT_BEAM_HIGH && HeadLightFeatureStatus.head_light_feature_high_beam == HEAD_LIGHT_FEATURE_ENABLED)
	{
		ancit_hl_SetHighBeamState(1);
		ancit_hl_SetLowBeamState(0);
	}
	else if (finalLightStatus_right_sdv.head_light_beam == HEAD_LIGHT_BEAM_HIGH && HeadLightFeatureStatus.head_light_feature_high_beam == HEAD_LIGHT_FEATURE_DISABLED && finalLightStatus_right_sdv.head_light_beam != HEAD_LIGHT_BEAM_LOW)
	{
		ancit_hl_SetHighBeamState(0);
		ancit_hl_SetLowBeamState(1);
	}
	else if (finalLightStatus_right_sdv.head_light_beam == HEAD_LIGHT_BEAM_LOW)
	{
		ancit_hl_SetHighBeamState(0);
		ancit_hl_SetLowBeamState(1);
	}

	else{

		ancit_hl_SetHighBeamState(0);
		ancit_hl_SetLowBeamState(0);

	}
		

    }
#else

    if (finalLightStatus_right.head_light_beam == HEAD_LIGHT_BEAM_HIGH)
	{
		ancit_hl_SetHighBeamState(1);
		ancit_hl_SetLowBeamState(0);
	}
	else if (finalLightStatus_right.head_light_beam == HEAD_LIGHT_BEAM_LOW)
	{
		ancit_hl_SetHighBeamState(0);
		ancit_hl_SetLowBeamState(1);
	}

	else
    {
/*    	if(finalLightStatus_right.head_light_beam == HEAD_LIGHT_BEAM_HIGH){
    		ancit_hl_SetHighBeamState(0);
			ancit_hl_SetLowBeamState(1);
    	}
    	else{

    		ancit_hl_SetHighBeamState(0);
			ancit_hl_SetLowBeamState(0);
    	}*/

	if (finalLightStatus_right_sdv.head_light_beam == HEAD_LIGHT_BEAM_HIGH)
	{
		ancit_hl_SetHighBeamState(1);
		ancit_hl_SetLowBeamState(0);
	}
	else if (finalLightStatus_right_sdv.head_light_beam == HEAD_LIGHT_BEAM_LOW)
	{
		ancit_hl_SetHighBeamState(0);
		ancit_hl_SetLowBeamState(1);
	}

	else{

		ancit_hl_SetHighBeamState(0);
		ancit_hl_SetLowBeamState(0);

	}
		

    }

#endif
    

	if(finalLightStatus_right.indicator_enabled == LIGHT_OFF){

		ancit_hl_SetIndicatorState(finalLightStatus_right_sdv.indicator_light);

	}

	else if(finalLightStatus_right.indicator_enabled == LIGHT_ON){

		ancit_hl_SetIndicatorState(finalLightStatus_right.indicator_light);

	}

    

	if(finalLightStatus_right.parking_light == LOW){

		ancit_hl_SetParkingLightState(finalLightStatus_right_sdv.parking_light);

	}
	else if(finalLightStatus_right.parking_light == HIGH){

		ancit_hl_SetParkingLightState(finalLightStatus_right.parking_light);
	}
    
	
}

void ancit_hl_pwm_main(void) {
//	static uint16_t dutyCycle = 0;
//	static bool increasing = true;

//	ancit_pwm_step_level();

//	PINS_DRV_WritePin(DO_HS_PWR_CH1_PORT, DO_HS_PWR_CH1_PIN, state1);
//	PINS_DRV_WritePin(DO_HS_PWR_CH2_PORT, DO_HS_PWR_CH2_PIN, state1);
//	PINS_DRV_WritePin(DO_HS_PWR_CH3_PORT, DO_HS_PWR_CH3_PIN, state1);
//	PINS_DRV_WritePin(DO_HS_PWR_CH4_PORT, DO_HS_PWR_CH4_PIN, state1);


	// Update PWM duty cycle
//	ancit_hl_pwm_update_duty(dutyCycle);

//	// Adjust dutyCycle based on direction
//	if (increasing) {
//		dutyCycle += 1;
//		if (dutyCycle >= 960) {
//			increasing = false;
//		}
//	}
//	else {
//		dutyCycle -= 1;
//		if (dutyCycle == 0) {
//			increasing = true;
//		}
//	}
}

void ancit_hl_Init(void) {
	headlightSwitches.indicatorSwitch = INDICATOR_SWITCH_NONE;
	headlightSwitches.lightSwitch = LIGHT_SWITCH_OFF;
	headlightSwitches.rotaryLevellerSwitch = LEVELLER_POS_0;
}

// This function updates the debounced status of all channels and determines the current state of the indicator and light.
// It reads the debounced status of the digital inputs and uses the helper functions to determine the
// current state of the indicator and light based on the status of the relevant channels.
// It is typically called periodically to ensure that the system's state is up-to-date.
void ancit_hl_ReadAllSensors(void) {
	ancit_hl_ReadAllDebouncedStatus();

	ancit_hl_ProcessIndicatorSwitch(di_status.ch1, di_status.ch2);
	ancit_hl_ProcessHeadLightState(di_status.ch3, di_status.ch5, di_status.ch6);
	ancit_hl_ProcessRotarySwitchPosition(ADC_ANG_CH1_get_filtered_val());

	ancit_hl_UpdateHeadLightStatus();
	ancit_hl_UpdateHeadLightLevelStatus();

	ancit_hl_UpdateIndicatorEnabled();

	// ancit_hl_ToggleIndicators();
}


void ancit_button_read(void){
	if(di_status.ch4 == 1){
//		if(button_status == 0){
			button_status = 1;

//		}
//		else{
//			button_status = 0;
//		}
	}
	else if(di_status.ch4 == 0){
		button_status = 0;
	}
}

#endif // HEAD_LIGHT_CONFIGURED
