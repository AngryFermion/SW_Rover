// http_upload.cpp
#include "ancit_http_client.h"
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "config_reader.h"
#include <BT_LOGGER.h>
#include "ancit_device.h"
#include "ancit_tasks.h"
extern JsonDocument devComm_doc;
HttpConfig http_config;
HTTPClient http;

void HttpClient_LoadFromJson(JsonDocument& doc) {
  if (!doc["http"].is<JsonObject>()) return;

  JsonObject http = doc["http"];
  http_config.enabled = http["en"] | false;
  http_config.data_format = http["df"] | "1";
  http_config.url = http["url"] | "";
  http_config.port = http["port"] | 80;
  http_config.tx_interval = http["tx_interval"] | 60;
  http_config.auth_type = http["auth_type"] | "none";
  http_config.username = http["username"] | "";
  http_config.password = http["password"] | "";
  http_config.token = http["token"] | "";
  http_config.content_type = http["content_type"] | "application/json";
}

static void HttpClient_PostData() {
  http.begin(http_config.url);
  g_Logger.Write(LogLevel::Debug, LogCategory::HTTP, "HttpClient_PostData",
                   "HTTP client initialized with URL: %s : %d",
                   http_config.url.c_str(), http_config.port);

  http.addHeader("Content-Type", http_config.content_type);

  if (http_config.auth_type == "none") {
    g_Logger.Write(LogLevel::Debug, LogCategory::HTTP, "HttpClient_PostData",
                     "No authentication configured for HTTP client.");
  } else if (http_config.auth_type == "basic") {
    if (http_config.username.isEmpty() || http_config.password.isEmpty()) {
      g_Logger.Write(LogLevel::Error, LogCategory::DEVICE, "HttpClient_PostData",
                       "Username or password is empty for basic auth.");
    } else {
      http.setAuthorization(http_config.username.c_str(),
                            http_config.password.c_str());
    }
  } else if (http_config.auth_type == "bearer") {
    http.addHeader("Authorization", "Bearer " + http_config.token);
  }

  String payload = "{\"pump1\":1,\"oht\":87}";

  g_Logger.Write(LogLevel::Debug, LogCategory::DEVICE, "HttpClient_PostData",
                        "HTTP POST to %s with payload: %s",
                        http_config.url.c_str(), payload.c_str());

  int httpResponseCode = http.POST(payload);

  if (httpResponseCode > 0) {
    String response = http.getString();
    g_Logger.Write(LogLevel::Debug, LogCategory::HTTP, "HttpClient_PostData",
                     "HTTP POST Response [%d]: %s", httpResponseCode,
                     response.c_str());
  } else {
    g_Logger.Write(LogLevel::Error, LogCategory::DEVICE, "HttpClient_PostData",
                     "HTTP POST failed: %s",
                     http.errorToString(httpResponseCode).c_str());
  }

  http.end();
}

void HttpClient_CreateTask() {
  xTaskCreatePinnedToCore(
      HttpClient_ApplicationTask,  // Task function
      "HttpClientTask",            // Task name
      ANCIT_HTTP_APP_STACK_SIZE,    // Stack size in words
      NULL,                        // Task parameter
      ANCIT_HTTP_APP_PRIORITY,      // Task priority
      NULL,                        // Task handle (optional)
      ANCIT_HTTP_APP_TASK_CORE      // Core to run the task on (0 or 1)
  );

  g_Logger.Write(LogLevel::Debug, LogCategory::DEVICE, "HttpClient_CreateTask",
                   "HTTP client task created successfully");
}

void HttpClient_ApplicationTask(void* param) {
  // Initialize HTTP config
  HttpClient_LoadFromJson(devComm_doc);

  if (!http_config.enabled) {
    g_Logger.Write(LogLevel::Warn, LogCategory::HTTP, "ApplicationTask",
                     "HTTP is disabled in config, exiting task");
    vTaskDelete(NULL);
  }

  // Wait for WiFi before proceeding
  int wifiWaitLogCount = 0;
  while (!globals.wifi.got_ip) {
    if (wifiWaitLogCount < 3) {
      g_Logger.Write(LogLevel::Debug, LogCategory::HTTP, "ApplicationTask",
                       "Waiting for WiFi connection...");
      wifiWaitLogCount++;
    }
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
  g_Logger.Write(LogLevel::Debug, LogCategory::HTTP, "ApplicationTask",
                   "WiFi connected, proceeding with HTTP setup");

  while (true) {
    g_Logger.Write(LogLevel::Debug, LogCategory::HTTP, "ApplicationTask",
                     "HTTP client task running");
    HttpClient_PostData();
    vTaskDelay(pdMS_TO_TICKS(http_config.tx_interval * 1000));
  }
}
