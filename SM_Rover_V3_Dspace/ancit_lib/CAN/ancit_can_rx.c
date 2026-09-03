#include <ancit_driver_can.h>
#include "ancit_can_rx.h"
#include <stdio.h>
#include <string.h>
#include "ancit_common.h"
#include "ancit_can.h"

#ifdef CAN_RX_CONFIGURED

extern g_struct_can_rx_t can_rx[];
extern can_rx_message_registration_params_t rx_reg[];

void ancit_can_rx_register_message(uint8_t can_idx) {
	// Static array to track mailbox indices for each instance (function scope)
	// Start from RX_MAILBOX - 1 to ensure the first mailbox is RX_MAILBOX
	// This allows us to increment it to start from RX_MAILBOX
	static uint8_t instance_mb_counter[CAN_MAX_INSTANCES] =
			{ RX_MAILBOX - 1 };

	// Bounds check for can_idx
	if (can_idx >= gVars.can_rx_max) {
		return;
	}

	// Get the registration parameters from rx_reg array
	const can_rx_message_registration_params_t *reg = &rx_reg[can_idx];

	// Bounds check for instance_id
	if (reg->instance_id >= CAN_MAX_INSTANCES) {
		return;
	}

	// Get pointer to the current CAN RX structure for cleaner code
	g_struct_can_rx_t *rx = &can_rx[can_idx];

	rx->sm_state = ANCIT_CAN_RX_INIT;
	rx->msg.can_id = reg->msg_id;
	rx->msg.msg_id_type = reg->msg_id_type;
	rx->instance_id = reg->instance_id;
	rx->rx_notification = reg->notification;

	// Get and increment mailbox index for this instance (starting from RX_MAILBOX)
	instance_mb_counter[reg->instance_id]++;

	// Store the mailbox index in the structure
	rx->mb_idx = instance_mb_counter[reg->instance_id];

	/* Set information about the data to be received
	 *  - 8 bytes in length
	 *  - Standard message ID
	 *  - Bit rate switch disabled
	 *  - Flexible data rate disabled
	 *  - Use zeros for FD padding
	 */
	flexcan_data_info_t dataInfo = {
			.data_length = 8U, // Narayan - this should come from the registration parameters
			.msg_id_type = rx->msg.msg_id_type, .enable_brs = false,
			.fd_enable = false, .fd_padding = 0U };

	ancit_driver_can_ConfigRxMb(rx->instance_id, rx->mb_idx, &dataInfo,
			rx->msg.can_id);
}

void ancit_can_rx_init(void) {
	// Initialize the entire structure to zero
	memset(&can_rx, 0, sizeof(g_struct_can_rx_t));

	// Register all the messages for Rx...
	for (uint8_t idx = 0; idx < gVars.can_rx_max; idx++) {
		ancit_can_rx_register_message(idx);
	}
}

void ancit_can_rx_sm(uint8_t can_idx) {
	// Get pointer to the current CAN RX structure for cleaner code
	g_struct_can_rx_t *rx = &can_rx[can_idx];

	switch (rx->sm_state) {
	case ANCIT_CAN_RX_INIT:
		/* Start receiving data in calculated mailbox. */
		ancit_driver_can_receive(rx->instance_id, rx->mb_idx, &rx->recvMsg);
		rx->sm_state = ANCIT_CAN_RX_WAIT_FOR_MSG;
		break;

	case ANCIT_CAN_RX_WAIT_FOR_MSG:
		// Wait for message
		if (ancit_driver_can_GetTransferStatus(rx->instance_id, rx->mb_idx)
				== STATUS_BUSY) {
			// wait till busy
		} else {
			rx->sm_state = ANCIT_CAN_RX_PROCESS_MSG;
		}
		break;

	case ANCIT_CAN_RX_PROCESS_MSG:
		// Call the specific function registered as call back in the init
		rx->rx_notification();
		rx->sm_state = ANCIT_CAN_RX_DONE;
		break;

	case ANCIT_CAN_RX_DONE:
		// Finalize and clean up
		// Code to finalize
		rx->sm_state = ANCIT_CAN_RX_INIT;
		break;
	}
}

void ancit_can_rx_main(void) {
	for (uint8_t idx = 0; idx < gVars.can_rx_max; idx++) {
		// Run the specific state machine....
		ancit_can_rx_sm(idx);
	}
}
#endif // CAN_RX_CONFIGURED
