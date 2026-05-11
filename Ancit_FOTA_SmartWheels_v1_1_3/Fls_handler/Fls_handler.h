/*
 * Fls_handler.h
 *
 *  Created on: May 3, 2024
 *      Author: Rahul CK
 */

#ifndef FLS_HANDLER_H_
#define FLS_HANDLER_H_
#include "flash_driver.h"
#include "Boot_mangr_Cfg.h"
#define DisableAllInterrupts()  __asm(" cpsid i")
/*enable all interrupts*/
#define EnableAllInterrupts() 	__asm(" cpsie i")

//#define FLS_SECTOR_STRT_ADDR           0x00500000
#define DFLS_SECTOR_LENGTH                  2048

typedef enum
{
	FLSHDLR_INIT_OK,
	FLSHDLR_INIT_FAIL,
}FlsInit_Status_t;

typedef enum
{
	CHECKSUM_PASS,
	CHECKSUM_FAIL,
} Checksum_statues_t;


FlsInit_Status_t FlsHdlr_Init(void);
status_t FlsHdlr_SectorErase(uint32_t Sector_Add ,uint16_t len);
bool FlsHdlr_WriteData(uint32_t startaddr,uint8_t *dptr, uint16_t dlen);
bool FlsHdlr_WriteDataVerify(uint32_t startaddr,uint8_t *dptr, uint16_t dlen,uint8_t checksum,uint8_t num_bytes);
Checksum_statues_t Calculate_Checksum(Bootmnger_bootbytes *Boot_Clc);
status_t FlsHdlr_ReadData(uint32_t Read_Address, uint32_t Buff_len, uint8_t *Fls_Rx_Buff);
extern Bootmnger_bootbytes Boot_buffer;

#endif /* FLS_HANDLER_H_ */
