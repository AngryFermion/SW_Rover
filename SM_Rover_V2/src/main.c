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
#include "ancit_pwm.h"
#include "ancit_uart_rte.h"
#include "ancit_timer.h"
#include "genx_common.h"
#ifdef SIMULINK_BRIDGE_CONFIGURED
#include "genx_simulink_bridge.h"
#endif /* SIMULINK_BRIDGE_CONFIGURED */

#ifdef SCHEDULER_CONFIGURED
#include "genx_scheduler.h"
#endif //SCHEDULER_CONFIGURED

#ifdef PWM_CONFIGURED
#include "genx_pwm.h"
#endif

#ifdef I2C_MANAGER_CONFIGURED
#include "genx_i2c_manager.h"
#endif
#include "genx_ultrason.h"

volatile int exit_code = 0;

int main(void) {

/* Write your local variable definition here */

	genx_clock_init();

	/***********************************************
	 * ANCIT_CG_Init_Start
	 ***********************************************/
	//Start the base timer (One millisecond)
	ancit_base_timer_start();

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
