/*
 * genx_adc.h
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
 
 * Created on: 01-07-2025
 *     Author: Narayan
 *  
 */

#ifndef GENX_ADC_H_
#define GENX_ADC_H_
#include "genx_config.h"

#ifdef ADC_CONFIGURED
#include "ancit_adc.h"

/***********************************************
 * ANCIT_CG_Define_Start
 ***********************************************/
//Total ADC Channels configured (From code generator)
#define ADC_ANG_CH1_IDX 		0


#define ADC_INPUT_MAX		1

/***********************************************
 * ANCIT_CG_Define_End
 ***********************************************/

extern adc_registration_t adc_reg[ADC_INPUT_MAX];			//Contains ADC configuration
extern adc_values_struct_t adc_values[ADC_INPUT_MAX];		//Contains ADC values

uint16_t ADC_ANG_CH1_get_raw_value(void);
uint16_t ADC_ANG_CH1_get_filtered_val(void); 
uint16_t ADC_ANG_CH1_get_mapped_val(void); 
uint32_t ADC_ANG_CH1_get_stable_val(void); 
double ADC_ANG_CH1_get_custom_value(void); 



void genx_adc_init(void);
// This macro replaces calls to genx_adc_main() with a direct call to ancit_adc_main().
#define genx_adc_main() ancit_adc_main()

#endif //ADC_CONFIGURED
#endif /* GENX_ADC_H_ */
