#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <Arduino.h>

typedef void (*TaskFunction)(void);

#define MAX_TASKS 10

typedef struct {
    TaskFunction  function;
    unsigned long interval_ms;
    unsigned long last_run_ms;
    bool          enabled;
} Task;

class Scheduler {
public:
    Scheduler();

    void init();
    int  addTask(TaskFunction function, unsigned long interval_ms);
    void enableTask(int taskId, bool enable);
    void update();
    unsigned long getSystemTimeMs();

private:
    Task          tasks[MAX_TASKS];
    int           taskCount;
    unsigned long systemStartTime;
};

extern Scheduler scheduler;

#endif // SCHEDULER_H
