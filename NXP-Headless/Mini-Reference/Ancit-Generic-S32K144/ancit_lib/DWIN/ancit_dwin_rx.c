/*
 * ancit_uart_conn_rx.c
 *
 *  Created on: 09-Jun-2024
 *      Author: Narayan
 */
#include <ancit_driver_uart.h>
#include <string.h>
#include "ancit_dwin_rx.h"
#include "ancit_uart_conn.h"
#include "ancit_timer.h"
#include "ancit_common.h"
#include "ancit_driver_digital_io.h"

#ifdef DWIN_DISPLAY_CONFIGURED
#include "../../src/genx_dwin.h"

static dwin_display_rx_t dwin_rx;
extern dwin_mem_items_t dwin_mem[DWIN_VP_MAX_ITEMS];

/* UART rx callback for continuous reception, byte by byte */
void ancit_dwin_rx_callback(void *driverState, uart_event_t event,
		void *userData) {
	/* Unused parameters */
	(void) driverState;
	(void) userData;

	/* Check the event type */
	if (event == UART_EVENT_RX_FULL) {
		switch (dwin_rx.sm_state) {
		case UART_RX_WAIT_FOR_HEADER:
			//Check if proper header is received
			if (dwin_rx.buff[DWIN_RX_IDX_HEADER1] == 0x5A
					&& dwin_rx.buff[DWIN_RX_IDX_HEADER2] == 0xA5) {
				//Set the Rx buffer to 3rd index
				//Set the length based on the length received in the command
				ancit_driver_uart_SetRxBuffer(EXT_UART, &dwin_rx.buff[3],
						dwin_rx.buff[DWIN_RX_IDX_LEN]);
				dwin_rx.rx_ok = true;
			} else {
				dwin_rx.errCounter++;
			}
			break;

		case UART_RX_WAIT_FOR_DATA:
			//Proper Rx command, process it....
			if (dwin_rx.buff[DWIN_RX_IDX_CMD] == 0x83) {
				dwin_rx.rx_ok = true;
			} else if (dwin_rx.buff[DWIN_RX_IDX_CMD] == 0x82) {
				//if 0x82 is received, that is a response to a write command,
				//So, ignore it
				dwin_rx.errCounter++;
			} else {
				//Error in command reception
				dwin_rx.errCounter++;
			}
			break;
		}
	} else if (event == UART_EVENT_ERROR) {
		dwin_rx.errCounter++;
	}
}

bool ancit_dwin_rx_any_errors(void) {
	uint8_t errStatus = false;
	//If error counter has increased, set the error flag as true
	if (dwin_rx.errCounter > 0) {
		errStatus = true;
	} else if (dwin_rx.timer_started == true
			&& is_timeout_expired(&dwin_rx.timer)) {
		//If timeout has occured, then set the error flag as true...
		errStatus = true;
	}

	return errStatus;
}

void ancit_dwin_rx_set_value_by_vp_addr_array(uint16_t vp_addr, uint8_t *data) {
	int8_t index = find_index_by_vp_addr(vp_addr);
	value_union_t rx_value;
	if (index == -1) {
		//vp_addr not found
		return;
	}

	// Now directly use the index to access the item and set the value
	switch (dwin_mem[index].type) {
	case VALUE_INT16:
		rx_value.int16_value = (data[0] << 8 | data[1]);
		dwin_mem[index].value.int16_value = rx_value.int16_value;
		break;
	case VALUE_INT32:
		rx_value.int32_value = ((data[0] << 24) | (data[1] << 16)
				| (data[2] << 8) | data[3]);
		dwin_mem[index].value.int32_value = rx_value.int32_value;
		break;
	case VALUE_UINT16:
		rx_value.uint16_value = (data[0] << 8 | data[1]);
		dwin_mem[index].value.uint16_value = rx_value.uint16_value;
		break;
	case VALUE_UINT32:
		rx_value.uint32_value = ((data[0] << 24) | (data[1] << 16)
				| (data[2] << 8) | data[3]);
		dwin_mem[index].value.uint32_value = rx_value.uint32_value;
		break;
	case VALUE_FLOAT:
		rx_value.float_value = ((data[0] << 24) | (data[1] << 16)
				| (data[2] << 8) | data[3]);
		dwin_mem[index].value.float_value = rx_value.float_value;
		break;
	case VALUE_DOUBLE:
		memcpy(&rx_value, data, 8);
		dwin_mem[index].value.double_value = rx_value.double_value;
		break;
	}
	dwin_mem[index].value_updated = true;
}

void ancit_dwin_rx_process_data(uint8_t *data) {
	uint16_t vp_addr = (data[DWIN_RX_IDX_ADDR1] << 8) | data[DWIN_RX_IDX_ADDR2];
	uint8_t word_count = data[DWIN_RX_IDX_WORD_COUNT];
	int8_t index = -1;

	//As of now, process only if word count is 1
	if (word_count == 1) {
		ancit_dwin_rx_set_value_by_vp_addr_array(vp_addr,
				&data[DWIN_RX_IDX_DATA_START]);

		index = find_index_by_vp_addr(vp_addr);

		if (index >= 0) {
			ancit_dwin_rx_data_rcvd_notfn(index);
		}
	}

}

void ancit_dwin_rx_sm(void) {
	static volatile uint8_t status = 0;
	uint32_t rx_pin_status = 0;

	switch (dwin_rx.sm_state) {
	case UART_RX_INIT:
		dwin_rx.timer_started = false;
		dwin_rx.errCounter = 0;
		dwin_rx.rx_ok = false;
		//Start the UART receiver
		ancit_driver_uart_ReceiveData(EXT_UART, &dwin_rx.buff[0], 3U);

		//Provide the start index for receiving, i.e.,0
		//First wait for the header bytes plus length (3 bytes)
		ancit_driver_uart_SetRxBuffer(EXT_UART, &dwin_rx.buff[0], 3U);
		dwin_rx.sm_state = UART_RX_WAIT_FOR_RX_START;
		break;

	case UART_RX_WAIT_FOR_RX_START:
		//Check if Rx Pin goes low, this is based on the UART RX protocol
		//where start bit is low

		rx_pin_status = ancit_dd_gpio_read(PTC, 2);
		if (rx_pin_status == LOW) {
			//If low, start a timer, this is to ensure that the whole RX happens
			//within a stipulated time
			ancit_init_timeout(&dwin_rx.timer, DWIN_RX_TIMEOUT_MS);
			dwin_rx.timer_started = true;
			//Go and wait for the header now...
			dwin_rx.sm_state = UART_RX_WAIT_FOR_HEADER;
		}
		break;

	case UART_RX_WAIT_FOR_HEADER:
		//If header reception is proper (set in rx callback function)
		if (dwin_rx.rx_ok == true) {
			dwin_rx.rx_ok = false;
			//Go to wait for data now...
			dwin_rx.sm_state = UART_RX_WAIT_FOR_DATA;
		} else if (ancit_dwin_rx_any_errors()) {
			//In case of error or timeout, restart process again
			dwin_rx.sm_state = UART_RX_DONE;
		}
		break;

	case UART_RX_WAIT_FOR_DATA:
		//If data reception is proper (set in rx callback function)
		if (dwin_rx.rx_ok == true) {
			//go to process data
			dwin_rx.sm_state = UART_RX_PROCESS_DATA;
		} else if (ancit_dwin_rx_any_errors()) {
			//In case of error or timeout, restart process again
			dwin_rx.sm_state = UART_RX_DONE;
		}
		break;
	case UART_RX_PROCESS_DATA:
		//Function to process the received data from DWIN
		ancit_dwin_rx_process_data(dwin_rx.buff);

		dwin_rx.sm_state = UART_RX_DONE;
		break;

	case UART_RX_DONE:
		//Reset the rx buffer
		memset(&dwin_rx.buff[0], 0, MAX_RX_BUFFER_SIZE);

		//Go back to init, to start a new RX
		dwin_rx.sm_state = UART_RX_INIT;
		break;
	}

	(void) status;
}

void ancit_dwin_rx_start(void) {
	//UART would have been initialised already by the basic function
	/* Install the callback for rx events */
	ancit_driver_uart_InstallRxCallback(EXT_UART, ancit_dwin_rx_callback, NULL);

	//Set the state machine to Init...
	dwin_rx.sm_state = UART_RX_INIT;
}

void ancit_dwin_rx_main(void) {
	ancit_dwin_rx_sm();
}

#endif //#ifdef DWIN_DISPLAY_CONFIGURED
