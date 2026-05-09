/*
 * genx_adc.c
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
 
 * Created on: 30-06-2025
 *     Author: Narayan
 *  
 */
#include "genx_adc.h"
#include "genx_di.h"
#include "genx_do.h"

#ifdef ADC_CONFIGURED
adc_registration_t adc_reg[ADC_INPUT_MAX];			//Contains ADC configuration
adc_values_struct_t adc_values[ADC_INPUT_MAX];		//Contains ADC values

/***********************************************
 * ANCIT_CG_Rx_Notfn_REGISTRATION_Start
 ***********************************************/
void ancit_conv_complete_notfn_ANG_CH1(uint8_t adc_idx) {
//The following structure gives you the required parameters.
//ADC Config Parameters - adc_reg[adc_idx]
//ADC values - adv_values[adc_idx]

//	test_val =
	(void) adc_idx;
}

adc_registration_t adc_reg[ADC_INPUT_MAX] = {
//ADC_ANG_CH1
		{ .enabled = true, //
				.interval_counter = 0, //
				.triggered = true, //
				.channel_config.interruptEnable = false, //
				.channel_config.channel = ADC_INPUTCHAN_EXT7, //
				.conv_complete_notfn = ancit_conv_complete_notfn_ANG_CH1, //
				.set_interval_ms = 5, //
				.filter_type = ADC_FILTER_TYPE_IIR, //
				.filter_setting1 = 10, //
				.apply_mapping = true, //
				.map.adc_min = 0, //
				.map.adc_max = 4095, //
				.map.val_min = 0, //
				.map.val_max = 1500, //
				.instance_idx = 1 //
// 
		}

};
/***********************************************
 * ANCIT_CG_Rx_Notfn_REGISTRATION_End
 ***********************************************/

/***********************************************
 * ANCIT_CG_ADC_Getters_And_Setters_Start
 ***********************************************/

//Getter Methods for ANG_CH1
uint16_t ADC_ANG_CH1_get_raw_value() {
	return adc_values[ADC_ANG_CH1_IDX].raw_val;
}

uint16_t ADC_ANG_CH1_get_filtered_val() {
	return adc_values[ADC_ANG_CH1_IDX].filtered_val;
}

uint16_t ADC_ANG_CH1_get_mapped_val() {
	return adc_values[ADC_ANG_CH1_IDX].mapped_val;
}

uint32_t ADC_ANG_CH1_get_stable_val() {
	return adc_values[ADC_ANG_CH1_IDX].stable_val;
}

double ADC_ANG_CH1_get_custom_value() {
	double ret_val = 0;
	return ret_val;
}

/***********************************************
 * ANCIT_CG_ADC_Getters_And_Setters_End
 ***********************************************/

void genx_adc_init(void) {
	gVars.ai_max = ADC_INPUT_MAX;

	ancit_adc_start();
}

#endif //ADC_CONFIGURED
