/*
 * ancit_head_light.h
 *
 *  Created on: 30-Jun-2025
 *      Author: Narayan
 */

#ifndef ANCIT_HEAD_LIGHT_H_
#define ANCIT_HEAD_LIGHT_H_
#include "genx_config.h"

#ifdef HEAD_LIGHT_CONFIGURED


#define TUNNEL_DURATION		15

typedef enum {
    LIGHT_OFF = 0,
    LIGHT_ON = 1
} LightStatus_t;

typedef enum {
    INDICATOR_SWITCH_NONE,
    INDICATOR_SWITCH_LEFT,
    INDICATOR_SWITCH_RIGHT,
    INDICATOR_SWITCH_HAZZARD
} IndicatorSwitch_t;

typedef enum {
    LIGHT_SWITCH_OFF,
    LIGHT_SWITCH_PARKING,
    LIGHT_SWITCH_LOW_BEAM,
    LIGHT_SWITCH_HIGH_BEAM
} LightStateSwitch_t;

typedef enum {
    LEVELLER_POS_0 = 0,
    LEVELLER_POS_1 = 1,
    LEVELLER_POS_2 = 2,
    LEVELLER_POS_3 = 3,
    LEVELLER_POS_4 = 4,
	LEVELLER_POS_MAX
} LevellerPosition_t;

typedef enum {
    HEAD_LIGHT_BEAM_OFF = 0,
    HEAD_LIGHT_BEAM_LOW = 1,
    HEAD_LIGHT_BEAM_HIGH = 2
} HeadLightBeam_t;

typedef enum {
	HEAD_LIGHT_FEATURE_ENABLED = true,
	HEAD_LIGHT_FEATURE_DISABLED = false,

}HeadLightFeature_t;

typedef struct {
    uint8_t ch1;
    uint8_t ch2;
    uint8_t ch3;
    uint8_t ch4;
    uint8_t ch5;
    uint8_t ch6;
} DigitalInputStatus_t;

typedef struct {
    IndicatorSwitch_t indicatorSwitch;
    LightStateSwitch_t lightSwitch;
    LevellerPosition_t rotaryLevellerSwitch;
} headlightSwitches_t;

typedef struct {
    HeadLightBeam_t head_light_beam;
    LevellerPosition_t head_light_level;
    LightStatus_t indicator_enabled;
    LightStatus_t indicator_light;
    LightStatus_t parking_light;
} finalLightStatus_t;

typedef struct {
	HeadLightFeature_t head_light_feature_high_beam;
	HeadLightFeature_t head_light_feature_low_beam;
}HeadLightFeatureStatus_t;

extern volatile headlightSwitches_t headlightSwitches;
extern volatile finalLightStatus_t finalLightStatus_left;
extern volatile finalLightStatus_t finalLightStatus_right;
extern volatile DigitalInputStatus_t digitalInput_status;

void ancit_hl_ReadAllDebouncedStatus(void);
LevellerPosition_t ancit_hl_GetRotarySwitchPosition(uint16_t adc_counts);

IndicatorSwitch_t ancit_hl_GetIndicatorState(bool pin1, bool pin2);
LightStateSwitch_t ancit_hl_GetLightState(bool pin3, bool pin5, bool pin6);
void ancit_hl_ToggleIndicators(void);

void ancit_hl_pwm_start(void);
void ancit_hl_pwm_main(void);


void ancit_tunnel_crossed(void);
void ancit_tunnel_started(void);
void ancit_button_read(void);

void ancit_hl_Init(void);
void ancit_hl_ReadAllSensors(void);
void ancit_hl_ApplyRightLightStatus(void);
void ancit_hl_ToggleIndicators(void);
void ancit_hl_UpdatePwmFromLeveller(void);

#endif /* HEAD_LIGHT_CONFIGURED */
#endif /* ANCIT_HEAD_LIGHT_H_ */
