/*
 * fota_uart.h
 *
 *  Created on: 24-Sep-2025
 *      Author: Lenovo
 */

#ifndef FOTA_UART_H_
#define FOTA_UART_H_

#include "stdio.h"
#include "stdint.h"
#include "sdk_project_config.h"
#include "fota_utils.h"
#include "lpuart_irq.h"
#include "fota_uart_config.h"

#define UART_CHANNEL		0

typedef enum Uart_rx_sm{
	UART_RX_SM_IDLE,
	UART_RX_SM_RECEV_START_FH,
	UART_RX_SM_RECEV_START_SH,
	UART_RX_SM_RECEV_PROCESS_FH,
	UART_RX_SM_RECEV_PROCESS_SH,
	UART_RX_SM_RECEV_COMPLETE,
	UART_RX_SM_PROCESS_DATA
}Uart_rx_sm_t;


typedef enum Uart_tx_sm{
	UART_TX_SM_IDLE,
	UART_TX_SM_SEND_START,
	UART_TX_SM_SEND_PROCESS,
	UART_TX_SM_SEND_COMPLETE,
}Uart_tx_sm_t;

typedef enum srec_type{
	TYPE_0 = 48,
	TYPE_3 = 51,
	TYPE_5 = 53
}srec_type_t;

typedef struct Uart_Data{
	uint8_t rx_data[UART_BUFF_LEN];
	uint8_t tx_data[3];
}Uart_Data_t;

typedef struct srec_data{
	uint8_t 	record_start;
	srec_type_t 	type;
	uint8_t 	byte_count;
	uint32_t 	address;
	uint8_t		payload[PAYLOAD];
	uint8_t		crc;

}srec_data_t;





void fota_uart_rx_sm(void);
void fota_uart_init(void);
void fota_uart_tx_sm(void);

void fota_pre_process(void);
void fota_process_data(void);

#endif /* FOTA_UART_H_ */
