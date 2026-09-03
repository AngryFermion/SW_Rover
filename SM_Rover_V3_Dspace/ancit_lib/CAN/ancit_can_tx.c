#include <ancit_driver_can.h>
#include "ancit_can.h"
#include "ancit_can_tx.h"
#include "string.h"
#include "ancit_common.h"

#ifdef CAN_TX_CONFIGURED

extern can_tx_message_registration_params_t tx_reg[];
extern can_tx_message_t can_tx_messages[];
tx_state_t can_tx_sm_state[CAN_MAX_INSTANCES];

void ancit_can_tx_register_message(uint8_t can_idx)
{
	// Bounds check for can_idx
	if (can_idx >= gVars.can_tx_max)
	{
		return;
	}

	// Get the registration parameters from tx_reg array
	const can_tx_message_registration_params_t *reg = &tx_reg[can_idx];

	// pointer to the specific can_tx_messages entry
	can_tx_message_t *msg = &can_tx_messages[can_idx];

	// Populate message fields from registration parameters
	msg->msg_id = reg->msg_id;
	msg->instance_id = reg->instance_id;
	msg->enabled = reg->enabled;
	msg->triggered = false; // Do not trigger on init
	msg->msg_type = reg->msg_type;
	msg->set_interval_ms = reg->set_interval_ms;
	msg->interval_counter = reg->set_interval_ms; // Set interval_counter to initial value

	msg->dataInfo.msg_id_type = reg->msg_id_type;
	// These are fixed values for the message
	msg->dataInfo.fd_enable = false;
	msg->dataInfo.enable_brs = false;
	msg->dataInfo.fd_padding = false;

	msg->setupMessage = reg->setupMessage;
}

void ancit_can_tx_init(void)
{
	// Initialize all CAN instance state machines to idle
	for (uint8_t i = 0; i < CAN_MAX_INSTANCES; i++)
	{
		can_tx_sm_state[i] = ANCIT_CAN_TX_IDLE;
	}

	// Register all the messages for Tx
	for (size_t idx = 0; idx < gVars.can_tx_max; idx++)
	{
		// Register the message using only the index
		ancit_can_tx_register_message(idx);
	}
}

void ancit_can_tx_sm(uint8_t instance_id)
{
	static can_tx_message_t local_tx_msg[CAN_MAX_INSTANCES];

	// Bounds check for instance_id
	if (instance_id >= CAN_MAX_INSTANCES)
	{
		return;
	}

	// Create a pointer to the current instance's working copy for cleaner code
	can_tx_message_t *working_copy = &local_tx_msg[instance_id];
	// Create a pointer to the current instance's state for cleaner code
	tx_state_t *current_state = &can_tx_sm_state[instance_id];

	switch (*current_state)
	{
	case ANCIT_CAN_TX_IDLE:
		for (uint8_t idx = 0; idx < gVars.can_tx_max; idx++)
		{
			can_tx_message_t *msg = &can_tx_messages[idx];

			// Check if the message is for this instance and is triggered
			if (msg->instance_id == instance_id && msg->triggered)
			{
				// Reset the triggered flag
				msg->triggered = false;

				// Call the respective API to setup the message
				msg->setupMessage(idx);

				// Copy the entire structure to working copy for this instance
				memcpy(working_copy, msg, sizeof(can_tx_message_t));

				// Change the state to send the message
				*current_state = ANCIT_CAN_TX_SEND;
				break;
			}
		}
		break;

	case ANCIT_CAN_TX_SEND:
		/* Send the information via CAN */
		ancit_driver_can_send(working_copy->instance_id, TX_MAILBOX,
							  &working_copy->dataInfo, working_copy->msg_id, working_copy->tx_data);

		*current_state = ANCIT_CAN_TX_WAIT_FOR_COMPLETE;
		break;

	case ANCIT_CAN_TX_WAIT_FOR_COMPLETE:
		// Wait for message
		if (FLEXCAN_DRV_GetTransferStatus(working_copy->instance_id, TX_MAILBOX) == STATUS_BUSY)
		{
			// wait till busy
		}
		else
		{
			*current_state = ANCIT_CAN_TX_COMPLETE;
		}
		break;

	case ANCIT_CAN_TX_COMPLETE:
		*current_state = ANCIT_CAN_TX_DONE;
		break;

	case ANCIT_CAN_TX_DONE:
		// Finalize and clean up
		*current_state = ANCIT_CAN_TX_IDLE;
		break;
	}
}

void ancit_can_tx_main(void)
{
	// Run state machine for all active instances
	for (uint8_t instance_id = 0; instance_id < CAN_MAX_INSTANCES; instance_id++)
	{
		if (can_active_instances[instance_id] == true)
		{
			ancit_can_tx_sm(instance_id);
		}
	}
}

void ancit_can_tx_onstart(void)
{
	// Loop through all CAN Tx messages
	for (uint8_t idx = 0; idx < gVars.can_tx_max; idx++)
	{
		// Create a pointer to the current message
		can_tx_message_t *msg = &can_tx_messages[idx];

		// If the message is enabled and it's of ON_START type
		if (msg->enabled && (msg->msg_type == CAN_TX_MSG_TYPE_ON_START))
		{
			// Mark it triggered
			msg->triggered = true;
		}
	}
}

void ancit_can_tx_one_ms(void)
{
	// Loop through all Tx messages
	for (uint8_t i = 0; i < gVars.can_tx_max; i++)
	{
		// Create a pointer to the current CAN Tx message
		can_tx_message_t *msg = &can_tx_messages[i];

		// If the message is enabled and cyclic
		if (msg->enabled && (msg->msg_type == CAN_TX_MSG_TYPE_CYCLIC))
		{
			// Decrement the tx interval counter
			msg->interval_counter--;

			// If the counter reaches zero
			if (msg->interval_counter == 0)
			{
				// Reset the interval counter with the set value
				msg->interval_counter = msg->set_interval_ms;

				// Indicate the message is triggered for transmission
				msg->triggered = true;
			}
		}
	}
}

#endif // CAN_TX_CONFIGURED
