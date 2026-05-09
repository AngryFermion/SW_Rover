/*
 * ancit_dd_can.c
 *
 *  Created on: 17-Jun-2024
 *      Author: Narayan
 */
#include <ancit_driver_can.h>
#include "sdk_project_config.h"

status_t ancit_driver_can_init(uint8_t instance, flexcan_state_t *state,
		const flexcan_user_config_t *data) {
	/* Initialize the CAN Hardware */
	return FLEXCAN_DRV_Init(instance, state, data);
}

status_t ancit_driver_can_send(uint8_t instance, uint8_t mb_idx,
		const flexcan_data_info_t *tx_info, uint32_t msg_id,
		const uint8_t *mb_data) {

	/* Send the information via CAN */
	return FLEXCAN_DRV_Send(instance, mb_idx, tx_info, msg_id, mb_data);
}

status_t ancit_driver_can_GetTransferStatus(uint8_t instance, uint8_t mb_idx) {
	//Get the transfer flag status;
	return FLEXCAN_DRV_GetTransferStatus(instance, mb_idx);
}

status_t ancit_driver_can_receive(uint8_t instance, uint8_t mb_idx,
		flexcan_msgbuff_t *data) {
	return FLEXCAN_DRV_Receive(instance, mb_idx, data);
}

status_t ancit_driver_can_ConfigRxMb(uint8_t instance, uint8_t mb_idx,
		const flexcan_data_info_t *rx_info, uint32_t msg_id) {

	return FLEXCAN_DRV_ConfigRxMb(instance, mb_idx, rx_info, msg_id);
}
