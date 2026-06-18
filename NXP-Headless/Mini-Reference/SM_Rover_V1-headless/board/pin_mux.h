#ifndef _PIN_MUX_H_
#define _PIN_MUX_H_

#include "pins_driver.h"

/***********************************************************************************************************************
 * Definitions
 **********************************************************************************************************************/

/*!
 * @addtogroup pin_mux
 * @{
 */

/***********************************************************************************************************************
 * API
 **********************************************************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif


/*! @brief Definitions/Declarations for BOARD_InitPins Functional Group */
/*! @brief User definition pins */
#define L_BLUE_PORT    PTD
#define L_BLUE_PIN     0U
#define IMU_RST_PORT    PTB
#define IMU_RST_PIN     10U
#define FTM2_OUTPUT_CH7_PORT    PTE
#define FTM2_OUTPUT_CH7_PIN     16U
/*! @brief User number of configured pins */
#define NUM_OF_CONFIGURED_PINS0 23
/*! @brief User configuration structure */
extern pin_settings_config_t g_pin_mux_InitConfigArr0[NUM_OF_CONFIGURED_PINS0];


#if defined(__cplusplus)
}
#endif

/*!
 * @}
 */
#endif /* _PIN_MUX_H_ */

/***********************************************************************************************************************
 * EOF
 **********************************************************************************************************************/

