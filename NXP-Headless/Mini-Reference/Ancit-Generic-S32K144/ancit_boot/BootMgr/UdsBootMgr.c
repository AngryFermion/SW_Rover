/*
 * UdsBootMgr.c
 *
 *  Created on: 05-Mar-2025
 *      Author: V.H.
 */

#include "../BootMgr/UdsBootMgr.h"

#include "system_S32K144.h"

#include "../NvmMgr/UdsNvmMgr.h"
uint8_t boot_mgr_state;
uint8_t *Uds_can_rx_data;
extern uint8_t boot_nvm_state;
typedef void(*jump)(uint32);
  jump      *func = NULL;

  uint32_t ADDR_APP = 0x00000000;

void UDS_BootManager_init(void){
	boot_mgr_state = BOOTMGR_STATE_IDLE;
}

void UDS_BootManager_sm(void){


	switch(boot_mgr_state){

	case BOOTMGR_STATE_IDLE:
		break;

	case BOOTMGR_PROCESS_CAN_RX:

		if(Uds_can_rx_data[1] == 0x10){

			if(Uds_can_rx_data[2] == 0x02){

				boot_mgr_state = BOOTMGR_SET_BOOT_FLAG;
			}
		}

		break;

	case BOOTMGR_SET_BOOT_FLAG:

		boot_nvm_state = BOOT_NVM_WRITE_START;
		boot_mgr_state = BOOTMGR_STATE_IDLE;

		break;

	case BOOTMGR_RESET_ECU:



					 //disable the interrupts
		SystemSoftwareReset();

		break;

	default:
		break;



	}

}
