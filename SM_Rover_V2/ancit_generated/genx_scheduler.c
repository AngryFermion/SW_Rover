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
 
 * Created on: 15-06-2026
 *     Author: SasiPrasanthSakhinal
 *  
 */
#include "ancit_common.h"
#include "ancit_scheduler.h"
#include "genx_scheduler.h"
#include "genx_common.h"
#include "string.h"

#ifdef UNIFIED_DIAGNOSTICS_SERVICES_CONFIGURED
#include "cantp.h"
#include "diagSess.h"
#include "timer_helper.h"

uint32_t mill_sec_value = 0;


#endif

#ifdef SCHEDULER_CONFIGURED
//#include "genx_runnables.h"
#ifdef SIMULINK_BRIDGE_CONFIGURED
#include "genx_simulink_bridge.h"
#endif /* SIMULINK_BRIDGE_CONFIGURED */
#ifdef UART_RTE_CONFIGURED
#include "genx_uart_rte.h"
#endif

void Task_OnStart(void) {
#ifdef RTE_VARIABLES_CONFIGURED
genx_global_init();
#endif
} 

void Task_1ms(void) {
#ifdef SIMULINK_BRIDGE_CONFIGURED
ANCIT_App_PreStep();
ACC_step();
ANCIT_App_PostStep();
#endif /* SIMULINK_BRIDGE_CONFIGURED */
} 

void Task_10ms(void) {
#ifdef UART_RTE_CONFIGURED
ancit_uart_message_setup();
#endif //UART_RTE_CONFIGURED
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
