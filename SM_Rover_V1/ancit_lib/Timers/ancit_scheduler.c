/*
 * ancit_scheduler.c
 *
 *  Created on: 14-Jun-2024
 *      Author: Narayan
 */
#include "ancit_scheduler.h"
#include "ancit_common.h"

#ifdef SCHEDULER_CONFIGURED

uint32_t tickCount = 0;

__attribute__((weak)) extern task_registration_t task_reg[];

void ancit_scheduler_initialize_tasks(void) {
	//Nothing here...
}

void ancit_scheduler_one_ms(void) {
	for (int i = 0; i < gVars.tasks_max; i++) {
		if (task_reg[i].run && (tickCount >= task_reg[i].start_delay)) {
			if ((tickCount - task_reg[i].start_delay) % task_reg[i].period
					== 0) {
				task_reg[i].ready = true;  // Set the task as ready to run
			}
		}
	}
	tickCount++;
}

void ancit_scheduler_main(void) {
	// Task execution handling in the main loop
	for (int i = 0; i < gVars.tasks_max; i++) {
		if (task_reg[i].run && task_reg[i].ready) {
			task_reg[i].taskFunction();  // Execute the task
			task_reg[i].ready = false;       // Clear the ready flag
		}
	}
}

void ancit_scheduler_enable_task(uint8_t task_id) {
	if (task_id < gVars.tasks_max) {
		task_reg[task_id].run = true;  // Enable the task
	}
}

void ancit_scheduler_disable_task(uint8_t task_id) {
	if (task_id < gVars.tasks_max) {
		task_reg[task_id].run = false;  // Disable the task
	}
}

#endif //SCHEDULER_CONFIGURED
