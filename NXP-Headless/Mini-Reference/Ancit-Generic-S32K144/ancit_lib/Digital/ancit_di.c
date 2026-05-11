#include <ancit_di.h>
#include <ancit_driver_digital_io.h>
#include "ancit_common.h"
#include "ancit_timer.h"

#ifdef DIGITAL_INPUT_CONFIGURED

__attribute__((weak)) extern di_registration_params_t di_reg[];

uint8_t ancit_digital_input_get_status(uint8_t di_idx) {
	return di_reg[di_idx].status;
}

void ancit_digital_input_state_machine(uint8_t di_idx) {
	di_registration_params_t *di_sm = &di_reg[di_idx];

	//Read the pin status to the global variable
	di_sm->status = ancit_dd_gpio_read(di_sm->GPIO_Port, di_sm->GPIO_Pin, di_sm->edgeType);

	switch (di_sm->sm_state) {
	case DI_WAIT_FOR_EDGE:
		//The following check will ensure that the digital_input trigger will
		//not happen on the first run (ie., if digital_input remains triggered on power on)
		if (di_sm->ignore_on_boot == TRUE) {
			if (di_sm->status != di_sm->edgeType) {
				di_sm->ignore_on_boot = FALSE;
			}
		} else if (di_sm->status == di_sm->edgeType) {
			//Check if the current status is equal to configured edge type FALLING(LOW)/RAISING(HIGH)
			//If true, Initialize the debounce timer timer and wait...
			ancit_init_timeout(&di_sm->timer, di_sm->debounce_ms);
			di_sm->sm_state = DI_WAIT_FOR_DEBOUNCE;
		}
		break;

	case DI_WAIT_FOR_DEBOUNCE:
		//If debounce timer expired
		if (is_timeout_expired(&di_sm->timer)) {
			//If the digital_input status still remains in the requested edge type (LOW/HIGH)
			if (di_sm->status == di_sm->edgeType) {
				di_sm->debounced_status = di_sm->edgeType;
				//Edge detection confirmed, set the flag and call the respective notification function...
				di_sm->edge_detected = true;
				di_sm->edge_notification(di_idx);

				di_sm->sm_state = DI_EDGE_DETECTED_WAIT_TILL_INVERSION;	//wait for inversion
			} else {
				di_sm->sm_state = DI_WAIT_FOR_EDGE;	//Go back to wait_for_edge state again...
			}
		}
		break;

	case DI_EDGE_DETECTED_WAIT_TILL_INVERSION:
		//if digital_input edge is released, go back and wait again, else wait here...
		if (di_sm->status != di_sm->edgeType) {
			di_sm->debounced_status = di_sm->status;
			di_sm->sm_state = DI_WAIT_FOR_EDGE;
		}
		break;
	}
}

void ancit_digital_input_start(void) {
	//Pin modes configured in the NXP tool

	//Initialize all digital input state machines to waiting for edge
	for (uint8_t idx = 0; idx < gVars.di_max; idx++) {
		di_reg[idx].sm_state = DI_WAIT_FOR_EDGE;
	}
}

void ancit_digital_input_main(void) {
	//Run the state machine for each digital_input independently...
	//Respective notification function will be called, if edge is detected from the state machine..
	for (uint8_t idx = 0; idx < gVars.di_max; idx++) {
		ancit_digital_input_state_machine(idx);
	}
}

#endif //DIGITAL_INPUT_CONFIGURED
