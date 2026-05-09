/*
 * ancit_can_tx_generated.h
 *
 *  Created on: 17-May-2024
 *      Author: Narayan
 */

#ifndef ANCIT_CAN_TX_H_
#define ANCIT_CAN_TX_H_
#include "genx_config.h"

#ifdef CAN_TX_CONFIGURED

//CAN Mailbox used for Tx
#define TX_MAILBOX  					(0UL)

#define CAN_TX_MSG_MAX_DLC				8

//CAN Message types (FIXED)
#define CAN_TX_MSG_TYPE_DISABLED		0
#define CAN_TX_MSG_TYPE_ON_START		1
#define CAN_TX_MSG_TYPE_ON_EVENT		2
#define CAN_TX_MSG_TYPE_ON_CAN_MSG		3
#define CAN_TX_MSG_TYPE_CYCLIC			4

// Define the states
typedef enum {
	ANCIT_CAN_TX_IDLE,
	ANCIT_CAN_TX_SEND,
	ANCIT_CAN_TX_WAIT_FOR_COMPLETE,
	ANCIT_CAN_TX_COMPLETE,
	ANCIT_CAN_TX_DONE
} tx_state_t;

//Send Function prototype
typedef void (*setup_can_tx_message_t)(uint8_t idx);

// Define the structure to hold the runtime configuration parameters
typedef struct {
	uint8_t enabled;
	uint8_t msg_type;
	uint32_t msg_id;
	flexcan_data_info_t dataInfo;
	uint8_t dlc;
	uint8_t tx_data[CAN_TX_MSG_MAX_DLC];

	uint16_t set_interval_ms;   			//CAN message set interval
	uint16_t interval_counter;
	uint8_t triggered;
	setup_can_tx_message_t setupMessage;		//Setup message function pointer
} can_tx_message_t;

// Define the structure to hold the parameters for ontime registration
typedef struct {
	uint8_t idx;
	uint32_t msg_id;
	uint8_t dlc;
	bool enabled;
	uint8_t msg_type;
	uint16_t set_interval_ms;
	setup_can_tx_message_t setupMessage;
} can_tx_message_registration_params_t;

extern tx_state_t can_tx_sm_state;

void ancit_can_tx_init(void);
void ancit_can_tx_main(void);
void ancit_can_tx_onstart(void);
void ancit_can_tx_one_ms(void);

#endif //CAN_TX_CONFIGURED
#endif /* ANCIT_CAN_TX_H_ */
