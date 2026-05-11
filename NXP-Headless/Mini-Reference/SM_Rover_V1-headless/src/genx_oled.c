/*
 * genx_oled.c
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

 * Created on: 08-05-2026
 *     Author: SasiPrasanthSakhinal
 *
 */

#include "genx_oled.h"
#include "genx_common.h"

#ifdef OLED_DISPLAY_CONFIGURED

#include "ancit_oled_handler.h"
#include <stdio.h>

#ifdef ADC_CONFIGURED
#include "genx_adc.h"
#endif

#ifdef IMU_SENSOR_CONFIGURED
#include "genx_imu.h"
#endif

/***********************************************
 * ANCIT_CG_OLED_Variables_Start
 ***********************************************/

/***********************************************
 * ANCIT_CG_OLED_Variables_End
 ***********************************************/

/***********************************************
 * ANCIT_CG_OLED_Formatter_Implementations_Start
 ***********************************************/
// Code generation tool will insert formatter function implementations here
// Formatters directly access the variables declared above
// Code generation tool will insert formatter function implementations here
// Formatters directly access the variables declared above
void OLED_Distance_get_formatted_val(char *buffer, uint8_t bufferSize) {
	uint16_t Distance;

	Distance = ggenx.Distance;

	snprintf(buffer, bufferSize, "%3d", Distance);
}
// Code generation tool will insert formatter function implementations here
// Formatters directly access the variables declared above
void OLED_V_set_get_formatted_val(char *buffer, uint8_t bufferSize) {
	uint16_t V_set;

	V_set = ggenx.V_set;

	snprintf(buffer, bufferSize, "%3d", V_set);
}
// Code generation tool will insert formatter function implementations here
// Formatters directly access the variables declared above
void OLED_D_min_get_formatted_val(char *buffer, uint8_t bufferSize) {
	uint16_t D_min;

	D_min = ggenx.D_min;

	snprintf(buffer, bufferSize, "%3d", D_min);
}
// Code generation tool will insert formatter function implementations here
// Formatters directly access the variables declared above
void OLED_PWM_get_formatted_val(char *buffer, uint8_t bufferSize) {
	uint16_t PWM;

	PWM = ggenx.PWM;

	snprintf(buffer, bufferSize, "%3d", PWM);
}


#ifdef IMU_SENSOR_CONFIGURED
void OLED_IMU_HEADING_get_formatted_val(char *buffer, uint8_t bufferSize) {
	int16_t heading = genx_imu_get_heading();
	snprintf(buffer, bufferSize, "H:%3d", heading);
}

void OLED_IMU_PITCH_get_formatted_val(char *buffer, uint8_t bufferSize) {
	int16_t pitch = genx_imu_get_pitch();
	snprintf(buffer, bufferSize, "P:%3d", pitch);
}

void OLED_IMU_ROLL_get_formatted_val(char *buffer, uint8_t bufferSize) {
	int16_t roll = genx_imu_get_roll();
	snprintf(buffer, bufferSize, "R:%3d", roll);
}
#endif

/***********************************************
 * ANCIT_CG_OLED_Formatter_Implementations_End
 ***********************************************/

/***********************************************
 * ANCIT_CG_OLED_Registration_Start
 ***********************************************/
// Code generation tool will insert OLED element registration array here
// Example configuration:
oled_element_reg_t oled_element_reg[OLED_ELEMENT_MAX] =
		{
		{ .type = OLED_ELEMENT_CONST_TEXT, .x = 10, .y = 10, .font =
						OLED_FONT_6x8, .const_text = "Distance",
.value_callback = NULL, .buffer_size = 0 },
{ .type = OLED_ELEMENT_CONST_TEXT, .x = 57, .y = 10, .font =
						OLED_FONT_6x8, .const_text = ":",
.value_callback = NULL, .buffer_size = 0 },
{ .type = OLED_ELEMENT_VARIABLE, .x = 65, .y = 10, .font =
						OLED_FONT_6x8, .const_text = NULL,
OLED_Distance_get_formatted_val, .buffer_size = 16 },
{ .type = OLED_ELEMENT_CONST_TEXT, .x = 10, .y = 20, .font =
						OLED_FONT_6x8, .const_text = "V_set",
.value_callback = NULL, .buffer_size = 0 },
{ .type = OLED_ELEMENT_CONST_TEXT, .x = 57, .y = 20, .font =
						OLED_FONT_6x8, .const_text = ":",
.value_callback = NULL, .buffer_size = 0 },
{ .type = OLED_ELEMENT_VARIABLE, .x = 65, .y = 20, .font =
						OLED_FONT_6x8, .const_text = NULL,
OLED_V_set_get_formatted_val, .buffer_size = 16 },
{ .type = OLED_ELEMENT_CONST_TEXT, .x = 10, .y = 30, .font =
						OLED_FONT_6x8, .const_text = "D_min",
.value_callback = NULL, .buffer_size = 0 },
{ .type = OLED_ELEMENT_CONST_TEXT, .x = 57, .y = 30, .font =
						OLED_FONT_6x8, .const_text = ":",
.value_callback = NULL, .buffer_size = 0 },
{ .type = OLED_ELEMENT_VARIABLE, .x = 65, .y = 30, .font =
						OLED_FONT_6x8, .const_text = NULL,
OLED_D_min_get_formatted_val, .buffer_size = 16 },
{ .type = OLED_ELEMENT_CONST_TEXT, .x = 10, .y = 40, .font =
						OLED_FONT_6x8, .const_text = "PWM",
.value_callback = NULL, .buffer_size = 0 },
{ .type = OLED_ELEMENT_CONST_TEXT, .x = 57, .y = 40, .font =
						OLED_FONT_6x8, .const_text = ":",
.value_callback = NULL, .buffer_size = 0 },
{ .type = OLED_ELEMENT_VARIABLE, .x = 65, .y = 40, .font =
						OLED_FONT_6x8, .const_text = NULL,
OLED_PWM_get_formatted_val, .buffer_size = 16 },

#ifdef IMU_SENSOR_CONFIGURED
				// IMU Heading
				{ .type = OLED_ELEMENT_VARIABLE, .x = 85, .y = 20, .font =
						OLED_FONT_6x8, .const_text = NULL, .value_callback =
						OLED_IMU_HEADING_get_formatted_val, .buffer_size = 16 },
				// IMU Pitch
				{ .type = OLED_ELEMENT_VARIABLE, .x = 85, .y = 35, .font =
						OLED_FONT_6x8, .const_text = NULL, .value_callback =
						OLED_IMU_PITCH_get_formatted_val, .buffer_size = 16 },
				// IMU Roll
				{ .type = OLED_ELEMENT_VARIABLE, .x = 85, .y = 50, .font =
						OLED_FONT_6x8, .const_text = NULL, .value_callback =
						OLED_IMU_ROLL_get_formatted_val, .buffer_size = 16 }
#endif
		};

/***********************************************
 * ANCIT_CG_OLED_Registration_End
 ***********************************************/

/***********************************************
 * Runtime State (RAM)
 ***********************************************/
// Runtime state array - parallel to oled_element_reg
// Tool will use OLED_ELEMENT_MAX to size this array
static oled_element_state_t oled_element_states[OLED_ELEMENT_MAX];

/***********************************************
 * Public API Implementation
 ***********************************************/

void genx_oled_init(void) {
	// Delegate to handler with configuration
	ancit_oled_handler_init(oled_element_reg, oled_element_states,
	OLED_ELEMENT_MAX,
	OLED_REFRESH_INTERVAL_MS);
}

void genx_oled_main(void) {
	// Delegate to handler
	ancit_oled_handler_main();
}

#endif // OLED_DISPLAY_CONFIGURED
