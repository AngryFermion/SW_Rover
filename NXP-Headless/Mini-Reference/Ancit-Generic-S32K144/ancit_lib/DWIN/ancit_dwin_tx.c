/*
 * ancit_dwin.c
 *
 *  Created on: 08-Jun-2024
 *      Author: Narayan
 */
#include "ancit_dwin.h"
#include "ancit_dwin_tx.h"
#include "ancit_uart_conn.h"
#include "ancit_timer.h"
#include "string.h"

#ifdef DWIN_DISPLAY_CONFIGURED
#include "../../src/genx_dwin.h"

//dwin tx header
char dwin_header_write_sequence[4] = { 0x5A, 0xA5, 0x05, 0x82 };

extern dwin_mem_items_t dwin_mem[DWIN_VP_MAX_ITEMS];

uint32_t dwin_cyclic_delay_ctr = 0;
uint32_t dwin_cyclic_delay_ms = 100;

void ancit_dwin_tx_start(void) {
	//Initialize all memory structure, where init value is set to true
	for (uint8_t idx = 0; idx < DWIN_VP_MAX_ITEMS; idx++) {
		if (dwin_mem[idx].init == true) {
			dwin_mem[idx].tx_type = DWIN_PKT_TX_TYPE_CYCLIC;
			dwin_mem[idx].tx_triggered = false;
			dwin_mem[idx].rx_triggered = false;
			dwin_mem[idx].value.int32_value = 0;
			dwin_mem[idx].value_updated = false;
		}
	}
	ancit_init_timeout(&dwin_cyclic_delay_ctr, dwin_cyclic_delay_ms);
}

bool ancit_dwin_tx_enqueue_index(uint8_t index) {
	bool status = false;
	char send_buffer[32];
	int current_length = 4; // Start after the header
	dwin_mem_items_t item = dwin_mem[index];

	memcpy(send_buffer, dwin_header_write_sequence, 4); // Copy the entire header

	// Add the address
	send_buffer[current_length++] = (item.vp_addr >> 8) & 0xFF; // MSB
	send_buffer[current_length++] = item.vp_addr & 0xFF; // LSB

	// Handle each type according to its specific size and copy in big-endian format
	switch (item.type) {
	case VALUE_INT16:
		send_buffer[current_length++] = (item.value.int16_value >> 8) & 0xFF;
		send_buffer[current_length++] = item.value.int16_value & 0xFF;
		break;
	case VALUE_INT32:
		send_buffer[current_length++] = (item.value.int32_value >> 24) & 0xFF;
		send_buffer[current_length++] = (item.value.int32_value >> 16) & 0xFF;
		send_buffer[current_length++] = (item.value.int32_value >> 8) & 0xFF;
		send_buffer[current_length++] = item.value.int32_value & 0xFF;
		break;
	case VALUE_UINT16:
		send_buffer[current_length++] = (item.value.uint16_value >> 8) & 0xFF;
		send_buffer[current_length++] = item.value.uint16_value & 0xFF;
		break;
	case VALUE_UINT32:
		send_buffer[current_length++] = (item.value.uint32_value >> 24) & 0xFF;
		send_buffer[current_length++] = (item.value.uint32_value >> 16) & 0xFF;
		send_buffer[current_length++] = (item.value.uint32_value >> 8) & 0xFF;
		send_buffer[current_length++] = item.value.uint32_value & 0xFF;
		break;
	case VALUE_FLOAT:
		memcpy(&send_buffer[current_length], &item.value.float_value, 4);
		current_length += 4;
		break;
	case VALUE_DOUBLE:
		memcpy(&send_buffer[current_length], &item.value.double_value, 8);
		current_length += 8;
		break;
	}

	// Set the payload length in the third byte of the header
	send_buffer[2] = current_length - 4 + 1; // Adjust the length byte in the header
	status = ancit_uart_conn_EnqueueString(send_buffer, current_length);
	return status;
}

void ancit_dwin_tx_main(void) {
	static uint8_t last_processed_idx = 0; // Static variable to remember the last processed index

	// If timeout occurred and value is updated, set the tx_trigger to true
	if (is_timeout_expired(&dwin_cyclic_delay_ctr)) {
		for (uint8_t idx = 0; idx < DWIN_VP_MAX_ITEMS; idx++) {
			if (dwin_mem[idx].value_updated == true) {
				dwin_mem[idx].tx_triggered = true;
			}
		}
		ancit_init_timeout(&dwin_cyclic_delay_ctr, dwin_cyclic_delay_ms);
	}

	uint8_t start_idx = last_processed_idx; // Start from the last processed index
	for (uint8_t i = 0; i < DWIN_VP_MAX_ITEMS; i++) {
		uint8_t idx = (start_idx + i) % DWIN_VP_MAX_ITEMS; // Wrap around using modulus
		if (dwin_mem[idx].tx_triggered == true) {
			bool status = ancit_dwin_tx_enqueue_index(idx);
			if (status == true) {
				dwin_mem[idx].tx_triggered = false;
				dwin_mem[idx].value_updated = false;
				last_processed_idx = (idx + 1) % DWIN_VP_MAX_ITEMS; // Update last processed index to the next one
				break; // Exit the loop after the first successful enqueue
			}
		}
	}
}

#endif //#ifdef DWIN_DISPLAY_CONFIGURED
