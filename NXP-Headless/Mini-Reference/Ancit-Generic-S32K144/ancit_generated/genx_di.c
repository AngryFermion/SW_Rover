/*
 * genx_di.c
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
#include "genx_di.h"
#include "ancit_common.h"
#include "genx_do.h"

#ifdef DIGITAL_INPUT_CONFIGURED

extern di_registration_params_t di_reg[DIGITAL_INPUT_MAX];

/***********************************************
 * ANCIT_CG_Digital_Input_Notfn_Registration_Start
 ***********************************************/
void ancit_notifiction_digital_DIG_CH1(uint8_t di_idx) {
//Write your code here....
//Pin/Input Status available in di_reg[di_idx].status
(void)di_idx;
}

di_registration_params_t di_reg[DIGITAL_INPUT_MAX] = {
//DI_DIG_CH1_IDX
{.GPIO_Port = PTC, // 
.GPIO_Pin = 12, // 
.edge_notification = ancit_notifiction_digital_DIG_CH1,// 
.debounce_ms = 25,// 
.edgeType = FALLING_EDGE,// 
.ignore_on_boot = true// 
}
};

/***********************************************
 * ANCIT_CG_Digital_Input_Notfn_Registration_End
 ***********************************************/
 
 /***********************************************
* ANCIT_CG_Digital_Input_Getters_And_Setters_Start
***********************************************/
//DI_DIG_CH1_IDX
uint8_t DI_DIG_CH1_get_raw_status(void){
return di_reg[DI_DIG_CH1_IDX].status;
}

uint8_t DI_DIG_CH1_get_debounced_status(void){
return di_reg[DI_DIG_CH1_IDX].debounced_status;
}

 //If edge is detected, return true and reset it...
bool DI_DIG_CH1_IsEdgeDetected(void) {
bool retVal = false;
if(di_reg[DI_DIG_CH1_IDX].edge_detected == true){
	di_reg[DI_DIG_CH1_IDX].edge_detected = false;
	retVal = true;
}
return retVal;
}


 
/***********************************************
* ANCIT_CG_Digital_Input_Getters_And_Setters_End
***********************************************/

void genx_di_init(void) {
	gVars.di_max = DIGITAL_INPUT_MAX;

	ancit_digital_input_start();
}
 
#endif //DIGITAL_INPUT_CONFIGURED
