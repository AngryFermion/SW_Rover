/*
 * ancit_ultrasonic.c
 *
 * Copyright (c) 2024-2025 ANCIT Consulting Pvt Ltd
 *
 * HC-SR04 Ultrasonic Sensor Driver for S32K144
 * Uses FTM3 Input Capture (CH0, PTB8) for echo pulse width measurement
 * and GPIO (PTB9) for trigger pulse.
 *
 * Flow:
 *   1. _one_ms() counts to 100ms -> sets trigger_pending flag
 *   2. _main() sees flag -> sends 10us trigger pulse -> FTM IC starts measuring
 *   3. FTM hardware captures echo pulse width -> callback_echo_received fires
 *   4. OR FTM overflow fires (~65.5ms) -> timeout
 *   5. _main() reads result and computes distance
 *
 * Created on: 18-03-2026
 *     Author: Narayan_PC
 */

#include "ancit_ultrasonic.h"

#ifdef ULTRASONIC_CONFIGURED

#include "sdk_project_config.h"
#include "peripherals_flexTimer_ic_echo.h"
#include "ftm_common.h"
#include "ftm_ic_driver.h"
#include "pins_driver.h"

/* FTM state structure required by the SDK driver */
static ftm_state_t ftm3_ic_state;

/* Ultrasonic driver state */
static ultrasonic_state_t us_state = {
    .echo_received          = false,
    .echo_timed_out         = false,
    .trigger_pending        = false,
    .measurement_in_progress = false,
    .interval_counter_ms    = 0,
    .last_distance_cm       = 0.0f,
    .temperature_c          = 22.0f,
    .measurement_valid      = false
};

/*
 * FTM3 Input Capture callback - called from ISR when pulse measurement completes.
 * The SDK has already stored the pulse width in measurementResults[channel].
 */
void callback_echo_received(ic_event_t event, void *userData)
{
    (void)userData;
    (void)event;

    us_state.echo_received = true;
    us_state.measurement_in_progress = false;
}

/*
 * FTM3 Timer Overflow IRQ handler.
 * Called when the 16-bit counter wraps around (~65.5ms at 1MHz).
 * If a measurement is in progress and no echo was received, this is a timeout.
 */
void FTM2_Ovf_Reload_IRQHandler(void)
{
    /* Clear the overflow flag */
    FTM_DRV_ClearStatusFlags(US_ECHO_FTM_INSTANCE, (uint32_t)FTM_TIME_OVER_FLOW_FLAG);

    if (us_state.measurement_in_progress && !us_state.echo_received)
    {
        us_state.echo_timed_out = true;
        us_state.measurement_in_progress = false;
    }
}

/*
 * Simple busy-wait microsecond delay using a calibrated loop.
 * At 48MHz core clock, ~12 cycles per iteration gives ~1us.
 */
static void delay_us(uint32_t us)
{
    volatile uint32_t count;
    for (count = 0; count < (us * 12U); count++)
    {
        /* busy wait */
    }
}

/*
 * Initialize the ultrasonic driver.
 * - Initializes FTM3 common config (clock, prescaler, mode)
 * - Initializes FTM3 CH0 for input capture (signal measurement, pulse HIGH)
 * - Trigger pin (PTB9) is already configured as GPIO output in pin_mux
 */
void ancit_ultrasonic_init(void)
{
    /* Initialize FTM3 with the generated config (clock, prescaler, mode) */
    FTM_DRV_Init(US_ECHO_FTM_INSTANCE,
                 &flexTimer_ic_echo_InitConfig,
                 &ftm3_ic_state);

    /* Initialize input capture on CH0 */
    FTM_DRV_InitInputCapture(US_ECHO_FTM_INSTANCE,
                             &flexTimer_ic_echo_InputCaptureConfig);

    /* Ensure trigger pin starts LOW */
    PINS_DRV_ClearPins(US_TRIGGER_PORT, (1U << US_TRIGGER_PIN));

    /* Reset state */
    us_state.echo_received = false;
    us_state.echo_timed_out = false;
    us_state.trigger_pending = false;
    us_state.measurement_in_progress = false;
    us_state.interval_counter_ms = 0;
    us_state.last_distance_cm = 0.0f;
    us_state.temperature_c = 22.0f;
    us_state.measurement_valid = false;
}

/*
 * 1ms tick handler - call from ancit_one_ms_handler().
 * Counts up to US_MEASUREMENT_INTERVAL_MS and sets trigger_pending flag.
 */
void ancit_ultrasonic_one_ms(void)
{
    us_state.interval_counter_ms++;
    if (us_state.interval_counter_ms >= US_MEASUREMENT_INTERVAL_MS)
    {
        us_state.interval_counter_ms = 0;
        if (!us_state.measurement_in_progress)
        {
            us_state.trigger_pending = true;
        }
    }
}

/*
 * Main loop function - call from the infinite while loop in main().
 *
 * Handles two things:
 * 1. If trigger_pending: sends 10us trigger pulse and starts FTM measurement
 * 2. If echo_received or echo_timed_out: processes the result
 */
void ancit_ultrasonic_main(void)
{
    /* --- Send trigger pulse --- */
    if (us_state.trigger_pending)
    {
        us_state.trigger_pending = false;
        us_state.echo_received = false;
        us_state.echo_timed_out = false;

        /* Reset FTM counter before new measurement */
        FTM_DRV_CounterReset(US_ECHO_FTM_INSTANCE, false);

        /* Send 10us trigger pulse */
        PINS_DRV_SetPins(US_TRIGGER_PORT, (1U << US_TRIGGER_PIN));
        delay_us(US_TRIGGER_DURATION_US);
        PINS_DRV_ClearPins(US_TRIGGER_PORT, (1U << US_TRIGGER_PIN));

        us_state.measurement_in_progress = true;
    }

    /* --- Process echo result --- */
    if (us_state.echo_received)
    {
        us_state.echo_received = false;

        /* Read pulse width from FTM driver (in timer ticks) */
        uint16_t pulse_ticks = FTM_DRV_GetInputCaptureMeasurement(US_ECHO_FTM_INSTANCE,
                                                                   US_ECHO_FTM_CHANNEL);

        /* Convert ticks to microseconds.
         * FTM3 clock: 8MHz external / prescaler 8 = 1MHz = 1us per tick.
         */
        uint32_t pulse_us = (uint32_t)pulse_ticks;

        /* Calculate distance:
         * distance_cm = (pulse_us * speed_of_sound_m_s) / 20000
         * speed_of_sound = 331.3 + 0.606 * temperature_c
         */
        float speed = 331.3f + 0.606f * us_state.temperature_c;
        float distance_cm = ((float)pulse_us * speed) / 20000.0f;

        if (distance_cm > US_MAX_RANGE_CM)
        {
            /* Beyond sensor range — treat as no object detected */
            us_state.last_distance_cm = 0.0f;
            us_state.measurement_valid = false;
        }
        else
        {
            us_state.last_distance_cm = distance_cm;
            us_state.measurement_valid = true;
        }
    }
    else if (us_state.echo_timed_out)
    {
        us_state.echo_timed_out = false;

        us_state.last_distance_cm = 0.0f;
        us_state.measurement_valid = false;
    }
}

float ancit_ultrasonic_get_distance_cm(void)
{
    return us_state.last_distance_cm;
}

uint16_t ancit_ultrasonic_get_distance_mm(void)
{
    if (!us_state.measurement_valid)
    {
        return 0;
    }
    return (uint16_t)(us_state.last_distance_cm * 10.0f);
}

bool ancit_ultrasonic_is_valid(void)
{
    return us_state.measurement_valid;
}

void ancit_ultrasonic_set_temperature(float temp_c)
{
    us_state.temperature_c = temp_c;
}

#endif /* ULTRASONIC_CONFIGURED */
