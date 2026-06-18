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
- {pin_num: '48', pin_signal: PTB2, label: ADC0_CH6}
- {pin_num: '47', pin_signal: PTB3, label: ADC0_CH7}
- {pin_num: '68', pin_signal: PTB12, label: CAN2_RX}
- {pin_num: '67', pin_signal: PTB13, label: CAN2_TX}
- {pin_num: '73', pin_signal: PTA2, label: LPI2C0_SDA}
- {pin_num: '72', pin_signal: PTA3, label: LPI2C0_SCL}
- {pin_num: '54', pin_signal: PTB0, label: CAN0_RX}
- {pin_num: '53', pin_signal: PTB1, label: CAN0_TX}
- {pin_num: '40', pin_signal: PTC0, label: FTM0_INPUT_CH0}
- {pin_num: '39', pin_signal: PTC1, label: FTM0_INPUT_CH1}
- {pin_num: '56', pin_signal: PTC8, label: LPUART1_LIN_RX}
- {pin_num: '55', pin_signal: PTC9, label: LPUART1_LIN_TX}
- {pin_num: '46', pin_signal: PTC14, label: ADC0_CH12}
- {pin_num: '45', pin_signal: PTC15, label: ADC0_CH13}
- {pin_num: '3', pin_signal: PTD1, label: FTM2_OUTPUT_CH1}
- {pin_num: '31', pin_signal: PTD7, label: LPUART2_TX}
- {pin_num: '100', pin_signal: PTA8, label: WIFI_RX}
- {pin_num: '99', pin_signal: PTA9, label: WIFI_TX}
- {pin_num: '29', pin_signal: PTC3, label: LPUART0_TX}
- {pin_num: '30', pin_signal: PTC2, label: LPUART0_RX}
- {pin_num: '1', pin_signal: PTE16, label: FTM2_OUTPUT_CH7, identifier: FTM2_OUTPUT_CH7}
- {pin_num: '2', pin_signal: PTE15, label: FTM2_OUTPUT_CH6}
- {pin_num: '62', pin_signal: PTA17, label: WIFI_RST}
- {pin_num: '75', pin_signal: PTB10, label: IMU_RST, identifier: IMU_RST}
- {pin_num: '52', pin_signal: PTC10, label: KL15_MCU}
- {pin_num: '51', pin_signal: PTC11, label: 12V_DIG_IP2}
- {pin_num: '50', pin_signal: PTC12, label: DIG_IP1}
- {pin_num: '49', pin_signal: PTC13, label: DIG_IP2}
- {pin_num: '70', pin_signal: PTD3, label: 5V_DIG_IP2}
- {pin_num: '69', pin_signal: PTD4, label: 5V_DIG_IP1}
- {pin_num: '6', pin_signal: PTE10, label: RELAY2}
- {pin_num: '5', pin_signal: PTE11, label: HIGH_MOS_1}
- {pin_num: '7', pin_signal: PTE13, label: RELAY1}
- {pin_num: '42', pin_signal: PTD8, label: DEBUG_PIN}
- {pin_num: '32', pin_signal: PTD6, label: SPARE4}
- {pin_num: '36', pin_signal: PTD10, label: SPARE2}
- {pin_num: '34', pin_signal: PTD12, label: SPARE3}
- {pin_num: '8', pin_signal: PTE5, label: CAN0_TX}
- {pin_num: '9', pin_signal: PTE4, label: CAN0_RX}
- {pin_num: '80', pin_signal: PTC7, label: LPUART1_TX}
- {pin_num: '81', pin_signal: PTC6, label: LPUART1_RX}
- {pin_num: '4', pin_signal: PTD0, label: RGB_LED_BLUE, identifier: L_BLUE}
- {pin_num: '21', pin_signal: PTD16, label: RGB_LED_GREEN}
- {pin_num: '22', pin_signal: PTD15, label: RGB_LED_RED}
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
  - {pin_num: '21', peripheral: PORTD, signal: 'port, 16', pin_signal: PTD16, direction: OUTPUT, initValue: state_1}
  - {pin_num: '73', peripheral: LPI2C0, signal: 'sda, sda', pin_signal: PTA2}
  - {pin_num: '72', peripheral: LPI2C0, signal: 'scl, scl', pin_signal: PTA3}
  - {pin_num: '4', peripheral: PORTD, signal: 'port, 0', pin_signal: PTD0, direction: OUTPUT, initValue: state_1}
  - {pin_num: '9', peripheral: CAN0, signal: 'rxd, rxd', pin_signal: PTE4}
  - {pin_num: '8', peripheral: CAN0, signal: 'txd, txd', pin_signal: PTE5}
  - {pin_num: '46', peripheral: ADC0, signal: 'se, 12', pin_signal: PTC14}
  - {pin_num: '50', peripheral: PORTC, signal: 'port, 12', pin_signal: PTC12, direction: INPUT}
  - {pin_num: '49', peripheral: PORTC, signal: 'port, 13', pin_signal: PTC13, direction: INPUT}
  - {pin_num: '80', peripheral: LPUART1, signal: txd, pin_signal: PTC7, direction: OUTPUT}
  - {pin_num: '81', peripheral: LPUART1, signal: rxd, pin_signal: PTC6}
  - {pin_num: '75', peripheral: PORTB, signal: 'port, 10', pin_signal: PTB10, direction: OUTPUT}
  - {pin_num: '22', peripheral: PORTD, signal: 'port, 15', pin_signal: PTD15, direction: OUTPUT, initValue: state_1}
  - {pin_num: '77', peripheral: FTM3, signal: 'ch, 0', pin_signal: PTB8, direction: OUTPUT}
  - {pin_num: '76', peripheral: FTM3, signal: 'ch, 1', pin_signal: PTB9, direction: OUTPUT}
  - {pin_num: '70', peripheral: FTM3, signal: 'ch, 5', pin_signal: PTD3, direction: OUTPUT}
  - {pin_num: '2', peripheral: FTM2, signal: 'ch, 6', pin_signal: PTE15, direction: INPUT}
  - {pin_num: '1', peripheral: PORTE, signal: 'port, 16', pin_signal: PTE16, direction: OUTPUT}
  - {pin_num: '71', peripheral: FTM3, signal: 'ch, 4', pin_signal: PTD2, direction: OUTPUT}
  - {pin_num: '34', peripheral: FTM2, signal: 'ch, 2', pin_signal: PTD12, direction: INPUT}
  - {pin_num: '35', peripheral: PORTD, signal: 'port, 11', pin_signal: PTD11, direction: OUTPUT}
  - {pin_num: '30', peripheral: LPUART0, signal: rxd, pin_signal: PTC2}
  - {pin_num: '29', peripheral: LPUART0, signal: txd, pin_signal: PTC3, direction: OUTPUT}
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS ***********
 */
/* clang-format on */

/* Generate array of configured pin structures */
pin_settings_config_t g_pin_mux_InitConfigArr0[NUM_OF_CONFIGURED_PINS0] = {
    {
        .base            = PORTA,
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
        .base            = PORTA,
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
        .base            = PORTB,
        .pinPortIdx      = 10U,
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
        .initValue       = 0U,
    },
    {
        .base            = PORTB,
        .pinPortIdx      = 8U,
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
        .base            = PORTB,
        .pinPortIdx      = 9U,
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
        .base            = PORTC,
        .pinPortIdx      = 12U,
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
    {
        .base            = PORTC,
        .pinPortIdx      = 13U,
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
        .pinPortIdx      = 2U,
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
        .base            = PORTC,
        .pinPortIdx      = 3U,
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
        .base            = PORTC,
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
        .base            = PORTC,
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
        .base            = PORTD,
        .pinPortIdx      = 0U,
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
    {
        .base            = PORTD,
        .pinPortIdx      = 11U,
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
        .pinPortIdx      = 12U,
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
    {
        .base            = PORTD,
        .pinPortIdx      = 16U,
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
    {
        .base            = PORTD,
        .pinPortIdx      = 2U,
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
        .pinPortIdx      = 3U,
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
        .pinPortIdx      = 15U,
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
        .pinPortIdx      = 16U,
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
        .initValue       = 0U,
    },
    {
        .base            = PORTE,
        .pinPortIdx      = 4U,
        .pullConfig      = PORT_INTERNAL_PULL_NOT_ENABLED,
        .driveSelect     = PORT_LOW_DRIVE_STRENGTH,
        .passiveFilter   = false,
        .mux             = PORT_MUX_ALT5,
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
        .mux             = PORT_MUX_ALT5,
        .pinLock         = false,
        .intConfig       = PORT_DMA_INT_DISABLED,
        .clearIntFlag    = false,
        .gpioBase        = NULL,
        .digitalFilter   = false,
    },
};
/***********************************************************************************************************************
 * EOF
 **********************************************************************************************************************/
