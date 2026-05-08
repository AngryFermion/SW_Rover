#ifndef ANCIT_COMMON_H
#define ANCIT_COMMON_H
#include <stdint.h>
#include <stddef.h>  // For NULL

#define MAX_WINDOW_SIZE 32  // Define the maximum window size

#define	TRUE				1
#define	FALSE   			0

//Pin Active types
#define ACTIVE_LOW			0
#define ACTIVE_HIGH			1

//Pin status
#define INIT_VALUE_LOW		0
#define INIT_VALUE_HIGH		1

//Pin status
#define LOW					0
#define HIGH				1

#define FALLING_EDGE		0
#define RISING_EDGE		1

// Define the moving average filter structure
typedef struct {
	uint16_t buffer[MAX_WINDOW_SIZE];
	size_t windowSize;
	size_t index;
	uint32_t sum;
	size_t count;
	uint16_t lastFilteredValue; // To store the last filtered value
	float alpha;        // Smoothing factor (0 < alpha <= 1)
	float lastOutput;   // Last filtered value
	int initialized;    // Initialization flag
} filter_master_struct_t;

// Define the one count filter structure
typedef struct {
	float lastValue;  // Last filtered value
	int initialized;     // Initialization flag
} OneCountFilter_struct_t;

typedef struct {
	uint8_t di_max;
	uint8_t do_max;
	uint8_t can_tx_max;
	uint8_t can_rx_max;
	uint8_t ai_max;
	uint8_t tasks_max;
} global_common_vars_t;

extern global_common_vars_t gVars;

long ancit_value_map(long x, long in_min, long in_max, long out_min,
		long out_max);

void ancit_init_moving_average_filter(filter_master_struct_t *filter,
		uint8_t windowSize);
uint16_t ancit_update_moving_average_filter(filter_master_struct_t *filter,
		uint16_t newValue);

void ancit_init_median_filter(filter_master_struct_t *filter, size_t windowSize);
uint16_t ancit_update_median_filter(filter_master_struct_t *filter,
		uint16_t newValue);

void ancit_init_IIR_filter(filter_master_struct_t *filter,
		uint16_t alphaPercentage);
uint16_t ancit_update_IIR_filter(filter_master_struct_t *filter,
		uint16_t newValue);

void ancit_init_OneCount_filter(OneCountFilter_struct_t *filter);
float ancit_update_OneCount_filter(OneCountFilter_struct_t *filter,
		int32_t newValue);

uint16_t byteSwap16(uint16_t value);
uint32_t byteSwap32(uint32_t value);

#endif  //ANCIT_COMMON_H
