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

#include <sdk_project_config.h>
#include <ancit_common.h>
#include "ancit_do.h"
#include "ancit_pwm.h"
#include "ancit_can.h"

#include "ancit_timer.h"
#include "genx_common.h"

#ifdef SIMULINK_BRIDGE_CONFIGURED
#include "genx_simulink_bridge.h"
#endif /* SIMULINK_BRIDGE_CONFIGURED */

#ifdef DIGITAL_OUTPUT_CONFIGURED
#include "genx_do.h"
#endif //DIGITAL_OUTPUT_CONFIGURED

#ifdef SCHEDULER_CONFIGURED
#include "genx_scheduler.h"
#endif //SCHEDULER_CONFIGURED

#ifdef CAN_TX_CONFIGURED
#include "genx_can_tx.h"
#endif

#ifdef CAN_RX_CONFIGURED
#include "genx_can_rx.h"
#endif

#ifdef PWM_CONFIGURED
#include "genx_pwm.h"
#endif

#ifdef I2C_MANAGER_CONFIGURED
#include "genx_i2c_manager.h"
#endif
#include "genx_ultrason.h"

#ifdef CAN_IDS_CONFIGURED
#include "genx_ids.h"
#endif

volatile int exit_code = 0;

int main(void) {

/* Write your local variable definition here */

	genx_clock_init();

	/***********************************************
	 * ANCIT_CG_Init_Start
	 ***********************************************/
	//Start the base timer (One millisecond)
	ancit_base_timer_start();

/* Initialize DIGITAL_OUTPUT_CONFIGURED */
#ifdef DIGITAL_OUTPUT_CONFIGURED
	genx_do_init();
#endif

/* Initialize canTx */
#if defined(CAN_TX_CONFIGURED)
	genx_can_tx_init();
#endif

/* Initialize canRx */
#if defined(CAN_RX_CONFIGURED)
	genx_can_rx_init();
#endif

#if defined(CAN_TX_CONFIGURED) || defined(CAN_RX_CONFIGURED)
	//Start the CAN
	ancit_can_start();
#endif //CAN_TX_CONFIGURED || CAN_RX_CONFIGURED

/* Initialize UART_CONFIGURED */
#ifdef UART_RTE_CONFIGURED
	ancit_uart_conn_start();
#endif //UART_RTE_CONFIGURED

/* Initialize PWM module */
#ifdef PWM_CONFIGURED
	genx_PWM_Init();
#endif

#ifdef ULTRASONIC_CONFIGURED
    genx_ultrason_init();
#endif

#ifdef I2C_MANAGER_CONFIGURED
	genx_i2c_manager_init();
#endif

#ifdef SCHEDULER_CONFIGURED
	genx_scheduler_init();
#endif //SCHEDULER_CONFIGURED

#ifdef CAN_IDS_CONFIGURED
	//Initialize CAN Intrusion Detection System
	genx_ids_init();
#endif //CAN_IDS_CONFIGURED

#ifdef SIMULINK_BRIDGE_CONFIGURED
	/* Initialize Simulink model */
	ANCIT_App_Init();
#endif /* SIMULINK_BRIDGE_CONFIGURED */

	/***********************************************
	 * ANCIT_CG_Init_End
	 ***********************************************/


	/* Infinite loop */
	while (1) {
		/***********************************************
		 * ANCIT_CG_Loop_Start
		 ***********************************************/
#ifdef DIGITAL_OUTPUT_CONFIGURED
	//Digital Output Main loop
	genx_do_main();
#endif

#if defined(CAN_TX_CONFIGURED)
	genx_can_tx_main();
#endif

#if defined(CAN_RX_CONFIGURED)
	genx_can_rx_main();
#endif

#ifdef I2C_MANAGER_CONFIGURED
	genx_i2c_manager_main();
#endif

#ifdef UART_RTE_CONFIGURED
ancit_uart_conn_main();
#endif //UART_CONN_CONFIGURED
#ifdef SCHEDULER_CONFIGURED
	genx_scheduler_main();
#endif
#ifdef ULTRASONIC_CONFIGURED
    genx_ultrason_main();
#endif

/* Added: drains genx_ids's pending-violation retry queue so a real violation
 * that lost the race for the shared IDS_Alert mailbox still gets sent once
 * it frees up, instead of being silently dropped. */
#ifdef CAN_IDS_CONFIGURED
	genx_ids_main();
#endif //CAN_IDS_CONFIGURED

    Task_While();

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
