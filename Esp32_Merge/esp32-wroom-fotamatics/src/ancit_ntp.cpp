#include "ancit_ntp.h"

#include <Arduino.h>
#include <BT_LOGGER.h>
#include <lwip/apps/sntp.h>
#include <ancit_device.h>
#include <ancit_rtc_hw.h>

#include "config_reader.h"
#include "ancit_time.h"
#include "ancit_tasks.h"
EventGroupHandle_t ntp_event_group = NULL;

const char *ntpServer1 = "0.in.pool.ntp.org";
const char *ntpServer2 = "time.google.com";
const char *ntpServer3 = "pool.ntp.org";
const int daylightOffset_sec = 0;

void ntp_setup(void) {
  // narayan - this is not used now
}

void ntp_start(void) {
  ntp_event_group = xEventGroupCreate();
  xTaskCreatePinnedToCore(ntp_task,
                          "ntp_task",
                          ANCIT_NTP_TASK_STACK_SIZE,
                          NULL,
                          ANCIT_NTP_TASK_PRIORITY,
                          NULL,
                          ANCIT_NTP_TASK_CORE);
}

void ntp_task(void *param) {
  uint16_t ntp_task_delay = 60000;
  ntp_setup();

  for (;;) {
    xEventGroupWaitBits(ntp_event_group, NTP_EVENT_START_BIT, pdTRUE, pdFALSE,
                        portMAX_DELAY);

    g_Logger.Write(LogLevel::Info, LogCategory::DEVICE, "NTP",
                     "Starting NTP sync...");

    sntp_stop();
    delay(5);
    configTime(0, daylightOffset_sec, ntpServer1, ntpServer2, ntpServer3);

    bool success = false;
    uint32_t start = millis();
    while (millis() - start < 15000) {
      if (mtime_check_valid()) {
        success = true;
        break;
      }
      delay(200);
    }

    if (success) {
      time_t ts = time(nullptr);
      // globals.time.esp32_time.setTime(ts);
      globals.time.valid = true;
      ntp_task_delay = 60000;

      char timeBuf[32];
      mtime_get_time_format(ts, timeBuf);
      g_Logger.Write(LogLevel::Info, LogCategory::DEVICE, "NTP", "Time synced to UTC: %s",
                       timeBuf);
    } else {
      globals.time.valid = false;
      ntp_task_delay = 15000;
      g_Logger.Write(LogLevel::Error, LogCategory::DEVICE, "NTP",
                       "NTP sync failed.. Will retry after 15 seconds...");
      xEventGroupSetBits(ntp_event_group, NTP_EVENT_START_BIT);
    }

    vTaskDelay(pdMS_TO_TICKS(ntp_task_delay));
  }
}
