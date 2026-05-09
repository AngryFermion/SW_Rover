#include <ancit_do.h>
#include <ancit_driver_digital_io.h>
#include "ancit_common.h"

#ifdef DIGITAL_OUTPUT_CONFIGURED

__attribute__((weak)) extern do_registration_params_t do_reg[];

void ancit_digital_output_write(uint8_t do_idx) {
	//Call the LLD API to write to pin
	ancit_dd_gpio_write(do_reg[do_idx].GPIO_Port, do_reg[do_idx].GPIO_Pin,
			do_reg[do_idx].status);
}

void ancit_digital_output_set(uint8_t do_idx, uint8_t value) {
	uint8_t pin_state = 0;

	if (do_reg[do_idx].active_type == ACTIVE_LOW) {
		pin_state = !value;
	} else {
		pin_state = value;
	}

	// Update the status and Mark the pin as triggered,
	// indicating that its state needs to be updated physically in the hardware
	do_reg[do_idx].status = pin_state;
	ancit_digital_output_write(do_idx);
}

// Toggle the status of the GPIO pin at index do_idx
void ancit_digital_output_toggle(uint8_t do_idx) {
	do_reg[do_idx].status ^= 1; // Toggle status using XOR. This flips 1 to 0 and 0 to 1.
	ancit_digital_output_write(do_idx);
}

void ancit_digital_output_start(void) {
	// Loop through each digital output defined by DIGITAL_OUTPUT_MAX
	// Set the init values with the configured status....
	for (uint8_t idx = 0; idx < gVars.do_max; idx++) {
		do_reg[idx].status = do_reg[idx].initialValue;
		ancit_digital_output_write(idx);
	}
}

// Function: ancit_digital_output_main
void ancit_digital_output_main(void) {
	// Nothing here...
}

#endif //DIGITAL_OUTPUT_CONFIGURED
