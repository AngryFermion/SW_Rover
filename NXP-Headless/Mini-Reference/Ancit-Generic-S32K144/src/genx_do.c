/*
 * genx_do.c
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
 
 * Created on: 10-06-2025
 *     Author: Narayan
 *  
 */
#include "genx_do.h"

#include "ancit_common.h"

#ifdef DIGITAL_OUTPUT_CONFIGURED
/***********************************************
 * ANCIT_CG_Digital_Output_Register_Start
 ***********************************************/
do_registration_params_t do_reg[DIGITAL_OUTPUT_MAX] = {
//DO_LED_RED_IDX
		{ .GPIO_Port = PTD, //
				.GPIO_Pin = 15, //
				.triggered = false, // 
				.initialValue = INIT_VALUE_HIGH, // 
				.active_type = ACTIVE_LOW // 
		},
		//DO_LED_GREEN_IDX
		{ .GPIO_Port = PTD, //
				.GPIO_Pin = 0, //
				.triggered = false, //
				.initialValue = INIT_VALUE_HIGH, //
				.active_type = ACTIVE_LOW //
		}
//Configure your digital outputs here...
//End
		};

/***********************************************
 * ANCIT_CG_Digital_Output_Register_End
 ***********************************************/
/***********************************************
 * ANCIT_CG_Digital_Output_Getters_And_Setters_Start
 ***********************************************/
//DO_LED_RED
void DO_LED_RED_set_value(uint8_t value) {

	ancit_digital_output_set(DO_LED_RED_IDX, value);
	
}

void DO_LED_RED_toggle(void) {
	ancit_digital_output_toggle(DO_LED_RED_IDX);
}

void DO_LED_GREEN_set_value(uint8_t value) {
	ancit_digital_output_set(DO_LED_GREEN_IDX, value);
}

void DO_LED_GREEN_toggle(void) {
	ancit_digital_output_toggle(DO_LED_GREEN_IDX);
}

/***********************************************
 * ANCIT_CG_Digital_Output_Getters_And_Setters_End
 ***********************************************/

void genx_do_init(void) {
	gVars.do_max = DIGITAL_OUTPUT_MAX;

	ancit_digital_output_start();
}

#endif //DIGITAL_OUTPUT_CONFIGURED
