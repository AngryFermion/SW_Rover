#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <Arduino.h>

// Task function pointer type
typedef void (*TaskFunction)(void);

// Maximum number of tasks
#define MAX_TASKS 10

// Task structure
typedef struct {
    TaskFunction function;
    unsigned long interval_ms;
    unsigned long last_run_ms;
    bool enabled;
} Task;

// Scheduler class
class Scheduler {
public:
    Scheduler();

    // Initialize scheduler
    void init();

    // Add a task to the scheduler
    // Returns task ID (index) or -1 if failed
    int addTask(TaskFunction function, unsigned long interval_ms);

    // Enable/disable a task
    void enableTask(int taskId, bool enable);

    // Update scheduler - call this in loop()
    void update();

    // Get current system time in milliseconds
    unsigned long getSystemTimeMs();

private:
    Task tasks[MAX_TASKS];
    int taskCount;
    unsigned long systemStartTime;
};

// Global scheduler instance
extern Scheduler scheduler;

#endif // SCHEDULER_H
