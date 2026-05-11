/*
 * Boot_mangr.c
 *
 *  Created on: May 3, 2024
 *      Author: Rahul CK
 */

#include "Boot_mangr_Cfg.h"
#include "Boot_mangr.h"
#include "Fls_handler.h"
#include "fota_uart.h"




uint8_t buff[14];

Bootmnger_bootbytes  Boot_buffer;
BootCfg_Data_Bytes  *BootCfg_ptr = NULL;
BootCfg_Data_Bytes   BootCfg_Data;
Board_Data_info      Board_info;


extern Uart_tx_sm_t tx_state;
extern Uart_Data_t UartData;
extern bool trigger_reset;

status_t Ers_Ret;
void Boot_manager_Init(void)
{

	  /* Initialize LPUART instance */


	   FlsHdlr_Init();

	   FlsHdlr_ReadData(BOOTCFG_STARTADDR, BOOTCFG_DATA_SIZE,BootCfg_Data.Boot_Cfg_Bytes );

	   		//copy the Boot_Confdata into the global pointer
	   BootCfg_ptr = BootCfg_Data.Boot_Cfg_Bytes;

		//check for boot Config init

	   fota_uart_init();

		if(BootCfg_ptr->BootCfg_frames.Boot_Flag != UPDATE_REQUEST_FLAG && BootCfg_ptr->BootCfg_frames.Boot_Flag != 0xFFFFFFFF){

			// no re-programming requested
			// jump to application
			Jump_to_App(APPL_START_ADDRS);

		}

		else if(BootCfg_ptr->BootCfg_frames.Boot_Flag == UPDATE_REQUEST_FLAG){

			//trigger an AOK response
			UartData.tx_data[0] = 'A';
			UartData.tx_data[1] = 'O';
			UartData.tx_data[2] = 'K';
			tx_state = UART_TX_SM_SEND_START;


		}



		BootManager_Erase_Flash();


		while(1)
		{
			//Start state Machine

			fota_uart_rx_sm();
			fota_uart_tx_sm();

			if(trigger_reset == true){

				Bootmanager_clear_BootFlag();

				//Jump_to_App(APPL_START_ADDRS);
				SystemSoftwareReset();

			}


		}
}



bool BootMng_Chck_BootCfg_forInit(void)
{
	if((BootCfg_ptr->BootCfg_frames.Boot_Flag==0xFFFFFFFF)&&(BootCfg_ptr->BootCfg_frames.Boot_Appl_Count==0xFFFFFFFF))
	{
		return true;
	}
	else
	{
		return false;
	}
}

void Bootmanager_Init_BootConf(void)
{
	BootCfg_ptr->BootCfg_frames.Boot_Flag=0x00;
	BootCfg_ptr->BootCfg_frames.Boot_Appl_Count=0x00;
	BootCfg_ptr->BootCfg_frames.Boot_Retry_Count=0x00;
	BootManager_UpdateBootConf();

}

void BootManager_Erase_Flash(void)
{
	   //Flash memory Erase Api
		uint32_t app_start_addr;
		app_start_addr = APPL_START_ADDRS;
		for(uint8_t Ers_Sector_Counter=0;Ers_Sector_Counter<111;Ers_Sector_Counter++)
		{
		Ers_Ret = FlsHdlr_SectorErase(app_start_addr,PFLS_SEC_SIZE);
		app_start_addr=app_start_addr+PFLS_SEC_SIZE;
		}


}

void Bootmanager_clear_BootFlag(void){

	BootCfg_ptr->BootCfg_frames.Boot_Flag=0x00;
	BootCfg_ptr->BootCfg_frames.Boot_Appl_Count += 1;
	BootCfg_ptr->BootCfg_frames.Boot_Retry_Count=0x00;
	BootManager_UpdateBootConf();

}

 void BootManager_UpdateBootConf(void)
{
	//erase one complete first sector of dflash
	FlsHdlr_SectorErase(BOOTCFG_STARTADDR, DFLS_SEC_SIZE);

	//rewrite the Updated BootConfig Block
	FlsHdlr_WriteData(BOOTCFG_STARTADDR,BootCfg_ptr->Boot_Cfg_Bytes,BOOTCFG_DATA_SIZE);

	//Read it Back
	FlsHdlr_ReadData(BOOTCFG_STARTADDR, BOOTCFG_DATA_SIZE, BootCfg_ptr->Boot_Cfg_Bytes);
}






