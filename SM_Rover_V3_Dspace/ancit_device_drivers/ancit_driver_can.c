/*
 * ancit_dd_can.c
 *
 *  Created on: 17-Jun-2024
 *      Author: Narayan
 */
#include <ancit_driver_can.h>
#include "sdk_project_config.h"
#include "flexcan_driver.h"
#include "interrupt_manager.h"
#include "genx_config.h"


#ifdef UNIFIED_DIAGNOSTICS_SERVICES_CONFIGURED
#include "canif.h"
#endif




void CAN0_ORed_0_15_MB_IRQHandler(void);

status_t ancit_driver_can_init(uint8_t instance, flexcan_state_t *state,
//		const flexcan_user_config_t *data) {
//	/* Initialize the CAN Hardware */
//	return FLEXCAN_DRV_Init(instance, state, data);

		const flexcan_user_config_t *data) {
			/* Initialize the CAN Hardware */
			if(instance == 0){

#ifdef UNIFIED_DIAGNOSTICS_SERVICES_CONFIGURED
			INT_SYS_InstallHandler(CAN0_ORed_0_15_MB_IRQn, &CAN0_ORed_0_15_MB_IRQHandler, NULL);

				       /* Enable buttons interrupt */
			INT_SYS_EnableIRQ(CAN0_ORed_0_15_MB_IRQn);
#endif

				 //Installing the callback function

			FLEXCAN_DRV_Init(instance, state, data);

#ifdef UNIFIED_DIAGNOSTICS_SERVICES_CONFIGURED
		    FLEXCAN_DRV_InstallEventCallback(instance,FlexCAN0_Callback,NULL);
#endif

			return 0;
			}

			else{

				return FLEXCAN_DRV_Init(instance, state, data);

			}

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

void ancit_driver_can_SetRxMaskType(uint8_t instance,
		flexcan_rx_mask_type_t type) {
	FLEXCAN_DRV_SetRxMaskType(instance, type);
}

void ancit_driver_can_SetRxIndividualMask(uint8_t instance,
		flexcan_msgbuff_id_type_t id_type, uint8_t mb_idx, uint32_t mask) {
	FLEXCAN_DRV_SetRxIndividualMask(instance, id_type, mb_idx, mask);
}
