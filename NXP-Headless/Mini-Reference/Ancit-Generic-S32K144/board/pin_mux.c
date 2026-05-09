/* clang-format off */
/*
 * TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
!!GlobalInfo
product: Pins v15.0
processor: S32K144
package_id: S32K144_LQFP100
mcu_data: s32sdk_s32k1xx_rtm_401
processor_version: 0.0.0
external_user_signals: {}
pin_labels:
- {pin_num: '59', pin_signal: PTE7, label: LED_GREEN, identifier: LED_GREEN}
- {pin_num: '58', pin_signal: PTA6, label: LED_RED, identifier: LED_RED}
- {pin_num: '57', pin_signal: PTA7, label: LED_BLUE, identifier: LED_BLUE}
- {pin_num: '73', pin_signal: PTA2, label: LPUART0_RX}
- {pin_num: '72', pin_signal: PTA3, label: LPUART0_TX}
- {pin_num: '56', pin_signal: PTC8, label: LPUART1_LIN_RX}
- {pin_num: '55', pin_signal: PTC9, label: KL_15_IN, identifier: KL_15_IN;DI_KL_15_IN;DI_KL_15}
- {pin_num: '31', pin_signal: PTD7, label: LIN_TX, identifier: DO_LIN_TX}
- {pin_num: '100', pin_signal: PTA8, label: WIFI_RX}
- {pin_num: '99', pin_signal: PTA9, label: WIFI_TX}
- {pin_num: '1', pin_signal: PTE16, label: FTM2_OUTPUT_CH7, identifier: FTM2_OUTPUT_CH7}
- {pin_num: '2', pin_signal: PTE15, label: FTM2_OUTPUT_CH6}
- {pin_num: '62', pin_signal: PTA17, label: WIFI_RST}
- {pin_num: '75', pin_signal: PTB10, label: KL15_MCU}
- {pin_num: '52', pin_signal: PTC10, label: KL15_MCU}
- {pin_num: '51', pin_signal: PTC11, label: 12V_DIG_IP2}
- {pin_num: '50', pin_signal: PTC12, label: DIG_IP1}
- {pin_num: '49', pin_signal: PTC13, label: DIG_IP2}
- {pin_num: '6', pin_signal: PTE10, label: S2_IO3, identifier: HS_DO_CH3;DO_HS_CH3;PWM_AI_CH3}
- {pin_num: '7', pin_signal: PTE13, label: RELAY1}
- {pin_num: '42', pin_signal: PTD8, label: DEBUG_PIN}
- {pin_num: '32', pin_signal: PTD6, label: LIN_RX, identifier: DI_LIN_RX}
- {pin_num: '36', pin_signal: PTD10, label: SPARE2}
- {pin_num: '34', pin_signal: PTD12, label: SPARE3}
- {pin_num: '8', pin_signal: PTE5, label: S2_IO2, identifier: HS_DO_CH2;DO_HS_CH2;PWM_AI_CH2}
- {pin_num: '9', pin_signal: PTE4, label: S2_IO1, identifier: HS_DO_CH1;DO_HS_CH1;PWM_AI_CH1}
- {pin_num: '80', pin_signal: PTC7, label: LPUART1_TX}
- {pin_num: '81', pin_signal: PTC6, label: LPUART1_RX}
- {pin_num: '26', pin_signal: PTE8, label: LED_RED, identifier: DO_LED_RED}
- {pin_num: '27', pin_signal: PTB5, label: LED_GREEN, identifier: DO_LED_GREEN}
- {pin_num: '28', pin_signal: PTB4, label: LED_BLUE, identifier: DO_LED_BLUE}
- {pin_num: '29', pin_signal: PTC3, label: CAN0_TX}
- {pin_num: '30', pin_signal: PTC2, label: CAN0_RX}
- {pin_num: '43', pin_signal: PTC17, label: CAN2_TX}
- {pin_num: '44', pin_signal: PTC16, label: CAN2_RX}
- {pin_num: '33', pin_signal: PTD5, label: LIN_SLP, identifier: DO_LIN_SLP}
- {pin_num: '22', pin_signal: PTD15, label: S3_IO2, identifier: DI_CH1;DI_PD_CH1;HALFB_CH1;DO_HALFB_CH1;DO_HALFB_CH2;DO_HS_PWR_CH2;DO_HS_PWR_CH1}
- {pin_num: '21', pin_signal: PTD16, label: S3_IO1, identifier: DI_PD_CH2;HALFB_CH2;DO_HALFB_CH2;DO_HALFB_CH1;DO_HS_PWR_CH1;DO_HS_PWR_CH2}
- {pin_num: '3', pin_signal: PTD1, label: S3_IO4, identifier: DI_PD_CH3;HALFB_CH3;DO_HALFB_CH3;DO_HALFB_CH4;DO_HS_PWR_CH4;DO_HS_PWR_CH3;DO_HS_DIA_EN1;DO_HS_DIA_EN}
- {pin_num: '4', pin_signal: PTD0, label: S3_IO3, identifier: DI_PD_CH4;HALFB_CH4;DO_HALFB_CH4;DO_HALFB_CH3;DO_HS_PWR_CH3;DO_HS_PWR_CH4}
- {pin_num: '39', pin_signal: PTC1, label: S3_IO5, identifier: DI_PD_CH5;IPROPI_HB1;AI_IPROPI_HB1;DO_HS_DIA_EN}
- {pin_num: '40', pin_signal: PTC0, label: S3_IO6, identifier: DI_PD_CH6;IPROPI_HB2;AI_IPROPI_HB2;DO_HS_SEL2}
- {pin_num: '45', pin_signal: PTC15, label: S3_IO7, identifier: DI_PD_CH7;HALFB_CH1;SLEEP_HB;DI_SLEEP_HB;DI_HB_SLEEP;DO_HB_SLEEP;AI_SNS;AI_HS1_SNS;AI_HS_SNS1;AI_HS_SNS_1}
- {pin_num: '46', pin_signal: PTC14, label: S3_IO8, identifier: DI_PD_CH8;FAULT_HB;DO_FAULT_HB;DO_HB_FAULT;DI_HB_FAULT;AI_HS2_SNS;AI_HS_SNS2;AI_HS_SNS_2}
- {pin_num: '5', pin_signal: PTE11, label: S2_IO4, identifier: HS_DO_CH4;DO_HS_CH4;PWM_AI_CH4}
- {pin_num: '53', pin_signal: PTB1, label: S2_IO5}
- {pin_num: '47', pin_signal: PTB3, label: S2_IO6}
- {pin_num: '48', pin_signal: PTB2, label: S2_IO7}
- {pin_num: '54', pin_signal: PTB0, label: S2_IO8}
- {pin_num: '70', pin_signal: PTD3, label: S1_IO1, identifier: DI_PD_CH1}
- {pin_num: '78', pin_signal: PTA1, label: S1_IO2, identifier: DI_PD_CH2}
- {pin_num: '84', pin_signal: PTE6, label: S1_IO3, identifier: DI_PD_CH3}
- {pin_num: '85', pin_signal: PTE2, label: S1_IO4, identifier: DI_PD_CH4}
- {pin_num: '71', pin_signal: PTD2, label: S1_IO5, identifier: DI_PD_CH5}
- {pin_num: '69', pin_signal: PTD4, label: S1_IO6, identifier: DI_PD_CH6}
- {pin_num: '67', pin_signal: PTB13, label: S1_IO7, identifier: DI_PD_CH7}
- {pin_num: '68', pin_signal: PTB12, label: S1_IO8, identifier: DI_PD_CH8;AI_PD_CH8}
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS ***********
 */
/* clang-format on */

/**
 * @page misra_violations MISRA-C:2012 violations
 *
 * @section [global]
 * Violates MISRA 2012 Advisory Rule 8.7, External variable could be made static.
 * The external variables will be used in other source files in application code.
 *
 * @section [global]
 * Violates MISRA 2012 Advisory Rule 11.4, Conversion between a pointer and integer type.
 * The cast is required to initialize a pointer with an unsigned long define, representing an address.
 *
 * @section [global]
 * Violates MISRA 2012 Required Rule 11.6, Cast from unsigned int to pointer.
 * The cast is required to initialize a pointer with an unsigned long define, representing an address.
 *
 */

#include "pin_mux.h"

/* clang-format off */
/*
 * TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
BOARD_InitPins:
- options: {callFromInitBoot: 'true', coreID: core0}
- pin_list:
  - {pin_num: '26', peripheral: PORTE, signal: 'port, 8', pin_signal: PTE8, direction: OUTPUT, initValue: state_1}
  - {pin_num: '27', peripheral: PORTB, signal: 'port, 5', pin_signal: PTB5, direction: OUTPUT, initValue: state_1}
  - {pin_num: '28', peripheral: PORTB, signal: 'port, 4', pin_signal: PTB4, direction: OUTPUT, initValue: state_1}
  - {pin_num: '29', peripheral: CAN0, signal: 'txd, txd', pin_signal: PTC3}
  - {pin_num: '30', peripheral: CAN0, signal: 'rxd, rxd', pin_signal: PTC2}
  - {pin_num: '43', peripheral: CAN2, signal: 'txd, txd', pin_signal: PTC17}
  - {pin_num: '44', peripheral: CAN2, signal: 'rxd, rxd', pin_signal: PTC16}
  - {pin_num: '31', peripheral: LPUART2, signal: txd, pin_signal: PTD7, direction: OUTPUT}
  - {pin_num: '32', peripheral: LPUART2, signal: rxd, pin_signal: PTD6}
  - {pin_num: '33', peripheral: PORTD, signal: 'port, 5', pin_signal: PTD5, direction: OUTPUT}
  - {pin_num: '55', peripheral: PORTC, signal: 'port, 9', pin_signal: PTC9, identifier: DI_KL_15, direction: INPUT}
  - {pin_num: '73', peripheral: LPUART0, signal: rxd, pin_signal: PTA2}
  - {pin_num: '72', peripheral: LPUART0, signal: txd, pin_signal: PTA3}
  - {pin_num: '22', peripheral: PORTD, signal: 'port, 15', pin_signal: PTD15, identifier: DO_HS_PWR_CH1, direction: OUTPUT}
  - {pin_num: '21', peripheral: PORTD, signal: 'port, 16', pin_signal: PTD16, identifier: DO_HS_PWR_CH2, direction: OUTPUT}
  - {pin_num: '3', peripheral: PORTD, signal: 'port, 1', pin_signal: PTD1, identifier: DO_HS_PWR_CH3, direction: OUTPUT}
  - {pin_num: '4', peripheral: PORTD, signal: 'port, 0', pin_signal: PTD0, identifier: DO_HS_PWR_CH4, direction: OUTPUT}
  - {pin_num: '39', peripheral: PORTC, signal: 'port, 1', pin_signal: PTC1, identifier: DO_HS_DIA_EN, direction: OUTPUT, initValue: state_0}
  - {pin_num: '40', peripheral: PORTC, signal: 'port, 0', pin_signal: PTC0, identifier: DO_HS_SEL2, direction: OUTPUT, initValue: state_0}
  - {pin_num: '45', peripheral: ADC0, signal: 'se, 13', pin_signal: PTC15, identifier: AI_HS_SNS_1}
  - {pin_num: '46', peripheral: ADC0, signal: 'se, 12', pin_signal: PTC14, identifier: AI_HS_SNS_2}
  - {pin_num: '9', peripheral: FTM2, signal: 'ch, 2', pin_signal: PTE4, identifier: PWM_AI_CH1, direction: OUTPUT}
  - {pin_num: '8', peripheral: FTM2, signal: 'ch, 3', pin_signal: PTE5, identifier: PWM_AI_CH2, direction: OUTPUT}
  - {pin_num: '6', peripheral: FTM2, signal: 'ch, 4', pin_signal: PTE10, identifier: PWM_AI_CH3, direction: OUTPUT}
  - {pin_num: '5', peripheral: FTM2, signal: 'ch, 5', pin_signal: PTE11, identifier: PWM_AI_CH4, direction: OUTPUT}
  - {pin_num: '53', peripheral: PORTB, signal: 'port, 1', pin_signal: PTB1, direction: INPUT}
  - {pin_num: '47', peripheral: PORTB, signal: 'port, 3', pin_signal: PTB3, direction: INPUT}
  - {pin_num: '48', peripheral: PORTB, signal: 'port, 2', pin_signal: PTB2, direction: INPUT}
  - {pin_num: '54', peripheral: PORTB, signal: 'port, 0', pin_signal: PTB0, direction: INPUT}
  - {pin_num: '70', peripheral: PORTD, signal: 'port, 3', pin_signal: PTD3, direction: INPUT}
  - {pin_num: '78', peripheral: PORTA, signal: 'port, 1', pin_signal: PTA1, direction: INPUT}
  - {pin_num: '84', peripheral: PORTE, signal: 'port, 6', pin_signal: PTE6, direction: INPUT}
  - {pin_num: '85', peripheral: PORTE, signal: 'port, 2', pin_signal: PTE2, direction: INPUT}
  - {pin_num: '71', peripheral: PORTD, signal: 'port, 2', pin_signal: PTD2, direction: INPUT}
  - {pin_num: '69', peripheral: PORTD, signal: 'port, 4', pin_signal: PTD4, direction: INPUT}
  - {pin_num: '67', peripheral: PORTB, signal: 'port, 13', pin_signal: PTB13, direction: INPUT}
  - {pin_num: '68', peripheral: ADC1, signal: 'se, 7', pin_signal: PTB12, identifier: DI_PD_CH8}
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS ***********
 */
/* clang-format on */

/* Generate array of configured pin structures */
pin_settings_config_t g_pin_mux_InitConfigArr0[NUM_OF_CONFIGURED_PINS0] = {
    {
        .base            = PORTA,
        .pinPortIdx      = 1U,
        .pullConfig      = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveSelect     = PORT_LOW_DRIVE_STRENGTH,
        .passiveFilter   = false,
        .mux             = PORT_MUX_AS_GPIO,
        .pinLock         = false,
        .intConfig       = PORT_DMA_INT_DISABLED,
        .clearIntFlag    = false,
        .gpioBase        = PTA,
        .direction       = GPIO_INPUT_DIRECTION,
        .digitalFilter   = false,
        .initValue       = 0U,
    },
    {
        .base            = PORTA,
        .pinPortIdx      = 2U,
        .pullConfig      = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveSelect     = PORT_LOW_DRIVE_STRENGTH,
        .passiveFilter   = false,
        .mux             = PORT_MUX_ALT6,
        .pinLock         = false,
        .intConfig       = PORT_DMA_INT_DISABLED,
        .clearIntFlag    = false,
        .gpioBase        = NULL,
        .digitalFilter   = false,
    },
    {
        .base            = PORTA,
        .pinPortIdx      = 3U,
        .pullConfig      = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveSelect     = PORT_LOW_DRIVE_STRENGTH,
        .passiveFilter   = false,
        .mux             = PORT_MUX_ALT6,
        .pinLock         = false,
        .intConfig       = PORT_DMA_INT_DISABLED,
        .clearIntFlag    = false,
        .gpioBase        = NULL,
        .digitalFilter   = false,
    },
    {
        .base            = PORTB,
        .pinPortIdx      = 0U,
        .pullConfig      = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveSelect     = PORT_LOW_DRIVE_STRENGTH,
        .passiveFilter   = false,
        .mux             = PORT_MUX_AS_GPIO,
        .pinLock         = false,
        .intConfig       = PORT_DMA_INT_DISABLED,
        .clearIntFlag    = false,
        .gpioBase        = PTB,
        .direction       = GPIO_INPUT_DIRECTION,
        .digitalFilter   = false,
        .initValue       = 0U,
    },
    {
        .base            = PORTB,
        .pinPortIdx      = 1U,
        .pullConfig      = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveSelect     = PORT_LOW_DRIVE_STRENGTH,
        .passiveFilter   = false,
        .mux             = PORT_MUX_AS_GPIO,
        .pinLock         = false,
        .intConfig       = PORT_DMA_INT_DISABLED,
        .clearIntFlag    = false,
        .gpioBase        = PTB,
        .direction       = GPIO_INPUT_DIRECTION,
        .digitalFilter   = false,
        .initValue       = 0U,
    },
    {
        .base            = PORTB,
        .pinPortIdx      = 12U,
        .pullConfig      = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveSelect     = PORT_LOW_DRIVE_STRENGTH,
        .passiveFilter   = false,
        .mux             = PORT_PIN_DISABLED,
        .pinLock         = false,
        .intConfig       = PORT_DMA_INT_DISABLED,
        .clearIntFlag    = false,
        .gpioBase        = NULL,
        .digitalFilter   = false,
    },
    {
        .base            = PORTB,
        .pinPortIdx      = 13U,
        .pullConfig      = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveSelect     = PORT_LOW_DRIVE_STRENGTH,
        .passiveFilter   = false,
        .mux             = PORT_MUX_AS_GPIO,
        .pinLock         = false,
        .intConfig       = PORT_DMA_INT_DISABLED,
        .clearIntFlag    = false,
        .gpioBase        = PTB,
        .direction       = GPIO_INPUT_DIRECTION,
        .digitalFilter   = false,
        .initValue       = 0U,
    },
    {
        .base            = PORTB,
        .pinPortIdx      = 2U,
        .pullConfig      = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveSelect     = PORT_LOW_DRIVE_STRENGTH,
        .passiveFilter   = false,
        .mux             = PORT_MUX_AS_GPIO,
        .pinLock         = false,
        .intConfig       = PORT_DMA_INT_DISABLED,
        .clearIntFlag    = false,
        .gpioBase        = PTB,
        .direction       = GPIO_INPUT_DIRECTION,
        .digitalFilter   = false,
        .initValue       = 0U,
    },
    {
        .base            = PORTB,
        .pinPortIdx      = 3U,
        .pullConfig      = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveSelect     = PORT_LOW_DRIVE_STRENGTH,
        .passiveFilter   = false,
        .mux             = PORT_MUX_AS_GPIO,
        .pinLock         = false,
        .intConfig       = PORT_DMA_INT_DISABLED,
        .clearIntFlag    = false,
        .gpioBase        = PTB,
        .direction       = GPIO_INPUT_DIRECTION,
        .digitalFilter   = false,
        .initValue       = 0U,
    },
    {
        .base            = PORTB,
        .pinPortIdx      = 4U,
        .pullConfig      = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveSelect     = PORT_LOW_DRIVE_STRENGTH,
        .passiveFilter   = false,
        .mux             = PORT_MUX_AS_GPIO,
        .pinLock         = false,
        .intConfig       = PORT_DMA_INT_DISABLED,
        .clearIntFlag    = false,
        .gpioBase        = PTB,
        .direction       = GPIO_OUTPUT_DIRECTION,
        .digitalFilter   = false,
        .initValue       = 1U,
    },
    {
        .base            = PORTB,
        .pinPortIdx      = 5U,
        .pullConfig      = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveSelect     = PORT_LOW_DRIVE_STRENGTH,
        .passiveFilter   = false,
        .mux             = PORT_MUX_AS_GPIO,
        .pinLock         = false,
        .intConfig       = PORT_DMA_INT_DISABLED,
        .clearIntFlag    = false,
        .gpioBase        = PTB,
        .direction       = GPIO_OUTPUT_DIRECTION,
        .digitalFilter   = false,
        .initValue       = 1U,
    },
    {
        .base            = PORTC,
        .pinPortIdx      = 0U,
        .pullConfig      = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveSelect     = PORT_LOW_DRIVE_STRENGTH,
        .passiveFilter   = false,
        .mux             = PORT_MUX_AS_GPIO,
        .pinLock         = false,
        .intConfig       = PORT_DMA_INT_DISABLED,
        .clearIntFlag    = false,
        .gpioBase        = PTC,
        .direction       = GPIO_OUTPUT_DIRECTION,
        .digitalFilter   = false,
        .initValue       = 0U,
    },
    {
        .base            = PORTC,
        .pinPortIdx      = 1U,
        .pullConfig      = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveSelect     = PORT_LOW_DRIVE_STRENGTH,
        .passiveFilter   = false,
        .mux             = PORT_MUX_AS_GPIO,
        .pinLock         = false,
        .intConfig       = PORT_DMA_INT_DISABLED,
        .clearIntFlag    = false,
        .gpioBase        = PTC,
        .direction       = GPIO_OUTPUT_DIRECTION,
        .digitalFilter   = false,
        .initValue       = 0U,
    },
    {
        .base            = PORTC,
        .pinPortIdx      = 14U,
        .pullConfig      = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveSelect     = PORT_LOW_DRIVE_STRENGTH,
        .passiveFilter   = false,
        .mux             = PORT_PIN_DISABLED,
        .pinLock         = false,
        .intConfig       = PORT_DMA_INT_DISABLED,
        .clearIntFlag    = false,
        .gpioBase        = NULL,
        .digitalFilter   = false,
    },
    {
        .base            = PORTC,
        .pinPortIdx      = 15U,
        .pullConfig      = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveSelect     = PORT_LOW_DRIVE_STRENGTH,
        .passiveFilter   = false,
        .mux             = PORT_PIN_DISABLED,
        .pinLock         = false,
        .intConfig       = PORT_DMA_INT_DISABLED,
        .clearIntFlag    = false,
        .gpioBase        = NULL,
        .digitalFilter   = false,
    },
    {
        .base            = PORTC,
        .pinPortIdx      = 16U,
        .pullConfig      = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveSelect     = PORT_LOW_DRIVE_STRENGTH,
        .passiveFilter   = false,
        .mux             = PORT_MUX_ALT3,
        .pinLock         = false,
        .intConfig       = PORT_DMA_INT_DISABLED,
        .clearIntFlag    = false,
        .gpioBase        = NULL,
        .digitalFilter   = false,
    },
    {
        .base            = PORTC,
        .pinPortIdx      = 17U,
        .pullConfig      = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveSelect     = PORT_LOW_DRIVE_STRENGTH,
        .passiveFilter   = false,
        .mux             = PORT_MUX_ALT3,
        .pinLock         = false,
        .intConfig       = PORT_DMA_INT_DISABLED,
        .clearIntFlag    = false,
        .gpioBase        = NULL,
        .digitalFilter   = false,
    },
    {
        .base            = PORTC,
        .pinPortIdx      = 2U,
        .pullConfig      = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveSelect     = PORT_LOW_DRIVE_STRENGTH,
        .passiveFilter   = false,
        .mux             = PORT_MUX_ALT3,
        .pinLock         = false,
        .intConfig       = PORT_DMA_INT_DISABLED,
        .clearIntFlag    = false,
        .gpioBase        = NULL,
        .digitalFilter   = false,
    },
    {
        .base            = PORTC,
        .pinPortIdx      = 3U,
        .pullConfig      = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveSelect     = PORT_LOW_DRIVE_STRENGTH,
        .passiveFilter   = false,
        .mux             = PORT_MUX_ALT3,
        .pinLock         = false,
        .intConfig       = PORT_DMA_INT_DISABLED,
        .clearIntFlag    = false,
        .gpioBase        = NULL,
        .digitalFilter   = false,
    },
    {
        .base            = PORTC,
        .pinPortIdx      = 9U,
        .pullConfig      = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveSelect     = PORT_LOW_DRIVE_STRENGTH,
        .passiveFilter   = false,
        .mux             = PORT_MUX_AS_GPIO,
        .pinLock         = false,
        .intConfig       = PORT_DMA_INT_DISABLED,
        .clearIntFlag    = false,
        .gpioBase        = PTC,
        .direction       = GPIO_INPUT_DIRECTION,
        .digitalFilter   = false,
        .initValue       = 0U,
    },
    // {
    //     .base            = PORTD,
    //     .pinPortIdx      = 0U,
    //     .pullConfig      = PORT_INTERNAL_PULL_NOT_ENABLED,
    //     .driveSelect     = PORT_LOW_DRIVE_STRENGTH,
    //     .passiveFilter   = false,
    //     .mux             = PORT_MUX_AS_GPIO,
    //     .pinLock         = false,
    //     .intConfig       = PORT_DMA_INT_DISABLED,
    //     .clearIntFlag    = false,
    //     .gpioBase        = PTD,
    //     .direction       = GPIO_OUTPUT_DIRECTION,
    //     .digitalFilter   = false,
    //     .initValue       = 0U,
    // },
    {
        .base            = PORTD,
        .pinPortIdx      = 1U,
        .pullConfig      = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveSelect     = PORT_LOW_DRIVE_STRENGTH,
        .passiveFilter   = false,
        .mux             = PORT_MUX_AS_GPIO,
        .pinLock         = false,
        .intConfig       = PORT_DMA_INT_DISABLED,
        .clearIntFlag    = false,
        .gpioBase        = PTD,
        .direction       = GPIO_OUTPUT_DIRECTION,
        .digitalFilter   = false,
        .initValue       = 0U,
    },
    {
        .base            = PORTD,
        .pinPortIdx      = 15U,
        .pullConfig      = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveSelect     = PORT_LOW_DRIVE_STRENGTH,
        .passiveFilter   = false,
        .mux             = PORT_MUX_AS_GPIO,
        .pinLock         = false,
        .intConfig       = PORT_DMA_INT_DISABLED,
        .clearIntFlag    = false,
        .gpioBase        = PTD,
        .direction       = GPIO_OUTPUT_DIRECTION,
        .digitalFilter   = false,
        .initValue       = 1U,
    },
    // {
    //     .base            = PORTD,
    //     .pinPortIdx      = 16U,
    //     .pullConfig      = PORT_INTERNAL_PULL_NOT_ENABLED,
    //     .driveSelect     = PORT_LOW_DRIVE_STRENGTH,
    //     .passiveFilter   = false,
    //     .mux             = PORT_MUX_AS_GPIO,
    //     .pinLock         = false,
    //     .intConfig       = PORT_DMA_INT_DISABLED,
    //     .clearIntFlag    = false,
    //     .gpioBase        = PTD,
    //     .direction       = GPIO_OUTPUT_DIRECTION,
    //     .digitalFilter   = false,
    //     .initValue       = 0U,
    // },
    {
        .base            = PORTD,
        .pinPortIdx      = 2U,
        .pullConfig      = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveSelect     = PORT_LOW_DRIVE_STRENGTH,
        .passiveFilter   = false,
        .mux             = PORT_MUX_AS_GPIO,
        .pinLock         = false,
        .intConfig       = PORT_DMA_INT_DISABLED,
        .clearIntFlag    = false,
        .gpioBase        = PTD,
        .direction       = GPIO_INPUT_DIRECTION,
        .digitalFilter   = false,
        .initValue       = 0U,
    },
    {
        .base            = PORTD,
        .pinPortIdx      = 3U,
        .pullConfig      = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveSelect     = PORT_LOW_DRIVE_STRENGTH,
        .passiveFilter   = false,
        .mux             = PORT_MUX_AS_GPIO,
        .pinLock         = false,
        .intConfig       = PORT_DMA_INT_DISABLED,
        .clearIntFlag    = false,
        .gpioBase        = PTD,
        .direction       = GPIO_INPUT_DIRECTION,
        .digitalFilter   = false,
        .initValue       = 0U,
    },
    {
        .base            = PORTD,
        .pinPortIdx      = 4U,
        .pullConfig      = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveSelect     = PORT_LOW_DRIVE_STRENGTH,
        .passiveFilter   = false,
        .mux             = PORT_MUX_AS_GPIO,
        .pinLock         = false,
        .intConfig       = PORT_DMA_INT_DISABLED,
        .clearIntFlag    = false,
        .gpioBase        = PTD,
        .direction       = GPIO_INPUT_DIRECTION,
        .digitalFilter   = false,
        .initValue       = 0U,
    },
    {
        .base            = PORTD,
        .pinPortIdx      = 5U,
        .pullConfig      = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveSelect     = PORT_LOW_DRIVE_STRENGTH,
        .passiveFilter   = false,
        .mux             = PORT_MUX_AS_GPIO,
        .pinLock         = false,
        .intConfig       = PORT_DMA_INT_DISABLED,
        .clearIntFlag    = false,
        .gpioBase        = PTD,
        .direction       = GPIO_OUTPUT_DIRECTION,
        .digitalFilter   = false,
        .initValue       = 0U,
    },
    {
        .base            = PORTD,
        .pinPortIdx      = 6U,
        .pullConfig      = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveSelect     = PORT_LOW_DRIVE_STRENGTH,
        .passiveFilter   = false,
        .mux             = PORT_MUX_ALT2,
        .pinLock         = false,
        .intConfig       = PORT_DMA_INT_DISABLED,
        .clearIntFlag    = false,
        .gpioBase        = NULL,
        .digitalFilter   = false,
    },
    {
        .base            = PORTD,
        .pinPortIdx      = 7U,
        .pullConfig      = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveSelect     = PORT_LOW_DRIVE_STRENGTH,
        .passiveFilter   = false,
        .mux             = PORT_MUX_ALT2,
        .pinLock         = false,
        .intConfig       = PORT_DMA_INT_DISABLED,
        .clearIntFlag    = false,
        .gpioBase        = NULL,
        .digitalFilter   = false,
    },
    {
        .base            = PORTE,
        .pinPortIdx      = 10U,
        .pullConfig      = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveSelect     = PORT_LOW_DRIVE_STRENGTH,
        .passiveFilter   = false,
        .mux             = PORT_MUX_ALT4,
        .pinLock         = false,
        .intConfig       = PORT_DMA_INT_DISABLED,
        .clearIntFlag    = false,
        .gpioBase        = NULL,
        .digitalFilter   = false,
    },
    {
        .base            = PORTE,
        .pinPortIdx      = 11U,
        .pullConfig      = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveSelect     = PORT_LOW_DRIVE_STRENGTH,
        .passiveFilter   = false,
        .mux             = PORT_MUX_ALT4,
        .pinLock         = false,
        .intConfig       = PORT_DMA_INT_DISABLED,
        .clearIntFlag    = false,
        .gpioBase        = NULL,
        .digitalFilter   = false,
    },
    {
        .base            = PORTE,
        .pinPortIdx      = 2U,
        .pullConfig      = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveSelect     = PORT_LOW_DRIVE_STRENGTH,
        .passiveFilter   = false,
        .mux             = PORT_MUX_AS_GPIO,
        .pinLock         = false,
        .intConfig       = PORT_DMA_INT_DISABLED,
        .clearIntFlag    = false,
        .gpioBase        = PTE,
        .direction       = GPIO_INPUT_DIRECTION,
        .digitalFilter   = false,
        .initValue       = 0U,
    },
    {
        .base            = PORTE,
        .pinPortIdx      = 4U,
        .pullConfig      = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveSelect     = PORT_LOW_DRIVE_STRENGTH,
        .passiveFilter   = false,
        .mux             = PORT_MUX_ALT4,
        .pinLock         = false,
        .intConfig       = PORT_DMA_INT_DISABLED,
        .clearIntFlag    = false,
        .gpioBase        = NULL,
        .digitalFilter   = false,
    },
    {
        .base            = PORTE,
        .pinPortIdx      = 5U,
        .pullConfig      = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveSelect     = PORT_LOW_DRIVE_STRENGTH,
        .passiveFilter   = false,
        .mux             = PORT_MUX_ALT4,
        .pinLock         = false,
        .intConfig       = PORT_DMA_INT_DISABLED,
        .clearIntFlag    = false,
        .gpioBase        = NULL,
        .digitalFilter   = false,
    },
    {
        .base            = PORTE,
        .pinPortIdx      = 6U,
        .pullConfig      = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveSelect     = PORT_LOW_DRIVE_STRENGTH,
        .passiveFilter   = false,
        .mux             = PORT_MUX_AS_GPIO,
        .pinLock         = false,
        .intConfig       = PORT_DMA_INT_DISABLED,
        .clearIntFlag    = false,
        .gpioBase        = PTE,
        .direction       = GPIO_INPUT_DIRECTION,
        .digitalFilter   = false,
        .initValue       = 0U,
    },
    {
        .base            = PORTE,
        .pinPortIdx      = 8U,
        .pullConfig      = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveSelect     = PORT_LOW_DRIVE_STRENGTH,
        .passiveFilter   = false,
        .mux             = PORT_MUX_AS_GPIO,
        .pinLock         = false,
        .intConfig       = PORT_DMA_INT_DISABLED,
        .clearIntFlag    = false,
        .gpioBase        = PTE,
        .direction       = GPIO_OUTPUT_DIRECTION,
        .digitalFilter   = false,
        .initValue       = 1U,
    },
};
/***********************************************************************************************************************
 * EOF
 **********************************************************************************************************************/
