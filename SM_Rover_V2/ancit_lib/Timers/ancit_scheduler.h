/*
 * ancit_scheduler.h
 *
 *  Created on: 14-Jun-2024
 *      Author: Narayan
 */

#ifndef ANCIT_SCHEDULER_H_
#define ANCIT_SCHEDULER_H_

#include "genx_config.h"

#ifdef SCHEDULER_CONFIGURED

typedef struct {
    void (*taskFunction)(void);  // Pointer to the task to run
    uint32_t period;             // Period of the task in ticks
    uint32_t start_delay;        // Initial delay in ticks before the first run
    bool ready;               // Flag to indicate if the task is ready to run
    bool run;                 // Flag to enable the task
} task_registration_t;

void ancit_scheduler_initialize_tasks(void);
void ancit_scheduler_one_ms(void);
void ancit_scheduler_main(void);
void ancit_scheduler_enable_task(uint8_t uint8_t);
void ancit_scheduler_disable_task(uint8_t uint8_t);

#endif //SCHEDULER_CONFIGURED
#endif /* ANCIT_SCHEDULER_H_ */
