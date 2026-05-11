/*
 * ancit_uart_esp.c
 *
 *  Created on: 18-May-2024
 *      Author: Narayan
 */
#include <ancit_driver_uart.h>
#include "ancit_uart_conn.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "ancit_dwin_tx.h"
#include "ancit_dwin_rx.h"
#include "ancit_timer.h"

#ifdef UART_CONN_CONFIGURED

// Global state variable
ext_uart_tx_state_t current_state = ANCIT_UART_CONN_TX_IDLE;

static StringQueue txQueue;
static char currentString[MAX_STRING_LENGTH];
static uint8_t currentStringLength;

void ancit_uart_conn_sm(void) {
	static uint32_t bytesRemaining;

	switch (current_state) {
	case ANCIT_UART_CONN_TX_IDLE:
		if (!ancit_uart_conn_IsQueueEmpty()) {
			ancit_uart_conn_DequeueString(currentString, &currentStringLength);
			current_state = ANCIT_UART_CONN_TX_START;
		}
		break;

	case ANCIT_UART_CONN_TX_START:
		/* Send data via LPUART */
		ancit_driver_uart_SendData(EXT_UART, (uint8_t*) currentString,
				currentStringLength);

		current_state = ANCIT_UART_CONN_TX_WAIT_FOR_COMPLETE;
		break;

	case ANCIT_UART_CONN_TX_WAIT_FOR_COMPLETE:
		// Check if messsage is sent
		if (ancit_driver_uart_GetTransmitStatus(EXT_UART, &bytesRemaining)
				!= STATUS_SUCCESS) {
			//wait till busy
		} else {
			current_state = ANCIT_UART_CONN_TX_COMPLETE;
		}
		break;

	case ANCIT_UART_CONN_TX_COMPLETE:
		current_state = ANCIT_UART_CONN_TX_DONE;
		break;

	case ANCIT_UART_CONN_TX_DONE:
		// Perform any cleanup or final steps
		current_state = ANCIT_UART_CONN_TX_IDLE; // Return to idle or end the process
		break;
	}
}

// Enqueue string for transmission
bool ancit_uart_conn_EnqueueString(const char *str, uint8_t length) {
	if (txQueue.count >= QUEUE_SIZE) {
		return false;  // Queue is full, cannot enqueue more strings
	}

	memcpy(txQueue.buffer[txQueue.tail], str, length);
	txQueue.length[txQueue.tail] = length;  // Store the given length
	txQueue.tail = (txQueue.tail + 1) % QUEUE_SIZE;
	txQueue.count++;
	return true;  // Successfully enqueued the string
}

// Dequeue string for transmission
void ancit_uart_conn_DequeueString(char *str, uint8_t *length) {
	if (txQueue.count > 0) {
		memcpy(str, txQueue.buffer[txQueue.head],
				txQueue.length[txQueue.head]);
		*length = txQueue.length[txQueue.head];  // Get the length
		txQueue.head = (txQueue.head + 1) % QUEUE_SIZE;
		txQueue.count--;
	}
}

// Check if queue is empty
bool ancit_uart_conn_IsQueueEmpty(void) {
	return txQueue.count == 0;
}

void ancit_uart_conn_start(void) {
	//Start the UART Hardware
	ancit_driver_uart_Init(EXT_UART, &lpUartState0, &lpuart_0_InitConfig0);

//	ancit_init_timeout(&uart_conn_delay_ctr, uart_conn_set_delay_test);
	//Initialize the state machine to idle
	current_state = ANCIT_UART_CONN_TX_IDLE;

#ifdef DWIN_DISPLAY_CONFIGURED
	ancit_dwin_tx_start();
	ancit_dwin_rx_start();
#endif
}

void ancit_uart_conn_main(void) {
#ifdef DWIN_DISPLAY_CONFIGURED
	ancit_dwin_tx_main();
	ancit_dwin_rx_main();
#endif

	ancit_uart_conn_sm();
}

#endif //UART_CONN_CONFIGURED
