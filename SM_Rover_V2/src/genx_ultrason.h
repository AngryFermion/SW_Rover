/*
 * genx_ultrason.h
 *
 * Copyright (c) 2024-2025 ANCIT Consulting Pvt Ltd
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.

 * Created on: 19-03-2026
 *     Author: Narayan_PC
 *
 */

/*
 * Application-level interface for the Ultrasonic Sensor module.
 *
 * This header provides a unified genx_ API that maps to the HC-SR04
 * ultrasonic sensor driver (FTM input capture on PTB8/PTB9).
 *
 * Usage:
 *   genx_ultrason_init()   - Call once during initialization
 *   genx_ultrason_main()   - Call in the main loop
 */

#ifndef GENX_ULTRASON_H_
#define GENX_ULTRASON_H_
#include "genx_config.h"

/*
 * HC-SR04 Ultrasonic Sensor (FTM Input Capture)
 * Driver: ancit_lib/Ultrasonic/ancit_ultrasonic.c
 */
#ifdef ULTRASONIC_CONFIGURED
#include "ancit_ultrasonic.h"

/* Initialize ultrasonic sensor hardware (FTM3 + trigger GPIO) */
#define genx_ultrason_init()                ancit_ultrasonic_init()
/* Main loop handler: triggers measurement and processes echo */
#define genx_ultrason_main()                ancit_ultrasonic_main()
/* Get last measured distance in millimeters (for CAN TX) */
#define genx_ultrason_get_distance_mm()     ancit_ultrasonic_get_distance_mm()
/* Get last measured distance in centimeters (returns -1.0 if invalid) */
#define genx_ultrason_get_distance_cm()     ancit_ultrasonic_get_distance_cm()
/* Returns true if the last measurement is valid */
#define genx_ultrason_is_valid()            ancit_ultrasonic_is_valid()
/* Set ambient temperature in deg C for speed-of-sound compensation */
#define genx_ultrason_set_temperature(t)    ancit_ultrasonic_set_temperature(t)

#endif /* ULTRASONIC_CONFIGURED */

#endif /* GENX_ULTRASON_H_ */
