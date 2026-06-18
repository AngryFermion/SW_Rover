/*
 * ancit_dd_uart.c
 *
 *  Created on: 20-Jun-2024
 *      Author: Narayan
 */
#include "sdk_project_config.h"

status_t ancit_driver_uart_Init(uint32_t instance,
		lpuart_state_t *lpuartStatePtr,
		const lpuart_user_config_t *lpuartUserConfig) {
	return LPUART_DRV_Init(instance, lpuartStatePtr, lpuartUserConfig);
}

uart_callback_t ancit_driver_uart_InstallRxCallback(uint32_t instance,
		uart_callback_t function, void *callbackParam) {
	return LPUART_DRV_InstallRxCallback(instance, function, callbackParam);
}

status_t ancit_driver_uart_SendData(uint32_t instance, const uint8_t *txBuff,
		uint32_t txSize) {
	return LPUART_DRV_SendData(instance, txBuff, txSize);
}

status_t ancit_driver_uart_GetTransmitStatus(uint32_t instance,
		uint32_t *bytesRemaining) {
	return LPUART_DRV_GetTransmitStatus(instance, bytesRemaining);
}

status_t ancit_driver_uart_ReceiveData(uint32_t instance, uint8_t *rxBuff,
		uint32_t rxSize) {
	return LPUART_DRV_ReceiveData(instance, rxBuff, rxSize);
}

status_t ancit_driver_uart_SetRxBuffer(uint32_t instance, uint8_t *rxBuff,
		uint32_t rxSize) {
	return LPUART_DRV_SetRxBuffer(instance, rxBuff, rxSize);
}


