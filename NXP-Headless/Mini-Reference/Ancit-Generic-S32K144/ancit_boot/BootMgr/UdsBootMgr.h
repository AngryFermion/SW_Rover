/*
 * UdsBootMgr.h
 *
 *  Created on: 05-Mar-2025
 *      Author: V.H.
 */

#ifndef UDSBOOTMGR_H_
#define UDSBOOTMGR_H_
#include "stdint.h"

typedef enum{
	BOOTMGR_STATE_IDLE,
	BOOTMGR_PROCESS_CAN_RX,
	BOOTMGR_SET_BOOT_FLAG,
	BOOTMGR_RESET_ECU
};



void UDS_BootManager_init(void);
void UDS_BootManager_sm(void);

#endif /* UDSBOOTMGR_H_ */
