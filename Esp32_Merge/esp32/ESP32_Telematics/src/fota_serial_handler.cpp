#include "fota_serial_handler.h"
#include "mqtt_handler.h"
#include "led_handler.h"
#include "utils.h"

FotaSerialHandler fotaSerialHandler;

FotaSerialHandler::FotaSerialHandler() {
}

void FotaSerialHandler::init() {
  Serial1.begin(FOTA_SERIAL_BAUD_RATE, SERIAL_8N1, FOTA_SERIAL_RX_PIN, FOTA_SERIAL_TX_PIN);
  DebugLog.Write(LogLevel::Info, LogCategory::SETUP, "init", "Serial1 initialized for FOTA transmission");
}

void FotaSerialHandler::transmitFotaData(char* buffer, int bufferSize) {
  DebugLog.Write(LogLevel::Info, LogCategory::SYSTEM, "transmitFotaData", "Starting FOTA data transmission via Serial1");

  publishProgress("transmission_started");

  int lineCount = 0;
  int bufferPos = 0;
  int timeoutCount = 0;

  while (bufferPos < bufferSize) {
    if (buffer[bufferPos] == 'S') {
      int lineStart = bufferPos;
      int lineEnd = lineStart;

      while (lineEnd < bufferSize) {
        if (buffer[lineEnd] == '\n') {
          break;
        }
        lineEnd++;
      }

      if (lineEnd < bufferSize) {
        String line = "";
        for (int i = lineStart; i < lineEnd; i++) {
          if (buffer[i] != '\r' && buffer[i] != '\n') {
            line += buffer[i];
          }
        }

        if (line.length() > 0) {
          lineCount++;

#if 1 // WAIT FOR OK - DISABLED FOR TESTING (change to #if 1 to enable)
          int retryCount = 0;
          bool lineAcknowledged = false;

          while (!lineAcknowledged && retryCount < 3) {
            Serial1.println(line);
            DebugLog.Write(LogLevel::Info, LogCategory::SYSTEM, "transmitFotaData", "Sent line " + String(lineCount) + " (attempt " + String(retryCount + 1) + "/3): " + line.substring(0, 20) + "...");

            delay(TRANSMISSION_DELAY);

            bool okReceived = false;
            unsigned long startTime = millis();
            String response = "";

            DebugLog.Write(LogLevel::Debug, LogCategory::SYSTEM, "transmitFotaData", "Waiting for OK response...");

            while (millis() - startTime < FOTA_TIMEOUT_MS) {
              if (Serial1.available()) {
                char c = Serial1.read();
                response += c;
                DebugLog.Write(LogLevel::Debug, LogCategory::SYSTEM, "transmitFotaData", "Received char: 0x" + String(c, HEX) + " (" + String(c) + ")");

                // if (response.endsWith("OK\r\n") || response.endsWith("OK\n")) {
                //   okReceived = true;
                //   DebugLog.Write(LogLevel::Info, LogCategory::SYSTEM, "transmitFotaData", "Received OK for line " + String(lineCount) + ", response: " + response);
                //   break;
                // }
                if (response.endsWith("AOK") || response.endsWith("OK\n")) {
                  okReceived = true;
                  DebugLog.Write(LogLevel::Info, LogCategory::SYSTEM, "transmitFotaData", "Received OK for line " + String(lineCount) + ", response: " + response);
                  break;
                }

                if (response.length() > 20) {
                  response = response.substring(response.length() - 10);
                }
              }
              delay(10);
            }

            if (okReceived) {
              lineAcknowledged = true;
              timeoutCount = 0;
              publishProgress("OK", lineCount);
            } else {
              DebugLog.Write(LogLevel::Error, LogCategory::SYSTEM, "transmitFotaData", "TIMEOUT waiting for OK response for line " + String(lineCount) + " - ABORTING TRANSMISSION");
              timeoutCount = 3;
              break;
            }
          }

          if (!lineAcknowledged) {
            break;
          }
          delay(TRANSMISSION_DELAY);
#else
          // Send once without waiting for OK
          Serial1.println(line);
          DebugLog.Write(LogLevel::Debug, LogCategory::SYSTEM, "transmitFotaData", "Sent line " + String(lineCount) + ": " + line.substring(0, 20) + "...");
          delay(TRANSMISSION_DELAY);
#endif

          publishProgress("SENT", lineCount);
        }

        bufferPos = lineEnd + 1;
      } else {
        break;
      }
    } else {
      bufferPos++;
    }
  }

  if (timeoutCount >= 3) {
    DebugLog.Write(LogLevel::Error, LogCategory::SYSTEM, "transmitFotaData", "FOTA transmission aborted - " + String(lineCount) + " lines sent before timeout");
    publishProgress("transmission_failed", lineCount);
    ledHandler.flashRed(5, 200);
  } else {
    DebugLog.Write(LogLevel::Info, LogCategory::SYSTEM, "transmitFotaData", "FOTA transmission complete - " + String(lineCount) + " lines sent");
    publishProgress("transmission_complete", lineCount);
    ledHandler.flashBlue(3, 200);
  }
}

void FotaSerialHandler::publishProgress(const String& status, int lineCount) {
  if (mqttHandler.isConnected()) {
    String progressTopic = String(topic_smartwheels) + "/progress";
    String progressMessage;

    if (status == "transmission_started") {
      progressMessage = "{\"status\":\"transmission_started\",\"timestamp\":\"" + getTimestamp() + "\"}";
    } else if (status == "transmission_failed") {
      progressMessage = "{\"status\":\"transmission_failed\",\"lines_sent\":" + String(lineCount) + ",\"timestamp\":\"" + getTimestamp() + "\"}";
    } else if (status == "transmission_complete") {
      progressMessage = "{\"status\":\"transmission_complete\",\"total_lines\":" + String(lineCount) + ",\"timestamp\":\"" + getTimestamp() + "\"}";
    } else {
      progressMessage = "{\"line\":" + String(lineCount) + ",\"status\":\"" + status + "\",\"timestamp\":\"" + getTimestamp() + "\"}";
    }

    mqttHandler.publish(progressTopic.c_str(), progressMessage.c_str());
    DebugLog.Write(LogLevel::Debug, LogCategory::MQTT, "publishProgress", "Published progress: " + status + (lineCount > 0 ? " line " + String(lineCount) : ""));
  }
}