#include <ancit_driver_can.h>
#include "ancit_can.h"
#include "ancit_can_tx.h"
#include "string.h"
#include "ancit_common.h"

#ifdef CAN_TX_CONFIGURED

__attribute__((weak)) extern can_tx_message_registration_params_t tx_reg[];
__attribute__((weak)) extern can_tx_message_t can_tx_messages[];
tx_state_t can_tx_sm_state;

void ancit_can_tx_register_message(uint8_t can_idx, uint32_t msg_id,
		uint8_t enabled, uint8_t msg_type, uint16_t set_interval_ms,
		setup_can_tx_message_t tx_setup_callback) {
	//pointer to the specific can_tx_messages entry
	can_tx_message_t *msg = &can_tx_messages[can_idx];

	msg->msg_id = msg_id;
	msg->enabled = enabled;
	msg->triggered = false;        // Do not trigger on init
	msg->msg_type = msg_type;
	msg->set_interval_ms = set_interval_ms;
	msg->interval_counter = set_interval_ms; // Set interval_counter to initial value

	msg->dataInfo.msg_id_type = FLEXCAN_MSG_ID_STD;
	msg->dataInfo.enable_brs = false;
	msg->dataInfo.fd_enable = false;
	msg->dataInfo.fd_padding = false;

	msg->setupMessage = tx_setup_callback;
}

void ancit_can_tx_init(void) {
	// Register all the messages for Tx
	for (size_t idx = 0; idx < gVars.can_tx_max; idx++) {
		// Create a pointer to the current tx_reg entry
		const can_tx_message_registration_params_t *reg = &tx_reg[idx];

		// Use the pointer to pass arguments to ancit_can_tx_register_message
		ancit_can_tx_register_message(idx, reg->msg_id, reg->enabled,
				reg->msg_type, reg->set_interval_ms, reg->setupMessage);
	}

	// Initialize the Tx state machine to idle
	can_tx_sm_state = ANCIT_CAN_TX_IDLE;
}

void ancit_can_tx_sm(void) {
	static can_tx_message_t tx_msg;

	switch (can_tx_sm_state) {
	case ANCIT_CAN_TX_IDLE:
		for (uint8_t idx = 0; idx < gVars.can_tx_max; idx++) {
			can_tx_message_t *msg = &can_tx_messages[idx];

			// If the message is triggered
			if (msg->triggered) {
				// Reset the triggered flag
				msg->triggered = false;

				// Call the respective API to setup the message
				msg->setupMessage(idx);

				// Copy the entire structure to tx_msg
				memcpy(&tx_msg, msg, sizeof(can_tx_message_t));

				// Change the state to send the message
				can_tx_sm_state = ANCIT_CAN_TX_SEND;
				break;
			}
		}
		break;

	case ANCIT_CAN_TX_SEND:
		/* Send the information via CAN */
		ancit_driver_can_send(INST_FLEXCAN_CONFIG_1, TX_MAILBOX,
				&tx_msg.dataInfo, tx_msg.msg_id, tx_msg.tx_data);

		can_tx_sm_state = ANCIT_CAN_TX_WAIT_FOR_COMPLETE;
		break;

	case ANCIT_CAN_TX_WAIT_FOR_COMPLETE:
		// Wait for message
		if (FLEXCAN_DRV_GetTransferStatus(INST_FLEXCAN_CONFIG_1, TX_MAILBOX)
				== STATUS_BUSY) {
			//wait till busy
		} else {
			can_tx_sm_state = ANCIT_CAN_TX_COMPLETE;
		}
		break;

	case ANCIT_CAN_TX_COMPLETE:
		can_tx_sm_state = ANCIT_CAN_TX_DONE;
		break;

	case ANCIT_CAN_TX_DONE:
		// Finalize and clean up
		can_tx_sm_state = ANCIT_CAN_TX_IDLE;
		break;
	}
}

void ancit_can_tx_main(void) {
	ancit_can_tx_sm();
}

void ancit_can_tx_onstart(void) {
	// Loop through all CAN Tx messages
	for (uint8_t idx = 0; idx < gVars.can_tx_max; idx++) {
		// Create a pointer to the current message
		can_tx_message_t *msg = &can_tx_messages[idx];

		// If the message is enabled and it's of ON_START type
		if (msg->enabled && (msg->msg_type == CAN_TX_MSG_TYPE_ON_START)) {
			// Mark it triggered
			msg->triggered = true;
		}
	}
}

void ancit_can_tx_one_ms(void) {
	// Loop through all Tx messages
	for (uint8_t i = 0; i < gVars.can_tx_max; i++) {
		// Create a pointer to the current CAN Tx message
		can_tx_message_t *msg = &can_tx_messages[i];

		// If the message is enabled and cyclic
		if (msg->enabled && (msg->msg_type == CAN_TX_MSG_TYPE_CYCLIC)) {
			// Decrement the tx interval counter
			msg->interval_counter--;

			// If the counter reaches zero
			if (msg->interval_counter == 0) {
				// Reset the interval counter with the set value
				msg->interval_counter = msg->set_interval_ms;

				// Indicate the message is triggered for transmission
				msg->triggered = true;
			}
		}
	}
}

#endif //CAN_TX_CONFIGURED
