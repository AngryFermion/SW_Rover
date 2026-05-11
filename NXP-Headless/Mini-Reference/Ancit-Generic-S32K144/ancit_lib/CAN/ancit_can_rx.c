#include <ancit_driver_can.h>
#include "ancit_can_rx.h"
#include <stdio.h>
#include <string.h>
#include "ancit_common.h"

#ifdef CAN_RX_CONFIGURED

__attribute__((weak)) extern g_struct_can_rx_t can_rx[];
__attribute__((weak)) extern can_rx_message_registration_params_t rx_reg[];

//void sendEsp(const char *sourceStr) {
//	uint32_t bytesRemaining;
//
//	/* Send data via LPUART */
//	LPUART_DRV_SendData(INST_LPUART_2, (uint8_t*) sourceStr, strlen(sourceStr));
//	/* Wait for transmission to be successful */
//	while (LPUART_DRV_GetTransmitStatus(INST_LPUART_2, &bytesRemaining)
//			!= STATUS_SUCCESS) {
//	}
//}
//
//void sendUart(const char *sourceStr) {
//	uint32_t bytesRemaining;
//
//	/* Send data via LPUART */
//	LPUART_DRV_SendData(INST_LPUART_0, (uint8_t*) sourceStr, strlen(sourceStr));
//	/* Wait for transmission to be successful */
//	while (LPUART_DRV_GetTransmitStatus(INST_LPUART_0, &bytesRemaining)
//			!= STATUS_SUCCESS) {
//	}
//}

void ancit_can_rx_register_message(uint8_t can_idx, uint32_t msg_id,
		can_rx_message_notification_t notification) {
	can_rx[can_idx].sm_state = ANCIT_CAN_RX_INIT;

	can_rx[can_idx].msg.can_id = msg_id;
	can_rx[can_idx].rx_notification = notification;

	/* Set information about the data to be received
	 *  - 1 byte in length
	 *  - Standard message ID
	 *  - Bit rate switch enabled to use a different bitrate for the data segment
	 *  - Flexible data rate enabled
	 *  - Use zeros for FD padding
	 */
	flexcan_data_info_t dataInfo = { .data_length = 8U, .msg_id_type =
			FLEXCAN_MSG_ID_STD, .enable_brs = false, .fd_enable = false,
			.fd_padding = 0U };

	ancit_driver_can_ConfigRxMb(INST_FLEXCAN_CONFIG_1, (RX_MAILBOX + can_idx),
			&dataInfo, can_rx[can_idx].msg.can_id);
}

void ancit_can_rx_init(void) {
	// Initialize the entire structure to zero
	memset(&can_rx, 0, sizeof(g_struct_can_rx_t));

	//Register all the messages for Rx...
	for (uint8_t idx = 0; idx < gVars.can_rx_max; idx++) {
		ancit_can_rx_register_message(idx, rx_reg[idx].msg_id,
				rx_reg[idx].notification);
	}
}

void ancit_can_rx_sm(uint8_t can_idx) {
	switch (can_rx[can_idx].sm_state) {
	case ANCIT_CAN_RX_INIT:
		/* Start receiving data in RX_MAILBOX. */
		ancit_driver_can_receive(INST_FLEXCAN_CONFIG_1, (RX_MAILBOX + can_idx),
				&can_rx[can_idx].recvMsg);
		can_rx[can_idx].sm_state = ANCIT_CAN_RX_WAIT_FOR_MSG;
		break;

	case ANCIT_CAN_RX_WAIT_FOR_MSG:
		// Wait for message
		if (ancit_driver_can_GetTransferStatus(INST_FLEXCAN_CONFIG_1,
				(RX_MAILBOX + can_idx)) == STATUS_BUSY) {
			//wait till busy
		} else {
			can_rx[can_idx].sm_state = ANCIT_CAN_RX_PROCESS_MSG;
		}
		break;

	case ANCIT_CAN_RX_PROCESS_MSG:
		// Call the specific function registered as call back in the init
		can_rx[can_idx].rx_notification();
		can_rx[can_idx].sm_state = ANCIT_CAN_RX_DONE;
		break;

	case ANCIT_CAN_RX_DONE:
		// Finalize and clean up
		// Code to finalize
		can_rx[can_idx].sm_state = ANCIT_CAN_RX_INIT;
		break;
	}
}

void ancit_can_rx_main(void) {
	for (uint8_t idx = 0; idx < gVars.can_rx_max; idx++) {
		//If last message, come out of loop
		if (can_rx[idx].msg.can_id == 0) {
			break;
		} else {
			//Run the specific state machine....
			ancit_can_rx_sm(idx);
		}
	}
}
#endif //CAN_RX_CONFIGURED
