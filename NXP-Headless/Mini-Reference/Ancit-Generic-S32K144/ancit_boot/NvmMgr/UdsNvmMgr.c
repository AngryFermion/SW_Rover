/*
 * UdsNvmMgr.c
 *
 *  Created on: 05-Mar-2025
 *      Author: V.H.
 */

#include "../NvmMgr/UdsNvmMgr.h"

#include "stdio.h"
#include "flash_driver.h"
#include "peripherals_flash_1.h"

#include "../BootMgr/UdsBootMgr.h"
uint8_t clean = 0;
extern uint8_t boot_mgr_state;
uint8_t boot_nvm_state;
FlsHdlr_Init_Status_t fls_status;
bool write_status = false;
flash_ssd_config_t flashSSDConfig;		//copy of configuration of flash driver
uint8_t data[1] = {0xAA};
BootManage_confblck_u BootMng_ConfData ;
BootManage_confblck_u *btcnfptr = NULL;
status_t status = STATUS_ERROR;


void Uds_NvmMgr_init(void){

	boot_nvm_state = BOOT_NVM_IDLE;
	fls_status = FlsHdlr_Init();

}
void Uds_NvmMgr_sm(void){

	switch(boot_nvm_state){

	case BOOT_NVM_IDLE:
		break;

	case BOOT_NVM_WRITE_START:

		btcnfptr = BootMng_ConfData.BootCfg_blckData_u;

		btcnfptr->data_s.Boot_Flag = 1;

		FlsHdlr_ReadData(BOOTMNG_CONFBLCK_STARTADDR, 24,BootMng_ConfData.BootCfg_blckData_u );
		btcnfptr->data_s.Boot_Flag = 1;
		status = FlsHdlr_SectorErase(BOOTMNG_CONFBLCK_STARTADDR, DFLS_SECTOR_LENGTH);



		boot_nvm_state = BOOT_NVM_WRITE_IN_PROCESS;


		break;

	case BOOT_NVM_WRITE_IN_PROCESS:

		write_status = FlsHdlr_WriteData(BOOTMNG_CONFBLCK_STARTADDR,btcnfptr->BootCfg_blckData_u,24);

		if(write_status == true){
			boot_nvm_state = BOOT_NVM_WRITE_COMPLETE;
		}
		break;

	case BOOT_NVM_WRITE_COMPLETE:
		boot_nvm_state = BOOT_NVM_IDLE;
		boot_mgr_state = BOOTMGR_RESET_ECU;
		break;

	default:
		break;

	}

}

FlsHdlr_Init_Status_t FlsHdlr_Init(void)
{
	status_t C40Status;

	/* Initialize Flash Driver  */
	C40Status = FLASH_DRV_Init(&Flash_InitConfig0, &flashSSDConfig);

	/** This is for to avoid the Hard fault **/
	flashSSDConfig.CallBack = NULL_CALLBACK;

	if (STATUS_SUCCESS == C40Status)
	{
		return FLSHDLR_INIT_OK;
	}

	else
	{
		return FLSHDLR_INIT_FAIL;
	}

}

bool FlsHdlr_WriteData(uint32_t startaddr,uint8_t *dptr, uint16_t dlen)
{
	//local variables
	bool retval = false;
	uint32_t failAddr;
	uint8_t loc_databuff[256] ;
	uint16_t i ;
	uint16_t dsize ;


	/*Aligning the Data into 4bytes*/
	dsize =  dlen;
	if(dlen % 16 !=0)
	{
		 dsize = ((dsize /16 ) +1 )* 16;

		 for( i = dlen-1;i<dsize;i++)
		 {
			 loc_databuff[i] = 0xFF;
		 }

	}

	for (i = 0; i < dlen; i++)
	{
		/*Copying the Data into DATA_BUFF*/
		loc_databuff[i] = dptr[i];
	}

	dlen = dsize;

	//Program the P- flash with incoming size
	if(FLASH_DRV_Program(&flashSSDConfig, startaddr, dlen, loc_databuff) == STATUS_SUCCESS)
	{
		/* Verify the erase operation at margin level value of 1, user read */
		if(FLASH_DRV_ProgramCheck(&flashSSDConfig, startaddr, dlen, loc_databuff, &failAddr, 1u) == STATUS_SUCCESS)
		{
			retval = true;
		}
	}
	else
	{
		//return the default value
	}
	return retval;
}

status_t FlsHdlr_SectorErase(uint32_t Sector_Add ,uint16_t len)
{
	//local variables
	status_t retval = STATUS_ERROR;

	//calling flash Erase Api to erase sector
	if(FLASH_DRV_EraseSector(&flashSSDConfig, Sector_Add, len) == STATUS_SUCCESS)
	{
		/* Verify the erase operation at margin level value of 1, user read */
		retval = FLASH_DRV_VerifySection(&flashSSDConfig, Sector_Add, len / FTFx_DPHRASE_SIZE, 1u);
	}
	else
	{
		//return the status error as default
	}

    return retval;
}

status_t FlsHdlr_ReadData(uint32_t Read_Address, uint32_t Buff_len, uint8_t *Fls_Rx_Buff)
{
	//local variables
	status_t retval = STATUS_SUCCESS;
	uint16_t loopvar1 =0;
	volatile uint8_t *strtaddr = (volatile uint8_t*)Read_Address;

	// Read the data directly by dereferencing as there is no API to read the flash memory
	for(loopvar1 = 0; loopvar1 < Buff_len; loopvar1++)
	{
		Fls_Rx_Buff[loopvar1] = *(strtaddr + loopvar1);
	}

	return retval;

}


