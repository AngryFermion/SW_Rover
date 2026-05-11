/*
 * ancit_uart_esp.h
 *
 *  Created on: 18-May-2024
 *      Author: Narayan
 */
#include "sdk_project_config.h"
#include "ancit_common.h"

#ifndef ANCIT_UART_ESP_H_
#define ANCIT_UART_ESP_H_

#define ESP_DEBUG
#ifdef ESP_DEBUG
#define ESP_UART			INST_LPUART_0
#else
#define ESP_UART			INST_LPUART_2
#endif

//UART packet tx types (FIXED)
#define UART_PKT_TX_TYPE_DISABLED		0
#define UART_PKT_TX_TYPE_ON_START		1
#define UART_PKT_TX_TYPE_ON_EVENT		2
#define UART_PKT_TX_TYPE_ON_RX_MSG		3
#define UART_PKT_TX_TYPE_CYCLIC			4

#define UART_TX_PKT_MAX					3

//Index for each packet //Only addition, no deletion or modification in ID at later stage
#define UART_TX_PKT_ID_WIFI_SETTINGS			0
#define UART_TX_PKT_ID_MQTT_ATTR				1
#define UART_TX_PKT_ID_MQTT_VALUE				2

/***********************************************
 * ANCIT_CG_Uart_Esp_Define_Start
 ***********************************************/

/***********************************************
 * ANCIT_CG_Uart_Esp_Define_End
 ***********************************************/

typedef struct {
    char param_name[64];   // Parameter name
    double value;          // Parameter value
    uint8_t decimals;      // Decimal places to be considered for the value
    uint8_t updated;	   //Set if value is updated
} uart_esp_struct_t;

// Define the states
typedef enum {
    ANCIT_UART_ESP_TX_IDLE,
    ANCIT_UART_ESP_TX_START,
    ANCIT_UART_ESP_TX_WAIT_FOR_COMPLETE,
    ANCIT_UART_ESP_TX_COMPLETE,
    ANCIT_UART_ESP_TX_DONE
} esp_uart_tx_state_t;

typedef struct
{
	uint8_t pkt_id;
	uint8_t pkt_ver;
	uint8_t pkt_type;
	uint32_t tx_freq;
} esp_uart_tx_packets_t;

void ancit_uart_esp_start(void);
void ancit_uart_esp_main(void);
void ancit_uart_esp_set_param(const char *key, double value);

#endif /* ANCIT_UART_ESP_H_ */
