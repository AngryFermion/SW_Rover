/*
 * ancit_dd_digital_io.c
 *  Device : S32K144
 *  Created on: 17-Jun-2024
 *      Author: Narayan
 */
#include <ancit_driver_digital_io.h>
#include <ancit_common.h>

uint8_t ancit_dd_gpio_read(GPIO_Type *const base, pins_channel_type_t pin, bool edge_type) {
	//Call the LLD API to read to pin
	uint32_t status = (PINS_DRV_ReadPins(base) & (1U << pin));

	// Adjust the return value based on active_low configuration
	if (edge_type == FALLING_EDGE) {
		return (status == 0) ? 1 : 0;  // FALLING_EDGE: 0 means ON, 1 means OFF
	} else {
		return (status > 0) ? 1 : 0;   // RISING_EDGE: 1 means ON, 0 means OFF
	}
}

void ancit_dd_gpio_write(GPIO_Type *const base, pins_channel_type_t pin,
		pins_level_type_t value) {
	//Call the LLD API to write to pin
	PINS_DRV_WritePin(base, pin, value);
}

