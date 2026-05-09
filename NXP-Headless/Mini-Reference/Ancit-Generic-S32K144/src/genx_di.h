/*
 * genx_di.h
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

#ifndef GENX_DI_H_
#define GENX_DI_H_

#include "genx_config.h"

#ifdef DIGITAL_INPUT_CONFIGURED
#include <ancit_di.h>
/***********************************************
 * ANCIT_CG_Define_Start
 ***********************************************/

#define DI_PD_CH1_IDX 		0
#define DI_PD_CH2_IDX 		1
#define DI_PD_CH3_IDX 		2
#define DI_PD_CH4_IDX 		3
#define DI_PD_CH5_IDX 		4
#define DI_PD_CH6_IDX 		5

#define DIGITAL_INPUT_MAX		6

#define DI_PD_DEBOUNCE_MS	100

/***********************************************
 * ANCIT_CG_Define_End
 ***********************************************/

/***********************************************
 * ANCIT_CG_Digital_Input_Getters_And_Setters_Start
 ***********************************************/
//DI_DIG_CH1_IDX
uint8_t DI_DIG_CH1_get_raw_status(void);
uint8_t DI_DIG_CH1_get_debounced_status(void);
bool DI_DIG_CH1_IsEdgeDetected(void);


uint8_t DI_PD_CH1_get_debounced_status(void);
uint8_t DI_PD_CH2_get_debounced_status(void);
uint8_t DI_PD_CH3_get_debounced_status(void);
uint8_t DI_PD_CH4_get_debounced_status(void);
uint8_t DI_PD_CH5_get_debounced_status(void);
uint8_t DI_PD_CH6_get_debounced_status(void);

/***********************************************
 * ANCIT_CG_Digital_Input_Getters_And_Setters_End
 ***********************************************/

void genx_di_init(void);
// This macro replaces calls to genx_di_main() with a direct call to ancit_digital_input_main().
#define genx_di_main() ancit_digital_input_main()

#endif //DIGITAL_INPUT_CONFIGURED
#endif /* GENX_ADC_H_ */
