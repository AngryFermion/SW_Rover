/*
 * Boot_mngr.h
 *
 *  Created on: May 3, 2024
 *      Author: Rahul CK
 */

#ifndef BOOT_MANGR_H_
#define BOOT_MANGR_H_

#include "main.h"


#define DIG_CH3 0x2000		/*PTC10*/
#define EXTERNAL_SWITCH  DIG_CH3
#define BufSize 1

#define LEDPORT PTE
#define LEDPIN 7

void Boot_manager_Init(void);
void Read_BootFlag(void);
uint8_t Check_Boot_Comm(void);
void Start_Flash_Write(void);
void Fail_Handler(char *TxFailBuff);
void BootManager_UpdateBootConf(void);
bool BootMng_Chck_BootCfg_forInit(void);
void Bootmanager_Init_BootConf(void);
void BootManager_Erase_Flash(void);

void Bootmanager_clear_BootFlag(void);
void Get_Security_UID(void);
extern bool Erase_Cnfm_Flg;
extern char Timeout_Error_Responce;
extern char Jump_ack_Response;
extern char Data_Rcv_Error;
extern char Flash_Success;


void LPUART_SetReceiver(uint32_t instance, bool enable);

#endif /* BOOT_MANGR_H_ */
