/*
 * ancit_dd_can.h
 *
 *  Created on: 17-Jun-2024
 *      Author: Narayan
 */

#ifndef ANCIT_DRIVER_CAN_H_
#define ANCIT_DRIVER_CAN_H_
#include "sdk_project_config.h"

status_t ancit_driver_can_init(uint8_t instance, flexcan_state_t *state,
		const flexcan_user_config_t *data);

status_t ancit_driver_can_config_tx_msg(uint8_t instance, uint8_t mb_idx,
		const flexcan_data_info_t *tx_info, uint32_t msg_id);

status_t ancit_driver_can_send(uint8_t instance, uint8_t mb_idx,
		const flexcan_data_info_t *tx_info, uint32_t msg_id,
		const uint8_t *mb_data);

status_t ancit_driver_can_GetTransferStatus(uint8_t instance, uint8_t mb_idx);

status_t ancit_driver_can_receive(uint8_t instance, uint8_t mb_idx,
		flexcan_msgbuff_t *data);

status_t ancit_driver_can_ConfigRxMb(uint8_t instance, uint8_t mb_idx,
		const flexcan_data_info_t *rx_info, uint32_t msg_id);

#endif /* ANCIT_DRIVER_CAN_H_ */
