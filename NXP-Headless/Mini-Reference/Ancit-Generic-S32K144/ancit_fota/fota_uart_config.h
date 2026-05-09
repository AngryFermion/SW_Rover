/*
 * fota_uart_config.h
 *
 *  Created on: 24-Sep-2025
 *      Author: Lenovo
 */

#ifndef FOTA_UART_CONFIG_H_
#define FOTA_UART_CONFIG_H_


#define NO_OF_LINES			1
#define UART_BUFF_LEN		270*NO_OF_LINES
#define OVERHEAD			7
#define PAYLOAD				UART_BUFF_LEN/2 - OVERHEAD

#endif /* FOTA_UART_CONFIG_H_ */
