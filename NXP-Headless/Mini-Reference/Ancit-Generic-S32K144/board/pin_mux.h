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
#define DO_LED_RED_PORT    PTE
#define DO_LED_RED_PIN     8U
#define DO_LED_GREEN_PORT    PTB
#define DO_LED_GREEN_PIN     5U
#define DO_LED_BLUE_PORT    PTB
#define DO_LED_BLUE_PIN     4U
#define DO_LIN_TX_PORT    PTD
#define DO_LIN_TX_PIN     7U
#define DI_LIN_RX_PORT    PTD
#define DI_LIN_RX_PIN     6U
#define DO_LIN_SLP_PORT    PTD
#define DO_LIN_SLP_PIN     5U
#define DI_KL_15_PORT    PTC
#define DI_KL_15_PIN     9U
#define DO_HS_PWR_CH1_PORT    PTD
#define DO_HS_PWR_CH1_PIN     15U
#define DO_HS_PWR_CH2_PORT    PTD
#define DO_HS_PWR_CH2_PIN     16U
#define DO_HS_PWR_CH3_PORT    PTD
#define DO_HS_PWR_CH3_PIN     1U
#define DO_HS_PWR_CH4_PORT    PTD
#define DO_HS_PWR_CH4_PIN     0U
#define DO_HS_DIA_EN_PORT    PTC
#define DO_HS_DIA_EN_PIN     1U
#define DO_HS_SEL2_PORT    PTC
#define DO_HS_SEL2_PIN     0U
#define AI_HS_SNS_1_PORT    PTC
#define AI_HS_SNS_1_PIN     15U
#define AI_HS_SNS_2_PORT    PTC
#define AI_HS_SNS_2_PIN     14U
#define PWM_AI_CH1_PORT    PTE
#define PWM_AI_CH1_PIN     4U
#define PWM_AI_CH2_PORT    PTE
#define PWM_AI_CH2_PIN     5U
#define PWM_AI_CH3_PORT    PTE
#define PWM_AI_CH3_PIN     10U
#define PWM_AI_CH4_PORT    PTE
#define PWM_AI_CH4_PIN     11U
#define DI_PD_CH1_PORT    PTD
#define DI_PD_CH1_PIN     3U
#define DI_PD_CH2_PORT    PTA
#define DI_PD_CH2_PIN     1U
#define DI_PD_CH3_PORT    PTE
#define DI_PD_CH3_PIN     6U
#define DI_PD_CH4_PORT    PTE
#define DI_PD_CH4_PIN     2U
#define DI_PD_CH5_PORT    PTD
#define DI_PD_CH5_PIN     2U
#define DI_PD_CH6_PORT    PTD
#define DI_PD_CH6_PIN     4U
#define DI_PD_CH7_PORT    PTB
#define DI_PD_CH7_PIN     13U
#define DI_PD_CH8_PORT    PTB
#define DI_PD_CH8_PIN     12U
/*! @brief User number of configured pins */
#define NUM_OF_CONFIGURED_PINS0 35
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

