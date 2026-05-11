/*
 * ancit_driver_adc.h
 *  Device : S32K144
 *  Created on: 17-Jun-2024
 *      Author: Narayan
 */

#ifndef ANCIT_DRIVER_ADC_H_
#define ANCIT_DRIVER_ADC_H_
#include "sdk_project_config.h"

void ancit_driver_adc_init(const uint32_t instance,
		const adc_converter_config_t *const config);

void ancit_driver_adc_calibrate(const uint32_t instance);

void ancit_driver_adc_start_channel_conversion(const uint32_t instance,
		const uint8_t chanIndex, const adc_chan_config_t *const config);

bool ancit_driver_adc_conv_complete(const uint32_t instance);

void ancit_driver_adc_get_result(const uint32_t instance, uint16_t *adcRawValue);

#endif /* ANCIT_DRIVER_ADC_H_ */
