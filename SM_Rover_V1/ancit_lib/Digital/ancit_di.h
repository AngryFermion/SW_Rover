#ifndef _ANCIT_DIGITAL_INPUT_H_
#define _ANCIT_DIGITAL_INPUT_H_
#include "genx_config.h"
#include "ancit_timer.h"

#ifdef DIGITAL_INPUT_CONFIGURED

//On Edge Detection notification Function prototype
typedef void (*edge_detect_notification_t)(uint8_t di_idx);

/* Enumeration for Switch state machine */
enum digital_input_state_t {
	DI_WAIT_FOR_EDGE,
	DI_WAIT_FOR_DEBOUNCE,
	DI_EDGE_DETECTED_WAIT_TILL_INVERSION,
};

enum digital_input_pull_type_t {
	DI_NONE_HZ,
	DI_PULL_UP,
	DI_PULL_DOWN
};

typedef struct {
	GPIO_Type* GPIO_Port;
	uint8_t GPIO_Pin;
	uint32_t timer;
	uint16_t debounce_ms;
	uint8_t edgeType;
	uint8_t sm_state;
	bool edge_detected;
	edge_detect_notification_t edge_notification;
	uint8_t status;
	uint8_t debounced_status;
	uint8_t ignore_on_boot;
} di_registration_params_t;

void ancit_digital_input_start(void);
void ancit_digital_input_main(void);
uint8_t ancit_digital_input_get_status(uint8_t di_idx);

#endif //DIGITAL_INPUT_CONFIGURED
#endif //_ANCIT_DIGITAL_INPUT_H_
