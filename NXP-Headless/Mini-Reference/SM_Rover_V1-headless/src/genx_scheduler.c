/*
 * genx_scheduler.c
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
#include "ancit_common.h"
#include "ancit_scheduler.h"
#include "genx_scheduler.h"
#include "genx_common.h"
#include "string.h"
#include "SK_DC.h"
#include "ancit_driver_uart.h"
#include "stdio.h"
#include "genx_ultrason.h"


#ifdef UNIFIED_DIAGNOSTICS_SERVICES_CONFIGURED
#include "cantp.h"
#include "diagSess.h"
#include "timer_helper.h"

uint32_t mill_sec_value = 0;


#endif


//#include "genx_runnables.h"
#ifdef SIMULINK_BRIDGE_CONFIGURED
#include "genx_simulink_bridge.h"
#endif /* SIMULINK_BRIDGE_CONFIGURED */

#ifdef SCHEDULER_CONFIGURED
//#include "genx_runnables.h"
#ifdef UART_RTE_CONFIGURED
#include "genx_uart_rte.h"
#endif

uint8_t buffer[10];
int mode=4, state=1, dir, pwm;

void Task_OnStart(void) {
#ifdef RTE_VARIABLES_CONFIGURED
genx_global_init();
#endif
} 


void Task_While(void){
	ggenx.Distance = genx_ultrason_get_distance_cm();
}
void Task_1ms(void) {

#ifdef ADAS_USECASE_CONFIGURED
if(mode == 4 && state == 1){
	ANCIT_App_PreStep();
	ACC_step();
	ANCIT_App_PostStep();
	genx_PWM_LFM_updateDutyCycle(100);
	genx_PWM_LBM_updateDutyCycle(100 - ggenx.PWM);
	genx_PWM_RFM_updateDutyCycle(100);
	genx_PWM_RBM_updateDutyCycle(100 - ggenx.PWM);
}
#endif

}

void Task_10ms(void) {



ancit_driver_uart_ReceiveData(INST_LPUART_1, buffer, 10U);
sscanf((char *)buffer, "%d,%d,%d,%d", &mode, &state, &dir, &pwm);
ggenx.PWM = pwm;
if(state == 1){
	if(mode ==1 || mode == 2){
		ancit_smartkit_dc();
	}
#ifdef ADAS_USECASE_CONFIGURED
	if(mode == 3){
		ancit_uart_message_setup();
		if(ggenx.Distance < 50){
			genx_PWM_LFM_updateDutyCycle(100);
			genx_PWM_LBM_updateDutyCycle(100);
			genx_PWM_RFM_updateDutyCycle(100);
			genx_PWM_RBM_updateDutyCycle(100);
		}
		else{
			genx_PWM_LFM_updateDutyCycle(100);
			genx_PWM_LBM_updateDutyCycle(100 - ggenx.PWM);
			genx_PWM_RFM_updateDutyCycle(100);
			genx_PWM_RBM_updateDutyCycle(100 - ggenx.PWM);
		}
	}
	if(mode == 4){
		ancit_uart_message_setup();
	}
#endif
	else{
		genx_PWM_LFM_updateDutyCycle(100);
		genx_PWM_LBM_updateDutyCycle(100);
		genx_PWM_RFM_updateDutyCycle(100);
		genx_PWM_RBM_updateDutyCycle(100);
	}
}
else{
	genx_PWM_LFM_updateDutyCycle(100);
	genx_PWM_LBM_updateDutyCycle(100);
	genx_PWM_RFM_updateDutyCycle(100);
	genx_PWM_RBM_updateDutyCycle(100);
}

} 



/***********************************************
 * ANCIT_CG_Scheduler_Register_Start
 ***********************************************/
// Initialize the parameters for each task
task_registration_t task_reg[MAX_TASKS] = {
//TASK_Task_OnStart_IDX_0// 
{ .taskFunction = Task_OnStart, // 
.period = 0, // 
.start_delay = 0, // 
.run =true // 
}, 
//TASK_Task_1ms_IDX_1// 
{ .taskFunction = Task_1ms, // 
.period = 1, // 
.start_delay = 0, // 
.run =true // 
}, 
//TASK_Task_10ms_IDX_2// 
{ .taskFunction = Task_10ms, // 
.period = 10, // 
.start_delay = 0, // 
.run =true // 
}
}; 

/***********************************************
 * ANCIT_CG_Scheduler_Register_End
 ***********************************************/
 
 void genx_scheduler_init(void) {
	gVars.tasks_max = MAX_TASKS;

	Task_OnStart();
	ancit_scheduler_initialize_tasks();
}
 
#endif //SCHEDULER_CONFIGURED
