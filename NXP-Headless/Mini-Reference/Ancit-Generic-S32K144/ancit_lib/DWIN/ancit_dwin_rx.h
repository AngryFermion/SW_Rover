/*
 * ancit_uart_conn_rx.h
 *
 *  Created on: 09-Jun-2024
 *      Author: Narayan
 */

#ifndef ANCIT_DWIN_RX_H_
#define ANCIT_DWIN_RX_H_
#include "genx_config.h"

#ifdef DWIN_DISPLAY_CONFIGURED
#include "ancit_dwin.h"

#define MAX_RX_BUFFER_SIZE 		64
#define DWIN_RX_TIMEOUT_MS 		10

#define DWIN_RX_IDX_HEADER1		0
#define DWIN_RX_IDX_HEADER2		1
#define DWIN_RX_IDX_LEN			2
#define DWIN_RX_IDX_CMD			3
#define DWIN_RX_IDX_ADDR1		4
#define DWIN_RX_IDX_ADDR2		5
#define DWIN_RX_IDX_WORD_COUNT	6
#define DWIN_RX_IDX_DATA_START	7

typedef enum {
	UART_RX_INIT,
	UART_RX_WAIT_FOR_RX_START,
	UART_RX_WAIT_FOR_HEADER,
	UART_RX_WAIT_FOR_DATA,
	UART_RX_PROCESS_DATA,
	UART_RX_DONE
} UARTState;

typedef struct {
	uint8_t errCounter;
	uint8_t buff[MAX_RX_BUFFER_SIZE];
	bool rx_ok;
	uint8_t sm_state;
	uint32_t timer;
	bool timer_started;
} dwin_display_rx_t;

void ancit_dwin_rx_sm(void);

void ancit_dwin_rx_start(void);
void ancit_dwin_rx_main(void);
void ancit_dwin_rx_set_value_by_vp_addr_array(uint16_t vp_addr, uint8_t *data);

#endif //DWIN_DISPLAY_CONFIGURED
#endif /* ANCIT_DWIN_RX_H_ */
