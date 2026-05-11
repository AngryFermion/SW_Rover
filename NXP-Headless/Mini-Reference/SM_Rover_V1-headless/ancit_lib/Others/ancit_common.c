/*
 * ancit_common.c
 *
 *  Created on: 31-May-2024
 *      Author: Narayan
 */

#include "ancit_common.h"

global_common_vars_t gVars;

int32_t ancit_value_map(int32_t x, int32_t in_min, int32_t in_max,
		int32_t out_min, int32_t out_max) {
	const int32_t dividend = out_max - out_min;
	const int32_t divisor = in_max - in_min;
	const int32_t delta = x - in_min;
	if (divisor == 0) {
		return -1; //AVR returns -1, SAM returns 0
	}
	return (delta * dividend + (divisor / 2)) / divisor + out_min;
}

// Initialize the moving average filter
void ancit_init_moving_average_filter(filter_master_struct_t *filter,
		uint8_t windowSize) {
	filter->windowSize = windowSize;
	filter->index = 0;
	filter->sum = 0;
	filter->count = 0;
	for (size_t i = 0; i < MAX_WINDOW_SIZE; i++) {
		filter->buffer[i] = 0;
	}
}

// Update the moving average filter with a new value and get the filtered result
uint16_t ancit_update_moving_average_filter(filter_master_struct_t *filter,
		uint16_t newValue) {
	// Remove the oldest value from the sum
	filter->sum -= filter->buffer[filter->index];

	// Add the new value to the buffer and the sum
	filter->buffer[filter->index] = newValue;
	filter->sum += newValue;

	// Update the index
	filter->index = (filter->index + 1) % filter->windowSize;

	// Update the count (up to the window size)
	if (filter->count < filter->windowSize) {
		filter->count++;
	}

	// Calculate the new filtered value
	uint16_t newFilteredValue = (uint16_t) (filter->sum / filter->count);

	// Return the new filtered value
	return newFilteredValue;
}

// Initialize the median filter
void ancit_init_median_filter(filter_master_struct_t *filter, size_t windowSize) {
	filter->windowSize = windowSize;
	filter->count = 0;
	for (size_t i = 0; i < MAX_WINDOW_SIZE; i++) {
		filter->buffer[i] = 0;
	}
}

// Helper function to sort an array
void ancit_sort_array(uint16_t *arr, size_t size) {
	for (size_t i = 0; i < size - 1; i++) {
		for (size_t j = 0; j < size - i - 1; j++) {
			if (arr[j] > arr[j + 1]) {
				uint16_t temp = arr[j];
				arr[j] = arr[j + 1];
				arr[j + 1] = temp;
			}
		}
	}
}

// Update the median filter with a new value and get the filtered result
uint16_t ancit_update_median_filter(filter_master_struct_t *filter,
		uint16_t newValue) {
	// Shift the buffer and insert the new value at the end
	for (size_t i = filter->windowSize - 1; i > 0; i--) {
		filter->buffer[i] = filter->buffer[i - 1];
	}
	filter->buffer[0] = newValue;

	// Update the count (up to the window size)
	if (filter->count < filter->windowSize) {
		filter->count++;
	}

	// Copy the buffer to a temporary array for sorting
	uint16_t tempBuffer[MAX_WINDOW_SIZE];
	for (size_t i = 0; i < filter->count; i++) {
		tempBuffer[i] = filter->buffer[i];
	}

	// Sort the temporary array
	ancit_sort_array(tempBuffer, filter->count);

	// Return the median value
	if (filter->count % 2 == 0) {
		return (tempBuffer[filter->count / 2 - 1]
				+ tempBuffer[filter->count / 2]) / 2;
	} else {
		return tempBuffer[filter->count / 2];
	}
}

// Initialize the IIR filter
void ancit_init_IIR_filter(filter_master_struct_t *filter,
		uint16_t alphaPercentage) {
	// Convert alpha percentage to a value between 0 and 1
	filter->alpha = alphaPercentage / 100.0f;
	filter->lastOutput = 0;
	filter->initialized = 0;  // Not initialized yet
}

// Update the IIR filter with a new value and get the filtered result
uint16_t ancit_update_IIR_filter(filter_master_struct_t *filter,
		uint16_t newValue) {
	if (!filter->initialized) {
		// First input value, initialize the filter output
		filter->lastOutput = newValue;
		filter->initialized = 1;
	} else {
		// Apply the IIR filter equation
		filter->lastOutput = filter->alpha * newValue
				+ (1 - filter->alpha) * filter->lastOutput;
	}
	return filter->lastOutput;
}

// Initialize the one count filter
void ancit_init_OneCount_filter(OneCountFilter_struct_t *filter) {
	filter->lastValue = 0.0f;
	filter->initialized = 0;  // Not initialized yet
}

// Custom rounding function
int32_t ancit_custom_round(float value) {
	if (value >= 0.0f) {
		return (int32_t) (value + 0.5f);
	} else {
		return (int32_t) (value - 0.5f);
	}
}

// Update the one count filter with a new value and get the filtered result
float ancit_update_OneCount_filter(OneCountFilter_struct_t *filter,
		int32_t newValue) {
	if (!filter->initialized) {
		// First input value, initialize the filter output
		filter->lastValue = (float) newValue;
		filter->initialized = 1;
	} else {
		// Calculate the difference
		float diff = (float) newValue - filter->lastValue;

		// Apply smoothing only if the difference is between 1 and 5 counts
		if (diff >= 1.0f && diff <= 5.0f) {
			filter->lastValue += 0.5f * diff;  // Smooth increment/decrement
		} else if (diff <= -1.0f && diff >= -5.0f) {
			filter->lastValue += 0.5f * diff;  // Smooth increment/decrement
		} else if (diff > 5.0f || diff < -5.0f) {
			// Directly update if the difference is greater than 5 counts
			filter->lastValue = (float) newValue;
		}
	}
	// Return the last filtered value
	return (int32_t) ancit_custom_round(filter->lastValue);
}

uint16_t byteSwap16(uint16_t value) {
    return (value >> 8) | (value << 8);
}

// Function to swap the byte order of 32-bit data
uint32_t byteSwap32(uint32_t value) {
    return ((value >> 24) & 0x000000FF) |
           ((value >>  8) & 0x0000FF00) |
           ((value <<  8) & 0x00FF0000) |
           ((value << 24) & 0xFF000000);
}
