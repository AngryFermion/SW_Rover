#include <ancit_driver_adc.h>
#include "ancit_adc.h"

#ifdef ADC_CONFIGURED

extern global_common_vars_t gVars;
__attribute__((weak)) extern adc_registration_t adc_reg[];	//Contains ADC configuration
__attribute__((weak)) extern adc_values_struct_t adc_values[];	//Contains ADC values

static uint8_t adc_state = ADC_SM_START_IDLE; // Variable to hold the current state of the ADC state machine

/**
 * @brief Starts the ADC and configures the channels.
 *
 * This function initializes the ADC, calibrates it, and configures each
 * channel with the appropriate filter.
 *
 * @param none
 * @return void
 */
void ancit_adc_start(void) {
	//Call the specific device driver
	ancit_driver_adc_init(ADC_INSTANCE_0, &ADC_0_ConvConfig0);
	ancit_driver_adc_calibrate(ADC_INSTANCE_0);

	ancit_driver_adc_init(ADC_INSTANCE_1, &ADC_1_ConvConfig0);
	ancit_driver_adc_calibrate(ADC_INSTANCE_1);

	// Loop through each ADC input channel and initialize the respective filters
	for (uint8_t i = 0; i < gVars.ai_max; i++) {
		switch (adc_reg[i].filter_type) {
		case ADC_FILTER_TYPE_MOV_AVERAGE:
			// Initialize moving average filter
			ancit_init_moving_average_filter(&adc_values[i].filter_master,
					adc_reg[i].filter_setting1);
			break;
		case ADC_FILTER_TYPE_MEDIAN:
			// Initialize median filter
			ancit_init_median_filter(&adc_values[i].filter_master,
					adc_reg[i].filter_setting1);
			break;
		case ADC_FILTER_TYPE_IIR:
			// Initialize IIR filter
			ancit_init_IIR_filter(&adc_values[i].filter_master,
					adc_reg[i].filter_setting1);
			break;
		default:
			break;
		}
		// Initialize one-count filter, used in the final value
		ancit_init_OneCount_filter(&adc_values[i].one_count_filter);
	}

}

/**
 * @brief Retrieves the index of the triggered ADC channel.
 *
 * This function cycles through the ADC channels and returns the index of the first triggered channel.
 *
 * @return uint8_t The index of the triggered ADC channel, or 255 if no channel is triggered.
 *
 * Note: DO NOT CALL THIS FUNCTION FROM ANY OTHER PLACE OTHER THAN STATE MACHINE
 *
 */
uint8_t get_triggered_index(void) {
	static uint8_t last_index = 255; // Static variable to keep track of the last index
	uint8_t start_index = (last_index + 1) % gVars.ai_max; // Start from the next index

	// Loop through each ADC input channel starting from the next index
	for (int i = 0; i < gVars.ai_max; i++) {
		uint8_t current_index = (start_index + i) % gVars.ai_max; // Wrap around using modulo
		if (adc_reg[current_index].triggered) {
			last_index = current_index;
			return current_index;	// Return the index of the triggered channel
		}
	}

	return 255;  // Return 255 if no triggered element is found
}

/**
 * @brief Processes the ADC data.
 *
 * This function applies the appropriate filters, maps the filtered value,
 * and updates the stable value.
 *
 * @param adc_idx The index of the ADC channel.
 * @return void
 */
void ancit_adc_post_process(uint8_t adc_idx) {
	//ADC data will be available in adc_values[adc_idx].raw_val
	adc_registration_t *loc_ar = &adc_reg[adc_idx];
	adc_values_struct_t *loc_av = &adc_values[adc_idx];

	//Apply filter on raw value based on registration
	switch (loc_ar->filter_type) {
	case ADC_FILTER_TYPE_MOV_AVERAGE:
		loc_av->filtered_val = ancit_update_moving_average_filter(
				&loc_av->filter_master, loc_av->raw_val);
		break;
	case ADC_FILTER_TYPE_MEDIAN:
		loc_av->filtered_val = ancit_update_median_filter(
				&loc_av->filter_master, loc_av->raw_val);
		break;
	case ADC_FILTER_TYPE_IIR:
		loc_av->filtered_val = ancit_update_IIR_filter(&loc_av->filter_master,
				loc_av->raw_val);
		break;
	default:
		//If not filter selected move raw value to filtered_val directly
		loc_av->filtered_val = loc_av->raw_val;
		break;
	}

	// Map the filtered ADC value to the configured range
	loc_av->mapped_val = ancit_value_map(loc_av->filtered_val,
			loc_ar->map.adc_min, loc_ar->map.adc_max, loc_ar->map.val_min,
			loc_ar->map.val_max);
	// Update the stable value using the one-count filter
	loc_av->stable_val = ancit_update_OneCount_filter(&loc_av->one_count_filter,
			loc_av->mapped_val);
}

/**
 * @brief ADC state machine.
 *
 * This function manages the ADC state machine, handling the start, conversion,
 * and completion states.
 *
 * @param none
 * @return void
 */
void ancit_adc_sm(void) {
	uint16_t adcRawValue;				// Variable to store the raw ADC value
	static uint8_t adc_idx_sm = 0;// Static variable to keep track of the current ADC
								  // index in the state machine

	switch (adc_state) {
	case ADC_SM_START_IDLE:
		// Get the index of the NEXT triggered ADC channel
		adc_idx_sm = get_triggered_index();

		//If proper index is set, start conversion
		if (adc_idx_sm != 255) {
			adc_state = ADC_SM_START_CONVERSION;
		}
		break;

	case ADC_SM_START_CONVERSION:
		/* Configure ADC channel and software trigger a conversion */
		ancit_driver_adc_start_channel_conversion(
				adc_reg[adc_idx_sm].instance_idx, ADC_CHANNEL_IDX,
				&adc_reg[adc_idx_sm].channel_config);

		//Move to wait state
		adc_state = ADC_SM_WAIT_FOR_COMPLETION;
		break;

	case ADC_SM_WAIT_FOR_COMPLETION:
		//Check if conversion is complete, else be in the same state
		if (ancit_driver_adc_conv_complete(
				adc_reg[adc_idx_sm].instance_idx) == true) {
			/* Store the channel result into a local variable */
			ancit_driver_adc_get_result(adc_reg[adc_idx_sm].instance_idx,
					&adcRawValue);
			adc_values[adc_idx_sm].raw_val = adcRawValue;

			// Process the ADC data... like, filter, map etc.,
			ancit_adc_post_process(adc_idx_sm);

			// Call the registered callback function for conversion complete notification
			adc_reg[adc_idx_sm].conv_complete_notfn(adc_idx_sm);

			adc_state = ADC_SM_DONE;
		}
		break;

	case ADC_SM_DONE:
		//If it is a cyclic sample in ms, then disable the trigger
		//In continous sample, trigger will not be reset
		if (adc_reg[adc_idx_sm].set_interval_ms > 0) {
			adc_reg[adc_idx_sm].triggered = false;
		}
		//Go back to idle state
		adc_state = ADC_SM_START_IDLE;
		break;

	default:
		break;
	}
}

/**
 * @brief Main ADC function.
 *
 * This function calls the ADC state machine function.
 *
 * @param none
 * @return void
 */
void ancit_adc_main(void) {
	// Call the ADC state machine function
	ancit_adc_sm();
}

/**
 * @brief ADC periodic function.
 *
 * This function should be called every 1 millisecond. It handles the timing and
 * triggering of ADC channels.
 *
 * @param none
 * @return void
 */
void ancit_adc_one_ms(void) {
	// Loop through all ADC input channels
	for (uint8_t idx = 0; idx < gVars.ai_max; idx++) {
		//check only the indexes where interval ms is set (cyclic, else it will be continuously triggered)
		if (adc_reg[idx].set_interval_ms > 0) {
			// Increment the interval counter
			adc_reg[idx].interval_counter++;

			// If the interval counter reaches the maximum value, trigger the ADC channel
			if (adc_reg[idx].interval_counter >= adc_reg[idx].set_interval_ms) {
				//Reset the interval counter to 0
				adc_reg[idx].interval_counter = 0;

				//set a flag to indicate the message is triggered for transmission
				adc_reg[idx].triggered = true;
			}
		} else {
			//if set_interval_ms = 0, keep triggered as true always
			adc_reg[idx].triggered = true;
		}
	}
}

#endif /* ADC_CONFIGURED */
