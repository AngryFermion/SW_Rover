/*
 * ancit_uart_esp.h
 *
 *  Created on: 18-May-2024
 *      Author: Narayan
 */
#include "genx_config.h"

#ifndef ANCIT_UART_CONN_H_
#define ANCIT_UART_CONN_H_

#define EXT_UART 		INST_LPUART_1

#ifdef UART_RTE_CONFIGURED

#define MAX_STRING_LENGTH 			100
#define QUEUE_SIZE 					16

#define MAX_RX_CMD_LENGTH			64
#define RX_CMD_DELIMITER			'\n'

//Index for each packet //Only addition, no deletion or modification in ID at later stage
#define UART_TX_PKT_ID_WIFI_SETTINGS			0
#define UART_TX_PKT_ID_MQTT_ATTR				1
#define UART_TX_PKT_ID_MQTT_VALUE				2

/***********************************************
 * ANCIT_CG_Uart_Ext_Define_Start
 ***********************************************/
//GENERATE UART COMMAND DEFINES
/***********************************************
 * ANCIT_CG_Uart_Ext_Define_End
 ***********************************************/

// TX state machine
typedef enum {
    ANCIT_UART_CONN_TX_IDLE,
    ANCIT_UART_CONN_TX_START,
    ANCIT_UART_CONN_TX_WAIT_FOR_COMPLETE,
    ANCIT_UART_CONN_TX_COMPLETE,
    ANCIT_UART_CONN_TX_DONE
} ext_uart_tx_state_t;

typedef struct {
    char buffer[QUEUE_SIZE][MAX_STRING_LENGTH];
    uint8_t length[QUEUE_SIZE];
    uint8_t head;
    uint8_t tail;
    uint8_t count;
} StringQueue;

// TX API
void ancit_uart_conn_start(void);
void ancit_uart_conn_main(void);
bool ancit_uart_conn_EnqueueString(const char* str, uint8_t length);
void ancit_uart_conn_DequeueString(char* str, uint8_t *length);
bool ancit_uart_conn_IsQueueEmpty(void);

// RX API
void ancit_uart_rx_byte_cb(void *driverState, uart_event_t event, void *param);
void ancit_uart_cmd_dispatch(const char *cmd);

#endif //UART_RTE_CONFIGURED
#endif /* ANCIT_UART_CONN_H_ */
