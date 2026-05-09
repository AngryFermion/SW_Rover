#ifndef ANCIT_CAN_RX_GENERATED_H
#define ANCIT_CAN_RX_GENERATED_H
#include "genx_config.h"

#ifdef CAN_RX_CONFIGURED

#define CAN_RX_MSG_MAX_DLC				8
#define RX_MAILBOX  					(1UL)			//RX mailbox start from 1
// Define the states
typedef enum {
	ANCIT_CAN_RX_INIT,
	ANCIT_CAN_RX_WAIT_FOR_MSG,
	ANCIT_CAN_RX_PROCESS_MSG,
	ANCIT_CAN_RX_DONE
} rx_state_t;


typedef struct
{
	uint16_t can_id;
	uint8_t length;
	uint8_t rx_data[CAN_RX_MSG_MAX_DLC];
} can_rx_msg_struct_t;

//Send Function prototype
typedef void (*can_rx_message_notification_t)(void);

// Define the structure to hold the runtime configuration parameters
typedef struct
{
	rx_state_t sm_state;
	can_rx_msg_struct_t msg;
	flexcan_msgbuff_t recvMsg;
	can_rx_message_notification_t rx_notification;
} g_struct_can_rx_t;

// Define the structure to hold the one-time registration parameters
typedef struct {
    uint8_t idx;
    uint32_t msg_id;
    can_rx_message_notification_t notification;
} can_rx_message_registration_params_t;

void ancit_can_rx_init(void);
void ancit_can_rx_main(void);

#endif
#endif //CAN_RX_CONFIGURED
