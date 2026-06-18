/*
 * Fls_handler.c
 *
 *  Created on: May 3, 2024
 *      Author: Rahul CK
 */
//Flash Api to erase the Sector
#include "Fls_handler.h"
#include <stdio.h>
#include "flash_driver.h"
#include "peripherals_flash_1.h"


//Global Variables
flash_ssd_config_t flashSSDConfig1;		//copy of configuration of flash driver

uint8_t RxBuffer[MAX_DATA_SIZE];
Checksum_statues_t Checksum_Ret;
FlsInit_Status_t FlsHdlr_Init(void)
{
	status_t Init_Status;

	/* Initialize Flash Driver  */
	Init_Status = FLASH_DRV_Init(&Flash_InitConfig0, &flashSSDConfig1);

	/** This is for to avoid the Hard fault **/
	flashSSDConfig1.CallBack = NULL_CALLBACK;

	if (STATUS_SUCCESS == Init_Status)
	{
		return FLSHDLR_INIT_OK;
	}
	else
	{
		return FLSHDLR_INIT_FAIL;
	}

}

status_t FlsHdlr_SectorErase(uint32_t Sector_Add ,uint16_t len)
{
	DisableAllInterrupts();
	//local variables
	status_t Ers_status  = STATUS_ERROR;
	//calling flash Erase Api to erase sector
	if( FLASH_DRV_EraseSector(&flashSSDConfig1, Sector_Add, len)== STATUS_SUCCESS)
	{
		/* Verify the erase operation at margin level value of 1, user read */
		Ers_status = FLASH_DRV_VerifySection(&flashSSDConfig1, Sector_Add, len / FTFx_DPHRASE_SIZE, 1u);
	}
	else
	{
		//return the status error as default
	}
	EnableAllInterrupts();
    return Ers_status;
}



//Flash Handler Api to write the data into the Flash Memory (P - Flash and D - Flash )
bool FlsHdlr_WriteData(uint32_t startaddr,uint8_t *dptr, uint16_t dlen)
{
	DisableAllInterrupts();
	//local variables
	bool retval = false;
	uint32_t failAddr;

	//Program the P- flash with incoming size
	if(FLASH_DRV_Program(&flashSSDConfig1, startaddr, dlen, dptr) == STATUS_SUCCESS)
	{
		/* Verify the erase operation at margin level value of 1, user read */
//		if(FLASH_DRV_ProgramCheck(&flashSSDConfig1, startaddr, dlen, dptr, &failAddr, 1u) == STATUS_SUCCESS)
//		{
//			retval = true;
//		}
		retval = true;
	}
	else
	{
		//return the default value
		retval = false;
	}

//if(startaddr!=BOOTCFG_STARTADDR)
//{
//	if(true==retval)
//	{
//		Checksum_Ret=Calculate_Checksum(&Boot_buffer);
//		if(CHECKSUM_PASS==Checksum_Ret)
//		{
//			retval=true;
//		}
//		else
//		{
//			retval=false;
//		}
//
//	}
//}
	EnableAllInterrupts();
	return retval;
}


bool FlsHdlr_WriteDataVerify(uint32_t startaddr,uint8_t *dptr, uint16_t dlen,uint8_t checksum, uint8_t num_bytes)
{
	DisableAllInterrupts();
	//local variables
	bool retval = false;
	uint32_t failAddr;

	//Program the P- flash with incoming size
	if(FLASH_DRV_Program(&flashSSDConfig1, startaddr, dlen, dptr) == STATUS_SUCCESS)
	{
		/* Verify the erase operation at margin level value of 1, user read */
//		if(FLASH_DRV_ProgramCheck(&flashSSDConfig1, startaddr, dlen, dptr, &failAddr, 1u) == STATUS_SUCCESS)
//		{
//			retval = true;
//		}
		retval = true;
	}
	else
	{
		//return the default value
		retval = false;
	}

	uint8_t verify_data[128];

	memcpy(verify_data,startaddr,dlen);

	uint8_t crc_index  = 0;
	uint32_t calculated_checksum_temp;
	uint8_t calculated_checksum;

	calculated_checksum_temp = num_bytes + ((startaddr>>24) & 0xFF) + ((startaddr>>16) & 0xFF) + ((startaddr>>8) & 0xFF) + (startaddr & 0xFF);

	for(;crc_index<dlen; crc_index++){
		calculated_checksum_temp += verify_data[crc_index];
	}

	calculated_checksum = (uint8_t) (0xFF & calculated_checksum_temp);

	calculated_checksum = ~calculated_checksum;

	if(calculated_checksum == checksum){
		retval = true;
	}

	else{
		retval = false;
		uint8_t k;
		k=0;
	}

//if(startaddr!=BOOTCFG_STARTADDR)
//{
//	if(true==retval)
//	{
//		Checksum_Ret=Calculate_Checksum(&Boot_buffer);
//		if(CHECKSUM_PASS==Checksum_Ret)
//		{
//			retval=true;
//		}
//		else
//		{
//			retval=false;
//		}
//
//	}
//}
	EnableAllInterrupts();
	return retval;
}



//Flash Handler Api to read the Data
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

Checksum_statues_t Calculate_Checksum(Bootmnger_bootbytes *Boot_Clc)
{
	Checksum_statues_t CksmRet;
	uint32_t checksum  = 0;
	uint8_t counter    = 0;
	 uint8_t CRC_phrase = 0;
	uint8_t i;
	uint8_t Check_Bytes= 0;

	/*Excluding the CRC size,PhraseType,PhraseType1  and Considering only Address and Data Bytes*/
	Check_Bytes = BOOT_BYTES_MAXSIZE - 3;

	/* Check for a valid SREC type */
	if ((Boot_Clc->Bootmnger_bootframes.PharesType < '0') || (Boot_Clc->Bootmnger_bootframes.PharesType > '9') )
	{
		return CHECKSUM_FAIL;
	}

	/* Check for a valid size and return if invalid */
	if (Boot_Clc->Bootmnger_bootframes.PharesSize > Check_Bytes )
	{
		return CHECKSUM_FAIL;
	}

	/* Initialize counter */
	counter = Boot_Clc->Bootmnger_bootframes.PharesSize;

	/* Add byte count to checksum */
	checksum += counter;

	if (Boot_Clc->Bootmnger_bootframes.PharesType == '3' )
	{ /* All the other records have a 16-bit address field */
		/* Add address */
		for (i = 0; i < 4; i++)
		{
			checksum += Boot_Clc->Bootmnger_bootframes.PharesAddress[i];
		}
		counter = counter - 4;
	}

	/* Add data */
	for (i = 0; ((i < (counter - 1)) && (counter <= MAX_DATA_SIZE + 1)); i++)
	{
		checksum += Boot_Clc->Bootmnger_bootframes.PharesData[i];
	}

	/* Keep least significant byte */
	CRC_phrase = (uint8_t) (0xFF & checksum);

	/* Compute one's complement of LSB */
	CRC_phrase = ~CRC_phrase;

	/* Check if CRC matches */
	if (CRC_phrase == Boot_Clc->Bootmnger_bootframes.PharesChecksum)
	{
		CksmRet = CHECKSUM_PASS;

	}
	else
		{
		CksmRet = CHECKSUM_FAIL;
		}
	return CksmRet;

}
