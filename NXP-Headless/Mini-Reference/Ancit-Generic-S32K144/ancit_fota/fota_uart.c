/*
 * fota_uart.c
 *
 *  Created on: 24-Sep-2025
 *      Author: Lenovo
 */

#include "fota_uart.h"

//#include "Fls_handler.h"

Uart_Data_t UartData;
srec_data_t srec;
Uart_rx_sm_t rx_state;
Uart_tx_sm_t tx_state;
//uint8_t fls_status;
status_t LPUARTStatus_rx;
status_t LPUARTStatus_tx;
uint32_t br; // bytes remaining
bool trigger_reset = false;
extern uint8_t boot_nvm_state;


void fota_uart_init(){

	status_t Init_ret;
		/*initializes a LPUART instance for operation*/
	Init_ret=LPUART_DRV_Init(UART_CHANNEL, &lpUartState0, &lpuart_0_InitConfig0);
	rx_state = UART_RX_SM_RECEV_START_FH;
	tx_state = UART_TX_SM_IDLE;
//	fls_status = false;

}

void fota_uart_rx_sm(void){

	switch(rx_state){

	case UART_RX_SM_IDLE:

			LPUARTStatus_rx = STATUS_ERROR;


		break;

	case UART_RX_SM_RECEV_START_FH:

			LPUARTStatus_rx = STATUS_ERROR;

			LPUARTStatus_rx = LPUART_DRV_ReceiveData(UART_CHANNEL,UartData.rx_data,UART_BUFF_LEN);

			rx_state = UART_RX_SM_RECEV_PROCESS_FH;


		break;

	case UART_RX_SM_RECEV_PROCESS_FH:

		if(LPUART_DRV_GetReceiveStatus(UART_CHANNEL,&br) != STATUS_BUSY){

						rx_state = UART_RX_SM_RECEV_COMPLETE;
		}


		break;

	case UART_RX_SM_RECEV_COMPLETE:



			rx_state = UART_RX_SM_PROCESS_DATA;

		break;

	case UART_RX_SM_PROCESS_DATA:

			fota_pre_process();
			fota_process_data();
			rx_state = UART_RX_SM_IDLE;

		break;

	default:
		break;

	}
}


void fota_uart_tx_sm(void){

	switch(tx_state){

	case UART_TX_SM_IDLE:
		LPUARTStatus_tx = STATUS_ERROR;
		break;

	case UART_TX_SM_SEND_START:

		LPUARTStatus_tx = LPUART_DRV_SendDataPolling(UART_CHANNEL, (const uint8_t*) UartData.tx_data,3);

		tx_state = UART_TX_SM_SEND_PROCESS;

		break;

	case UART_TX_SM_SEND_PROCESS:


		if(LPUART_DRV_GetTransmitStatus(UART_CHANNEL,&br) != STATUS_BUSY){

						tx_state = UART_TX_SM_SEND_COMPLETE;
		}

		break;

	case UART_TX_SM_SEND_COMPLETE:
		tx_state = UART_TX_SM_IDLE;
		break;

	default:
		break;

	}



}



void fota_pre_process(void){


	srec.record_start = UartData.rx_data[0];

	srec.type = UartData.rx_data[1];

	srec.byte_count =  asciiHexToByte((char *)&UartData.rx_data[2]);

	srec.address = asciiHexToUint32((char *)&UartData.rx_data[4]);
	// UartData.data has 256 ASCII hex characters starting from offset 7
	asciiHexToBytes(srec.payload, (char *)&UartData.rx_data[12], sizeof(srec.payload));

	srec.crc = asciiHexToByte((char *)&UartData.rx_data[268]);


}

void fota_process_data(void){

	switch(srec.type){

	case TYPE_0:
		// OTA initiated acknowledge
		// set boot flag 0xA5A5 and trigger a reset

//		tx_state = UART_TX_SM_SEND_START;
		boot_nvm_state = BOOT_NVM_WRITE_START;
		break;

	case TYPE_3:
		// trigger flash handler
		// after that trigger UART ACK

//		fls_status = FlsHdlr_WriteDataVerify(srec.address, srec.payload, sizeof(srec.payload),srec.crc,srec.byte_count);

//		if(fls_status == true){
//
//			UartData.tx_data[0] = 'A';
//			UartData.tx_data[1] = 'O';
//			UartData.tx_data[2] = 'K';
//			fls_status = false;
//
//		}
//		else{
//
//			UartData.tx_data[0] = 'N';
//			UartData.tx_data[1] = 'O';
//			UartData.tx_data[2] = 'K';
//
//		}

		tx_state = UART_TX_SM_SEND_START;
		break;

	case TYPE_5:

		trigger_reset = true;
		// update boot configuration parameters
		// set boot flag to 0

		// jump to application
//		Jump_to_App(0x0000C000);

		break;

	default:
		break;

	}

}




























