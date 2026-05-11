/*
 * ancit_dd_uart.h
 *
 *  Created on: 20-Jun-2024
 *      Author: Narayan
 */

#ifndef ANCIT_DRIVER_UART_H_
#define ANCIT_DRIVER_UART_H_
#include "sdk_project_config.h"

status_t ancit_driver_uart_Init(uint32_t instance,
		lpuart_state_t *lpuartStatePtr,
		const lpuart_user_config_t *lpuartUserConfig);

status_t ancit_driver_uart_SendData(uint32_t instance, const uint8_t *txBuff,
		uint32_t txSize);

status_t ancit_driver_uart_GetTransmitStatus(uint32_t instance,
		uint32_t *bytesRemaining);

status_t ancit_driver_uart_ReceiveData(uint32_t instance, uint8_t *rxBuff,
		uint32_t rxSize);

status_t ancit_driver_uart_SetRxBuffer(uint32_t instance, uint8_t *rxBuff,
		uint32_t rxSize);

uart_callback_t ancit_driver_uart_InstallRxCallback(uint32_t instance,
		uart_callback_t function, void *callbackParam);

#endif /* ANCIT_DRIVER_UART_H_ */
