/*
 * Boot_mangr_Cfg.h
 *
 *  Created on: 06-May-2024
 *      Author: Rahul CK
 */

#ifndef BOOT_MANGR_CFG_H_
#define BOOT_MANGR_CFG_H_


#include "stdio.h"
#define BOOT_BYTES_MAXSIZE    136  //24   Assuming a maximum size for boot bytes
#define MAX_DATA_SIZE         128//16  Assuming a maximum size for PharesData
#define BOOTCFG_STARTADDR     0x10000000
#define DFLS_SEC_SIZE         2048
#define PFLS_SEC_SIZE 		  4096
#define BOOTCFG_DATA_SIZE     16
#define APPL_START_ADDRS      0x0000C000
#define BOARD_DATA_BYTES      18
#define BOOTLODER_START_ADDRS 0x00000000

#define UPDATE_REQUEST_FLAG		0x0001

typedef union{
uint8_t Boot_bytes[BOOT_BYTES_MAXSIZE];

	struct
	{
		char PhraseStyle;
		char PharesType;
		uint8_t PharesSize;
		uint8_t PharesAddress[4];
		uint8_t PharesData[MAX_DATA_SIZE];
		uint8_t PharesChecksum;
	}Bootmnger_bootframes;
}Bootmnger_bootbytes;


typedef union{
	uint8_t Boot_Cfg_Bytes[BOOTCFG_DATA_SIZE];
		struct
		{
			uint32_t  Boot_Flag;
			uint32_t Boot_Appl_Count;
			uint32_t Boot_Retry_Count;

		}BootCfg_frames;

}BootCfg_Data_Bytes;

typedef union {
	char Board_Data[BOARD_DATA_BYTES];
	struct
	{
	uint8_t Board_Msg;
	char Msg_ID;
	uint8_t Board_UID[16];
	}Board_Data_frame;
}Board_Data_info;


extern BootCfg_Data_Bytes BootCfg_Data;
extern Bootmnger_bootbytes Boot_buffer;
extern BootCfg_Data_Bytes  *BootCfg_ptr;
extern Board_Data_info      Board_info;
#endif /* BOOT_MANGR_CFG_H_ */
