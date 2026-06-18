/*
 * genx_oled.h
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

 * Created on: 15-06-2026
 *     Author: SasiPrasanthSakhinal
 *
 */

#ifndef GENX_OLED_H_
#define GENX_OLED_H_

#include "genx_config.h"

#ifdef OLED_DISPLAY_CONFIGURED

#include <stdint.h>
#include <stdbool.h>
#include "ancit_oled_handler.h"

/***********************************************
 * Configuration
 ***********************************************/

/**
 * @brief OLED screen refresh interval in milliseconds
 *
 * Controls how often the screen is updated with new variable values.
 * Recommended: 100-500ms for typical dashboard displays
 * - Lower values = smoother updates, more CPU usage
 * - Higher values = less smooth, less CPU usage
 */
#ifndef OLED_REFRESH_INTERVAL_MS
#define OLED_REFRESH_INTERVAL_MS    100
#endif

/***********************************************
 * ANCIT_CG_OLED_Define_Start
 ***********************************************/
// Code generation tool will insert element index defines here
// Example:
#define OLED_Distance_IDX	0
#define OLED_:_IDX	1
#define OLED_Ultra_Distance_IDX	2
#define OLED_V_set_IDX	3
#define OLED_:_IDX	4
#define OLED_Vset_IDX	5
#define OLED_D_min_IDX	6
#define OLED_:_IDX	7
#define OLED_Dmin_IDX	8
#define OLED_PWM_IDX	9
#define OLED_:_IDX	10
#define OLED_PWM_IDX	11
#define OLED_ELEMENT_MAX	12



/***********************************************
 * ANCIT_CG_OLED_Define_End
 ***********************************************/

/***********************************************
 * ANCIT_CG_OLED_Variable_Declarations_Start
 ***********************************************/
// Code generation tool will insert external variable declarations here
// Tool will directly assign values to these variables from other modules
// Example:
extern uint16_t OLED_CAN_DATA1;
extern uint16_t OLED_ADC_VOLTAGE;

/***********************************************
 * ANCIT_CG_OLED_Variable_Declarations_End
 ***********************************************/

/***********************************************
 * ANCIT_CG_OLED_Formatter_Declarations_Start
 ***********************************************/
// Code generation tool will insert formatter function declarations here
// Example:
// Code generation tool will insert formatter function implementations here
// Formatters directly access the variables declared above
void OLED_Ultra_Distance_get_formatted_val(char *buffer, uint8_t bufferSize);
// Code generation tool will insert formatter function implementations here
// Formatters directly access the variables declared above
void OLED_Vset_get_formatted_val(char *buffer, uint8_t bufferSize);
// Code generation tool will insert formatter function implementations here
// Formatters directly access the variables declared above
void OLED_Dmin_get_formatted_val(char *buffer, uint8_t bufferSize);
// Code generation tool will insert formatter function implementations here
// Formatters directly access the variables declared above
void OLED_PWM_get_formatted_val(char *buffer, uint8_t bufferSize);


#ifdef IMU_SENSOR_CONFIGURED
void OLED_IMU_HEADING_get_formatted_val(char *buffer, uint8_t bufferSize);
void OLED_IMU_PITCH_get_formatted_val(char *buffer, uint8_t bufferSize);
void OLED_IMU_ROLL_get_formatted_val(char *buffer, uint8_t bufferSize);
#endif

/***********************************************
 * ANCIT_CG_OLED_Formatter_Declarations_End
 ***********************************************/

/***********************************************
 * Public API
 ***********************************************/

/**
 * @brief Initialize OLED display module
 *
 * Initializes the OLED driver, clears the screen, and draws all
 * constant text elements. Must be called once during system init.
 *
 * NOTE: This function contains BLOCKING operations and may take 100-200ms.
 * Call only during system initialization, not in the main loop.
 */
void genx_oled_init(void);

/**
 * @brief OLED main loop function
 *
 * Must be called periodically from the main loop.
 * - Runs the OLED state machine (ssd1306_Main)
 * - Updates variable elements at configured refresh rate
 * - Triggers screen updates when needed
 *
 * Non-blocking - returns immediately.
 */
void genx_oled_main(void);

/**
 * @brief Force immediate update of all variable elements
 *
 * Normally, variables are updated at OLED_REFRESH_INTERVAL_MS rate.
 * Call this function to force an immediate update of all variables
 * and screen refresh (useful after important events).
 */
static inline void genx_oled_force_update(void) {
	ancit_oled_handler_force_update();
}

/**
 * @brief Check if OLED is currently busy
 *
 * @return true if OLED is processing an update, false if idle
 */
static inline bool genx_oled_is_busy(void) {
	return ancit_oled_handler_is_busy();
}

#endif // OLED_DISPLAY_CONFIGURED

#endif /* GENX_OLED_H_ */
