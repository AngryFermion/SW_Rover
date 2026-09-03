/*
 * ancit_driver_digital_io.h
 *  Device : S32K144
 *  Created on: 17-Jun-2024
 *      Author: Narayan
 */
#ifndef ANCIT_DRIVER_DIGITAL_IO_H_
#define ANCIT_DRIVER_DIGITAL_IO_H_
#include "sdk_project_config.h"

uint8_t ancit_dd_gpio_read(GPIO_Type *const base, pins_channel_type_t pin, bool edge_type);
void ancit_dd_gpio_write(GPIO_Type *const base, pins_channel_type_t pin,
		pins_level_type_t value);

#endif /* ANCIT_DRIVER_DIGITAL_IO_H_ */
