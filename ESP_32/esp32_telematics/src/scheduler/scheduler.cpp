#include "scheduler.h"
#include "../config/system_config.h"
#include "../managers/uart_manager.h"

// Global scheduler instance
Scheduler scheduler;

Scheduler::Scheduler() {
    taskCount = 0;
    systemStartTime = 0;
}

void Scheduler::init() {
    systemStartTime = millis();
    taskCount = 0;

    for (int i = 0; i < MAX_TASKS; i++) {
        tasks[i].function = nullptr;
        tasks[i].interval_ms = 0;
        tasks[i].last_run_ms = 0;
        tasks[i].enabled = false;
    }

    #if ENABLE_SERIAL_DEBUG
        Serial.println("[SCHEDULER] Initialized");
        Serial.printf("[SCHEDULER] Tick interval: %d ms\n", SCHEDULER_TICK_MS);
    #endif
}

int Scheduler::addTask(TaskFunction function, unsigned long interval_ms) {
    if (taskCount >= MAX_TASKS) {
        #if ENABLE_SERIAL_DEBUG
            uartManager.loggerWriteLine("[SCHEDULER] ERROR: Maximum tasks reached!");
        #endif
        return -1;
    }

    int taskId = taskCount;
    tasks[taskId].function = function;
    tasks[taskId].interval_ms = interval_ms;
    tasks[taskId].last_run_ms = millis();
    tasks[taskId].enabled = true;

    taskCount++;

    #if ENABLE_SERIAL_DEBUG
        uartManager.loggerPrintf("[SCHEDULER] Task added - ID: %d | Interval: %lu ms\n", taskId, interval_ms);
    #endif

    return taskId;
}

void Scheduler::enableTask(int taskId, bool enable) {
    if (taskId >= 0 && taskId < taskCount) {
        tasks[taskId].enabled = enable;

        #if ENABLE_SERIAL_DEBUG
            uartManager.loggerPrintf("[SCHEDULER] Task %d %s\n", taskId, enable ? "enabled" : "disabled");
        #endif
    }
}

void Scheduler::update() {
    unsigned long currentTime = millis();

    for (int i = 0; i < taskCount; i++) {
        if (tasks[i].enabled && tasks[i].function != nullptr) {
            if (currentTime - tasks[i].last_run_ms >= tasks[i].interval_ms) {
                tasks[i].last_run_ms = currentTime;
                tasks[i].function();
            }
        }
    }
}

unsigned long Scheduler::getSystemTimeMs() {
    return millis() - systemStartTime;
}
