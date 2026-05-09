/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 * NXP Confidential. This software is owned or controlled by NXP and may only be
 * used strictly in accordance with the applicable license terms. By expressly
 * accepting such terms or by downloading, installing, activating and/or otherwise
 * using the software, you are agreeing that you have read, and that you agree to
 * comply with and are bound by, such license terms. If you do not agree to be
 * bound by the applicable license terms, then you may not retain, install,
 * activate or otherwise use the software. The production use license in
 * Section 2.3 is expressly granted for this software.
 */
/* ###################################################################
 **     Filename    : main.c
 **     Project     : ANCIT_SmartWheelsV2_TemplateProject
 **     Processor   : S32K144
 **     Abstract    :
 **         Main module.
 **         This module contains user's application code.
 **     Settings    :
 **     Contents    :
 **         No public methods
 **
 ** ###################################################################*/
/*!
 ** @file main.c
 ** @brief
 **         Main module.
 **         This module contains user's application code.
 */
/* Including necessary configuration files. */
#include "sdk_project_config.h"
#include "ancit_common.h"
#include "ancit_can.h"
#include "ancit_dwin_tx.h"
#include "ancit_dwin_rx.h"
#include "ancit_uart_conn.h"
#include "ancit_timer.h"
#include "genx_common.h"

#include "fota_uart.h"


#ifdef DIGITAL_INPUT_CONFIGURED
#include "genx_di.h"
#endif

#ifdef DIGITAL_OUTPUT_CONFIGURED
#include "genx_do.h"
#endif //DIGITAL_OUTPUT_CONFIGURED

#ifdef ADC_CONFIGURED
#include "genx_adc.h"
#endif

#ifdef SCHEDULER_CONFIGURED
#include "genx_scheduler.h"
#endif //SCHEDULER_CONFIGURED

#ifdef CAN_TX_CONFIGURED
#include "genx_can_tx.h"
#endif

#ifdef CAN_RX_CONFIGURED
#include "genx_can_rx.h"
#endif

#ifdef HEAD_LIGHT_CONFIGURED
#include "ancit_head_light.h"
#endif

//Narayan - TEST
//void hbridge_test(void);

volatile int exit_code = 0;

int main(void) {
	/* Write your local variable definition here */

	genx_clock_init();

	/***********************************************
	 * ANCIT_CG_Init_Start
	 ***********************************************/
#ifdef DIGITAL_OUTPUT_CONFIGURED
	genx_do_init();
#endif //DIGITAL_OUTPUT_CONFIGURED

	//Start the base timer (One millisecond)
	ancit_base_timer_start();

#ifdef DIGITAL_INPUT_CONFIGURED
	//Start the switch inputs
	genx_di_init();
#endif //DIGITAL_INPUT_CONFIGURED

#if defined(CAN_TX_CONFIGURED) || defined(CAN_RX_CONFIGURED)
	//Start the CAN
	ancit_can_start();
#endif //CAN_TX_CONFIGURED || CAN_RX_CONFIGURED

#if defined(CAN_TX_CONFIGURED)
	genx_can_tx_init();
#endif

#if defined(CAN_RX_CONFIGURED)
	genx_can_rx_init();
#endif

#ifdef ADC_CONFIGURED
	//Start the switch inputs
	genx_adc_init();
#endif //ADC_CONFIGURED

#ifdef UART_CONN_CONFIGURED
	ancit_uart_conn_start();
#endif //UART_CONN_CONFIGURED

	//Start the ESP UART
	//ancit_uart_esp_start();
#ifdef DWIN_DISPLAY_CONFIGURED
	ancit_dwin_rx_start();
#endif //DWIN_DISPLAY_CONFIGURED

#ifdef NVM_EEPROM_CONFIGURED
//	ancit_eeprom_init();
#endif

#ifdef HEAD_LIGHT_CONFIGURED
	ancit_hl_pwm_start();
#endif
//	hbridge_test();

#ifdef SCHEDULER_CONFIGURED
	genx_scheduler_init();
#endif //SCHEDULER_CONFIGURED

	fota_uart_init();
	Uds_NvmMgr_init();
	UDS_BootManager_init();
	/***********************************************
	 * ANCIT_CG_Init_End
	 ***********************************************/
	/* Infinite loop */
	while (1) {
		/***********************************************
		 * ANCIT_CG_Loop_Start
		 ***********************************************/
#ifdef DIGITAL_INPUT_CONFIGURED
		//Digital Input Main loop
		genx_di_main();
#endif

#ifdef ADC_CONFIGURED
		//ADC Main Lopp
		genx_adc_main();
#endif

#if defined(CAN_RX_CONFIGURED)
		genx_can_rx_main();
#endif

#if defined(CAN_TX_CONFIGURED)
		genx_can_tx_main();
#endif

#ifdef DIGITAL_OUTPUT_CONFIGURED
		genx_do_main();
#endif //DIGITAL_OUTPUT_CONFIGURED
		//ESP UART main routine
//		ancit_uart_esp_main();

#ifdef UART_CONN_CONFIGURED
		ancit_uart_conn_main();
#endif //UART_CONN_CONFIGURED

#ifdef SCHEDULER_CONFIGURED
		genx_scheduler_main();
#endif //SCHEDULER_CONFIGURED

#ifdef NVM_EEPROM_CONFIGURED
//		ancit_eeprom_main();
#endif
		fota_uart_rx_sm();
		fota_uart_tx_sm();
		Uds_NvmMgr_sm();
		UDS_BootManager_sm();
		//Toggle the Debug Pin PTD8 to indicate end of main loop
		//Toggles every main loop execution
//		ancit_digital_output_toggle(DO_PTD8_DEBUG_IDX);
		/***********************************************
		 * ANCIT_CG_Loop_End
		 ***********************************************/
	}
	return exit_code;
}

/* END main */
/*!
 ** @}
 */
