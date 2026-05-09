/*
 * ancit_dd_adc.c
 *  Device : S32K144
 *  Created on: 17-Jun-2024
 *      Author: Narayan
 */
#include "sdk_project_config.h"
#include "ancit_adc.h"

void ancit_driver_adc_init(const uint32_t instance,
		const adc_converter_config_t *const config) {
	// Configure the ADC converter with the specified instance
	ADC_DRV_ConfigConverter(instance, config);
}

void ancit_driver_adc_calibrate(const uint32_t instance) {
	// Perform auto-calibration of the ADC
	ADC_DRV_AutoCalibration(instance);
}

//Conversion happens one channel after the another...
void ancit_driver_adc_start_channel_conversion(const uint32_t instance,
		const uint8_t chanIndex, const adc_chan_config_t *const config) {
	/* Configure ADC channel and software trigger a conversion */
	ADC_DRV_ConfigChan(instance, chanIndex, config);
}

bool ancit_driver_adc_conv_complete(const uint32_t instance) {
	return ADC_DRV_GetConvCompleteFlag(instance, ADC_CHANNEL_IDX);
}

void ancit_driver_adc_get_result(const uint32_t instance, uint16_t *adcRawValue) {
	ADC_DRV_GetChanResult(instance, ADC_CHANNEL_IDX, adcRawValue);
}

