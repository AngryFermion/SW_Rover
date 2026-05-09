#ifndef ANCIT_ADC_H_
#define ANCIT_ADC_H_
#include "genx_config.h"

#define ADC_INSTANCE_0 		0U				// Define the ADC instance being used
#define ADC_INSTANCE_1 		1U				// Define the ADC instance being used
#define ADC_CHANNEL_IDX 	0U				// Define the ADC channel index

#ifdef ADC_CONFIGURED
#include "ancit_common.h"

//ADC conversion complete notification prototype
typedef void (*adc_conv_complete_notfn_t)(uint8_t adc_idx);

/* Enumeration for ADC state machine */
enum ADC_State {
	ADC_SM_START_IDLE,
	ADC_SM_START_CONVERSION,
	ADC_SM_WAIT_FOR_COMPLETION,
	ADC_SM_DONE
};

/* Enumeration for FILTER TYPES */
enum ADC_Filter {
	ADC_FILTER_TYPE_NONE, ADC_FILTER_TYPE_MOV_AVERAGE, ADC_FILTER_TYPE_MEDIAN, ADC_FILTER_TYPE_IIR
};

typedef struct {
	uint16_t adc_min;
	uint16_t adc_max;
	int32_t val_min;
	int32_t val_max;
} adc_value_map_struct_t;

/****Filter setting table******************************************
 Type						filter_setting1		min    max
 ------------------------------------------------------------------
 ADC_FILTER_TYPE_NONE		NA					0		0
 ADC_FILTER_TYPE_AVERAGE	windowSize			5		32
 ADC_FILTER_TYPE_MEDIAN		windowSize			5		31			//Ideally, choose only odd numbers
 ADC_FILTER_TYPE_IIR		alpha_value			10		80
 *****************************************************************/

typedef struct {
	uint8_t enabled;
	uint16_t set_interval_ms;   			//ADC Sample interval
	uint16_t interval_counter;
	uint8_t triggered;
	uint8_t instance_idx;
	adc_chan_config_t channel_config;
	adc_conv_complete_notfn_t conv_complete_notfn;
	uint8_t filter_type;
	uint8_t filter_setting1;
	bool apply_mapping;
	adc_value_map_struct_t map;
} adc_registration_t;

typedef struct {
	uint16_t raw_val;
	filter_master_struct_t filter_master;
	OneCountFilter_struct_t one_count_filter;
	uint16_t filtered_val;
	int32_t mapped_val;
	int32_t stable_val;
	int32_t user_val;					//can be used by the user for further processing
} adc_values_struct_t;

void ancit_adc_start(void);
void ancit_adc_main(void);
void ancit_adc_one_ms(void);

#endif /* ADC_CONFIGURED */
#endif /* ANCIT_ADC_H_ */
