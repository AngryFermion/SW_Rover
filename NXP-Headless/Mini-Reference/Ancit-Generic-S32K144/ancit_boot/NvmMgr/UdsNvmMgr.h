/*
 * UdsNvmMgr.h
 *
 *  Created on: 05-Mar-2025
 *      Author: V.H.
 */

#ifndef UDSNVMMGR_H_
#define UDSNVMMGR_H_

#include "stdint.h"
#include "flash_driver.h"

typedef enum{
	BOOT_NVM_IDLE,
	BOOT_NVM_WRITE_START,
	BOOT_NVM_WRITE_IN_PROCESS,
	BOOT_NVM_WRITE_COMPLETE
};


typedef struct
{
    status_t (*FLASH_EraseSector)   (const flash_ssd_config_t *pSSDConfig, uint32_t dest, uint32_t size);
    status_t (*FLASH_VerifySection) (const flash_ssd_config_t *pSSDConfig, uint32_t dest, uint16_t number, uint8_t marginLevel);
    status_t (*FLASH_Program)       (const flash_ssd_config_t *pSSDConfig, uint32_t dest, uint32_t size, const uint8_t *pData);
    status_t (*FLASH_ProgramCheck)  (const flash_ssd_config_t *pSSDConfig, uint32_t dest, uint32_t size, const uint8_t *pExpectedData, uint32_t *pFailAddr, uint8_t marginLevel);
} tFlashOptInfo;



typedef enum C40_TransferStatus{
	C40_WRITE_ERROR,
	C40_WRITE_SUCCESS,
	C40_READ_ERROR,
	C40_READ_SUCCESS,
	C40_ERASE_ERROR,
	C40_ERASE_SUCCESS,
} C40_TransferStatus_t;

typedef enum Checksum_statues{
	CHECKSUM_PASS,
	CHECKSUM_FAIL,
}Checksum_statues_t;

typedef enum
{
	FLSHDLR_INIT_OK,
	FLSHDLR_INIT_FAIL
}FlsHdlr_Init_Status_t;


typedef union
{
	uint8_t   BootCfg_blckData_u[24];
	struct
	{
		uint32_t Boot_Flag;									// Boot_Flag (will indicate the trigger from application )
		uint32_t AppEraseStatus;							// App erase status will become true when UDS will erase the Application
		uint32_t AppCRCStatus;								// CRC status will get updated when UDS Stack will report to boot manager saying CRC validation is successfull
		uint32_t BootCount;									// No of counts for successful flashed application
		uint8_t BootVersion[4] ; 							// Bootloader Version

	}data_s;

}BootManage_confblck_u;


#define BOOTMNG_CONFBLCK_STARTADDR		0x10000000
#define DFLS_SECTOR_LENGTH                  2048


FlsHdlr_Init_Status_t FlsHdlr_Init(void);
bool FlsHdlr_WriteData(uint32_t startaddr,uint8_t *dptr, uint16_t dlen);
status_t FlsHdlr_SectorErase(uint32_t Sector_Add ,uint16_t len);
status_t FlsHdlr_ReadData(uint32_t Read_Address, uint32_t Buff_len, uint8_t *Fls_Rx_Buff);
void Uds_NvmMgr_init(void);
void Uds_NvmMgr_sm(void);

#endif /* UDSNVMMGR_H_ */
