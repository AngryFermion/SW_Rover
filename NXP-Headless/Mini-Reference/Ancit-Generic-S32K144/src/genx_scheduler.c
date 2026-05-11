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
 
 * Created on: 12-06-2025
 *     Author: Narayan
 *  
 */
#include "ancit_common.h"
#include "ancit_scheduler.h"
#include "genx_scheduler.h"

#ifdef SCHEDULER_CONFIGURED
#include "genx_runnables.h"

void Task_100ms(void) {
	Runnable_100ms();
}

void Task_1000ms(void) {
//	ancit_hl_pwm_main();
	Runnable_1000ms();
}

void Task_10ms(void) {
	Runnable_10ms();
}

void Task_500ms(void) {
	Runnable_500ms();
}

/***********************************************
 * ANCIT_CG_Scheduler_Register_Start
 ***********************************************/
// Initialize the parameters for each task
task_registration_t task_reg[MAX_TASKS] = {
//TASK_Task_100ms_IDX_0// 
		{ .taskFunction = Task_100ms, //
				.period = 100, //
				.start_delay = 0, //
				.run = true //
		},
//TASK_Task_1000ms_IDX_1// 
		{ .taskFunction = Task_1000ms, //
				.period = 1000, //
				.start_delay = 0, //
				.run = true //
		},
//TASK_Task_10ms_IDX_2// 
		{ .taskFunction = Task_10ms, //
				.period = 10, //
				.start_delay = 0, //
				.run = true //
		},
//TASK_Task_500ms_IDX_1//
		{ .taskFunction = Task_500ms, //
				.period = 500, //
				.start_delay = 0, //
				.run = true //
		}, };

/***********************************************
 * ANCIT_CG_Scheduler_Register_End
 ***********************************************/

void genx_scheduler_init(void) {
	gVars.tasks_max = MAX_TASKS;

	ancit_scheduler_initialize_tasks();
}

#endif //SCHEDULER_CONFIGURED
