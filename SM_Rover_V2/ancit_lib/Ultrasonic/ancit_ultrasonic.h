/*
 * ancit_ultrasonic.h
 *
 * Copyright (c) 2024-2025 ANCIT Consulting Pvt Ltd
 *
 * HC-SR04 Ultrasonic Sensor Driver for S32K144
 * Uses FTM3 Input Capture on CH0 (PTB8) for echo measurement
 * and GPIO output on PTB9 for trigger pulse.
 *
 * Created on: 18-03-2026
 *     Author: Narayan_PC
 */

#ifndef ANCIT_ULTRASONIC_H_
#define ANCIT_ULTRASONIC_H_

#include "genx_config.h"

#ifdef ULTRASONIC_CONFIGURED

#include <stdint.h>
#include <stdbool.h>

/* Pin definitions - Trigger on PTB9, Echo on PTB8 (FTM3_CH0) */
#define US_TRIGGER_PORT     PTD
#define US_TRIGGER_PIN      11U

#define US_ECHO_FTM_INSTANCE    INST_FLEXTIMER_IC_ECHO  /* FTM3 = instance 3 */
#define US_ECHO_FTM_CHANNEL     2U                       /* CH0 on PTB8 */

/* Timing constants */
#define US_TRIGGER_DURATION_US  20U     /* HC-SR04 requires minimum 10us trigger pulse, using 15us for safety margin */
#define US_MEASUREMENT_INTERVAL_MS  60U  /* Measurement every 60ms (HC-SR04 min safe cycle: 60ms) */

/* Max valid range in cm. Readings above this are treated as invalid (no object detected) */
#define US_MAX_RANGE_CM         400.0f  /* HC-SR04 rated max range: 4 meters */

/* Stability filter: require this many consecutive readings within tolerance before accepting */
/* Added by Sasi - 2026-06-15 */
#define US_FILTER_SIZE          3
#define US_FILTER_TOLERANCE_CM  3.0f  /* unused — kept for reference */

/* Ultrasonic driver state */
typedef struct {
    /* Volatile flags set from ISR context */
    volatile bool echo_received;
    volatile bool echo_timed_out;
    volatile bool trigger_pending;
    volatile bool measurement_in_progress;

    /* Measurement interval counter */
    uint16_t interval_counter_ms;

    /* Result storage */
    float last_distance_cm;
    float temperature_c;
    bool measurement_valid;
} ultrasonic_state_t;

/* Initialize ultrasonic driver: configures FTM3 input capture and trigger GPIO */
void ancit_ultrasonic_init(void);

/* Main loop function: handles trigger and reads measurement results */
void ancit_ultrasonic_main(void);

/* 1ms tick handler: call from ancit_one_ms_handler() for measurement interval timing */
void ancit_ultrasonic_one_ms(void);

/* Get last measured distance in centimeters. Returns 0.0 if invalid/timeout */
float ancit_ultrasonic_get_distance_cm(void);

/* Get last measured distance in millimeters. Returns 0 if invalid/timeout */
uint16_t ancit_ultrasonic_get_distance_mm(void);

/* Returns true if the last measurement was valid (echo received within range) */
bool ancit_ultrasonic_is_valid(void);

/* Set ambient temperature for speed-of-sound compensation (default 22.0 C) */
void ancit_ultrasonic_set_temperature(float temp_c);

#endif /* ULTRASONIC_CONFIGURED */
#endif /* ANCIT_ULTRASONIC_H_ */
