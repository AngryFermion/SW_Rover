/*
 * ancit_uart_esp.c
 *
 *  Created on: 18-May-2024
 *      Author: Narayan
 */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "ancit_uart_esp.h"
#include "ancit_timer.h"

// Global state variable
esp_uart_tx_state_t current_state = ANCIT_UART_ESP_TX_IDLE;
static timeout_counter_t uart_esp_delay_ctr;
char csv_output[1024]; // Ensure this buffer is large enough to hold all concatenated strings

/***********************************************
 * ANCIT_CG_Uart_Esp_Init_Start
 ***********************************************/
// Array of structures to hold the parameters
// Limit this to 20 parameters max in the tool
#define SELECTED_PARAMS_MAX				6

uart_esp_struct_t esp_params[] = { { "PetrolLevel_ltr", 40, 0, false }, {
		"EngPower_kW", 32.1, 2, false }, { "EngForce_N", 1, 2, false }, {
		"IdleRunning_", 1, 0, false }, { "EngTemp_degC", 25, 3, false }, {
		"EngSpeed_rpm", 3500, 0, false },
// Additional entries can be initialized similarly if needed
		};

esp_uart_tx_packets_t utx_reg[] = { {
UART_TX_PKT_ID_MQTT_ATTR, 1, UART_PKT_TX_TYPE_ON_START, 0 }, {
UART_TX_PKT_ID_WIFI_SETTINGS, 1, UART_PKT_TX_TYPE_ON_START, 0 }, {
UART_TX_PKT_ID_MQTT_VALUE, 1, UART_PKT_TX_TYPE_CYCLIC, 1000 } };

/***********************************************
 * ANCIT_CG_Uart_Esp_Init_End
 ***********************************************/

void ancit_uart_esp_sm(void) {
	static uint32_t bytesRemaining;

	switch (current_state) {
	case ANCIT_UART_ESP_TX_IDLE:
		//Do nothing....
		break;

	case ANCIT_UART_ESP_TX_START:
		/* Send data via LPUART */
		LPUART_DRV_SendData(ESP_UART, (uint8_t*) csv_output,
				strlen(csv_output));

		current_state = ANCIT_UART_ESP_TX_WAIT_FOR_COMPLETE;
		break;

	case ANCIT_UART_ESP_TX_WAIT_FOR_COMPLETE:
		// Check if messsage is sent
		if (LPUART_DRV_GetTransmitStatus(ESP_UART, &bytesRemaining)
				!= STATUS_SUCCESS) {
			//wait till busy
		} else {
			current_state = ANCIT_UART_ESP_TX_COMPLETE;
		}
		break;

	case ANCIT_UART_ESP_TX_COMPLETE:
		current_state = ANCIT_UART_ESP_TX_DONE;
		break;

	case ANCIT_UART_ESP_TX_DONE:
		// Perform any cleanup or final steps
		current_state = ANCIT_UART_ESP_TX_IDLE; // Return to idle or end the process
		break;
	}
}

void ancit_uart_esp_create_csv_line(void) {
	char buffer[64];  // Buffer for individual formatted strings
	uint8_t updated_count = 0;

	csv_output[0] = '\0';  // Start with an empty string

	//Get the number of parameters updated first...
	for (int i = 0; i < SELECTED_PARAMS_MAX; i++) {
		if (esp_params[i].updated == true) {
			updated_count++;
		}
	}
	//Include the protocol header
	//- Start with !
	//- Protocol version
	//- Number of parameters to follow...
	sprintf(csv_output, "!,%d,%d,", UART_TX_PKT_ID_MQTT_VALUE, updated_count);

	for (int i = 0; i < SELECTED_PARAMS_MAX; i++) {
		//Check if the parameter has an updated value
		if (esp_params[i].updated == true) {
			//reset updated flag
			esp_params[i].updated = false;
			// Append each formatted parameter and its details to the output string
			sprintf(buffer, "%s,%.*f,", esp_params[i].param_name,
					esp_params[i].decimals, esp_params[i].value);
			strcat(csv_output, buffer); // Concatenate to the final output string
		}
	}

	//Include the protocol tail....
	strcat(csv_output, "#\r\n");
}

esp_uart_tx_state_t ancit_uart_esp_get_sm_state(void) {
	return current_state;
}

void ancit_uart_esp_set_sm_state(esp_uart_tx_state_t set_state) {
	current_state = set_state;
}

void ancit_uart_esp_start(void) {
	//Start the UART Hardware
#ifdef ESP_DEBUG
	LPUART_DRV_Init(ESP_UART, &lpUartState0, &lpuart_0_InitConfig0);
#else
	LPUART_DRV_Init(ESP_UART, &lpUartState2, &lpuart_0_InitConfig2);
#endif

	//Initialize the state machine to idle
	current_state = ANCIT_UART_ESP_TX_IDLE;

	//Initialize the wait timer timer and wait for set milliseconds...
	ancit_init_timeout(&uart_esp_delay_ctr,
			utx_reg[UART_TX_PKT_ID_MQTT_VALUE].tx_freq);
}

void ancit_uart_esp_main(void) {
	//Check if timeout has occurred for the state machine to run...
	if (is_timeout_expired(&uart_esp_delay_ctr)) {
		//Initialize the wait timer timer and wait for set milliseconds...
		ancit_init_timeout(&uart_esp_delay_ctr,
				utx_reg[UART_TX_PKT_ID_MQTT_VALUE].tx_freq);

		//If UART bus is idle
		if (ancit_uart_esp_get_sm_state() == ANCIT_UART_ESP_TX_IDLE) {
			//Create the csv line
			ancit_uart_esp_create_csv_line();
			//set the state machine to start...
			ancit_uart_esp_set_sm_state(ANCIT_UART_ESP_TX_START);
		}
	}

	//Run the state machine...
	ancit_uart_esp_sm();
}

void ancit_uart_esp_set_param(const char *key, double value) {
	// Search for the key in the existing parameters
	for (int i = 0; i < SELECTED_PARAMS_MAX; i++) {
		if (strcmp(esp_params[i].param_name, key) == 0) {
			// If key exists, update the value
			esp_params[i].value = value;
			esp_params[i].updated = true;
			return;
		}
	}
}

