
/* ###################################################################
**     Filename    : main.c
**     Project     : ANCIT FOTA SW
**     Processor   : S32K144
**     Version     : 0.1.2 [Hardware Major.Software Major.Software Minor]
**     			   : 0 - V2 Hardware
**     			   : 1 - Bootloader on Srec
**     Compiler    : GNU C Compiler
**     Date/Time   : 2025-09-25

**
** ###################################################################*/

/* MODULE main */

/* Including needed modules to compile this module/procedure */
#include "sdk_project_config.h"
#include "Boot_mangr.h"
//#include "Comm_mangr.h"
volatile int exit_code = 0;


int main(void)
{
  /* Write your local variable definition here */
  /* Initialize and configure clocks*/
  CLOCK_SYS_Init(g_clockManConfigsArr, CLOCK_MANAGER_CONFIG_CNT,g_clockManCallbacksArr, CLOCK_MANAGER_CALLBACK_CNT);
  CLOCK_SYS_UpdateConfiguration(0U, CLOCK_MANAGER_POLICY_AGREEMENT);

  /* Initialize pins*/
  PINS_DRV_Init(NUM_OF_CONFIGURED_PINS0, g_pin_mux_InitConfigArr0);
// Initializing the Boot manager
  Boot_manager_Init();


  for(;;)
  {



  }
  return exit_code;
} /*** End of main routine. DO NOT MODIFY THIS TEXT!!! ***/

/* END main */
/*!
 ** @}
 */
