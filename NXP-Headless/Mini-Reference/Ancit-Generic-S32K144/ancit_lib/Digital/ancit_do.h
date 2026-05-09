#ifndef _ANCIT_DIGITAL_OUTPUT_H_
#define _ANCIT_DIGITAL_OUTPUT_H_

#include "genx_config.h"
#include "ancit_timer.h"

#ifdef DIGITAL_OUTPUT_CONFIGURED

typedef struct {
	GPIO_Type* GPIO_Port;
	uint8_t GPIO_Pin;
	uint8_t active_type;
	uint8_t triggered;
	uint8_t initialValue;
	uint8_t status;
} do_registration_params_t;

void ancit_digital_output_start(void);
void ancit_digital_output_main(void);
void ancit_digital_output_set(uint8_t do_idx, uint8_t value);
void ancit_digital_output_toggle(uint8_t do_idx);

#endif //DIGITAL_OUTPUT_CONFIGURED
#endif //_ANCIT_DIGITAL_OUTPUT_H_
