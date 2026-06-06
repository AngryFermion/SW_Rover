#include "ancit_timer.h"

#include "ancit_wifi_manager.h"  //for event flag setting
#include "ancit_tasks.h"
extern EventGroupHandle_t WifiManager_event_group;

void vTimerTriggerTask(void *pvParameters) {
  const TickType_t baseTick = pdMS_TO_TICKS(100);
  TickType_t xLastWakeTime = xTaskGetTickCount();

  uint8_t counter500 = 0;
  uint8_t counter1000 = 0;

  for (;;) {
    vTaskDelayUntil(&xLastWakeTime, baseTick);

    if (++counter1000 >= 10) {
      counter1000 = 0;
      xEventGroupSetBits(WifiManager_event_group, EVENT_WIFI_1000MS);
    }
  }
}

void vTimerTriggerTask_start(void) {
  // Start WiFi monitor task on Core 1
  xTaskCreatePinnedToCore(
      vTimerTriggerTask,          /* Task function. */
      "TimerTrigger",             /* name of task. */
      ANCIT_TIMER_TASK_STACK_SIZE, /* Stack size of task */
      NULL,                       /* parameter of the task */
      ANCIT_TIMER_TASK_PRIORITY,   /* priority of the task */
      NULL,                  /* Task handle to keep track of created task */
      ANCIT_TIMER_TASK_CORE); /* Task to be executed in Core-1 only */
}
