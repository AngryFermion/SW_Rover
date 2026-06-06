#include "ancit_fota_serial_handler.h"
#include "AncitRgbLed.h"
#include <BT_LOGGER.h>
#include <esp_task_wdt.h>

// Global instance
AncitFotaSerialHandler ancitFotaSerialHandler;

AncitFotaSerialHandler::AncitFotaSerialHandler() {
  transmissionInProgress = false;
  totalLines = 0;
  currentLine = 0;
  currentBootloadStep = 0;
  onProgressCallback = nullptr;
}

void AncitFotaSerialHandler::init() {
  // Initialize Serial2 for FOTA transmission
  FOTA_SERIAL_HW.begin(FOTA_SERIAL_BAUD_RATE, SERIAL_8N1, FOTA_SERIAL_RX_PIN, FOTA_SERIAL_TX_PIN);

  g_Logger.Write(LogLevel::Debug, LogCategory::SER, "AncitFotaSerialHandler::init",
                   "Serial2 initialized for FOTA transmission (baud: %d, RX: %d, TX: %d)",
                   FOTA_SERIAL_BAUD_RATE, FOTA_SERIAL_RX_PIN, FOTA_SERIAL_TX_PIN);
}

void AncitFotaSerialHandler::startTransmission() {
  if (!fotaHandler.isReadyToTransmit()) {
    g_Logger.Write(LogLevel::Error, LogCategory::SER, "AncitFotaSerialHandler::startTransmission",
                     "No FOTA file ready for transmission");
    if (onProgressCallback) onProgressCallback("error_no_file", 0);
    return;
  }

  if (transmissionInProgress) {
    g_Logger.Write(LogLevel::Warn, LogCategory::SER, "AncitFotaSerialHandler::startTransmission",
                     "Transmission already in progress");
    if (onProgressCallback) onProgressCallback("error_transmission_busy", 0);
    return;
  }

  g_Logger.Write(LogLevel::Debug, LogCategory::SER, "AncitFotaSerialHandler::startTransmission",
                   "Starting FOTA transmission via Serial");
  transmitFotaFile();
}

void AncitFotaSerialHandler::transmitFotaFile() {
  if (!fotaHandler.isReadyToTransmit()) {
    g_Logger.Write(LogLevel::Error, LogCategory::SER, "AncitFotaSerialHandler::transmitFotaFile",
                     "FOTA handler not ready to transmit");
    return;
  }

  totalLines = 0;
  currentLine = 0;

  if (onProgressCallback) onProgressCallback("transmission_started", 0);
  // net_led_set_state(LED_COLOR_BLUE, LED_MODE_BLINK_FAST);

  // Get file from FOTA handler
  File& fotaFile = fotaHandler.getFile();
  if (!fotaFile) {
    g_Logger.Write(LogLevel::Error, LogCategory::SER, "AncitFotaSerialHandler::transmitFotaFile",
                     "Failed to open FOTA file for reading");
    if (onProgressCallback) onProgressCallback("transmission_failed", 0);
    return;
  }

  // Reset file position to beginning
  fotaFile.seek(0);

  String line = "";
  bool transmissionFailed = false;

  // Unsubscribe current task from watchdog during long FOTA transmission
  TaskHandle_t currentTask = xTaskGetCurrentTaskHandle();
  esp_task_wdt_delete(currentTask);

  g_Logger.Write(LogLevel::Debug, LogCategory::SER, "AncitFotaSerialHandler::transmitFotaFile",
                   "Starting transmission of %d bytes (WDT disabled for this task)", fotaFile.size());

  while (fotaFile.available() && !transmissionFailed) {

    char c = fotaFile.read();

    if (c == '\n' || c == '\r') {
      if (line.length() > 0 && line.startsWith("S")) {
        totalLines++;
        currentLine = totalLines;

        // Publish progress for first line only
        if (totalLines == 1 && onProgressCallback) {
          onProgressCallback("line_ok", totalLines);
        }

        // Check if this is a metadata record (S1 or S5) that doesn't need acknowledgment
        bool isMetadataRecord = (line.startsWith("S1") || line.startsWith("S5"));

        bool lineSuccess = false;
        int retryCount = 0;

        // Retry logic for each line
        while (!lineSuccess && retryCount < FOTA_MAX_RETRIES) {
          sendLine(line);

          g_Logger.Write(LogLevel::Debug, LogCategory::SER, "AncitFotaSerialHandler::transmitFotaFile",
                           "Sent line %d (attempt %d/%d): %s...",
                           totalLines, retryCount + 1, FOTA_MAX_RETRIES,
                           line.substring(0, 20).c_str());

          delay(FOTA_TRANSMISSION_DELAY);

          // Skip waiting for OK response on S1/S5 metadata records
          if (isMetadataRecord) {
            lineSuccess = true;
            g_Logger.Write(LogLevel::Debug, LogCategory::SER, "AncitFotaSerialHandler::transmitFotaFile",
                             "Line %d (metadata) sent without ACK", totalLines);
          } else {
            // Wait for OK response for data records
            if (waitForOkResponse()) {
              lineSuccess = true;
              g_Logger.Write(LogLevel::Debug, LogCategory::SER, "AncitFotaSerialHandler::transmitFotaFile",
                               "Line %d acknowledged", totalLines);
            } else {
              retryCount++;
              g_Logger.Write(LogLevel::Warn, LogCategory::SER, "AncitFotaSerialHandler::transmitFotaFile",
                               "Line %d failed, retry %d/%d", totalLines, retryCount, FOTA_MAX_RETRIES);
            }
          }
        }

        if (!lineSuccess) {
          g_Logger.Write(LogLevel::Error, LogCategory::SER, "AncitFotaSerialHandler::transmitFotaFile",
                           "Line %d failed after %d retries - aborting transmission",
                           totalLines, FOTA_MAX_RETRIES);
          transmissionFailed = true;
          break;
        }

        delay(FOTA_TRANSMISSION_DELAY);
      }
      line = "";
    } else {
      line += c;
    }

    // Yield to prevent watchdog timeout
    if (totalLines % 10 == 0) {
      vTaskDelay(pdMS_TO_TICKS(1));
    }
  }

  // Handle last line if it doesn't end with newline
  if (line.length() > 0 && line.startsWith("S") && !transmissionFailed) {
    totalLines++;
    sendLine(line);
    if (!waitForOkResponse()) {
      transmissionFailed = true;
    }
  }

  // Re-subscribe task to watchdog after transmission
  esp_task_wdt_add(currentTask);

  // Cleanup and final status
  if (transmissionFailed) {
    g_Logger.Write(LogLevel::Error, LogCategory::SER, "AncitFotaSerialHandler::transmitFotaFile",
                     "FOTA transmission failed - %d lines sent before failure", totalLines);
    if (onProgressCallback) onProgressCallback("transmission_failed", totalLines);
    netLed->setState(LedMode::BLINK_ONCE, LedColor::RED);
  } else {
    g_Logger.Write(LogLevel::Debug, LogCategory::SER, "AncitFotaSerialHandler::transmitFotaFile",
                     "FOTA transmission completed successfully - %d lines sent", totalLines);

    // Report last line progress if more than 1 line was transmitted
    if (totalLines > 1 && onProgressCallback) {
      onProgressCallback("line_ok", totalLines);
    }

    if (onProgressCallback) onProgressCallback("transmission_complete", totalLines);
    netLed->setState(LedMode::BLINK_ONCE, LedColor::GREEN);

    // Mark transmission as complete in FOTA handler
    fotaHandler.setTransmissionComplete();
  }

  g_Logger.Write(LogLevel::Debug, LogCategory::SER, "AncitFotaSerialHandler::transmitFotaFile",
                   "WDT re-enabled for task");
}

void AncitFotaSerialHandler::sendLine(const String& line) {
  FOTA_SERIAL_HW.println(line);
}

bool AncitFotaSerialHandler::sendBootloaderCommand(const String& command) {
  g_Logger.Write(LogLevel::Debug, LogCategory::SER, "AncitFotaSerialHandler::sendBootloaderCommand",
                   "Sending bootloader command: %s", command.c_str());

  int retryCount = 0;
  const int maxRetries = 3; // Total 3 attempts

  while (retryCount < maxRetries) {
    // Send command
    FOTA_SERIAL_HW.println(command);

    g_Logger.Write(LogLevel::Debug, LogCategory::SER, "AncitFotaSerialHandler::sendBootloaderCommand",
                     "Command '%s' sent (attempt %d/%d)", command.c_str(), retryCount + 1, maxRetries);

    // Wait for OK response
    if (waitForOkResponse()) {
      g_Logger.Write(LogLevel::Debug, LogCategory::SER, "AncitFotaSerialHandler::sendBootloaderCommand",
                       "Command '%s' acknowledged", command.c_str());
      return true;
    }

    retryCount++;
    if (retryCount < maxRetries) {
      g_Logger.Write(LogLevel::Warn, LogCategory::SER, "AncitFotaSerialHandler::sendBootloaderCommand",
                       "Command '%s' failed, retrying (%d/%d)", command.c_str(), retryCount, maxRetries - 1);
      delay(500); // Brief delay before retry
    }
  }

  g_Logger.Write(LogLevel::Error, LogCategory::SER, "AncitFotaSerialHandler::sendBootloaderCommand",
                   "Command '%s' failed after %d attempts", command.c_str(), maxRetries);
  return false;
}

void AncitFotaSerialHandler::executeBootloadSequence() {
  if (!fotaHandler.isReadyToTransmit()) {
    g_Logger.Write(LogLevel::Error, LogCategory::SER, "AncitFotaSerialHandler::executeBootloadSequence",
                     "No FOTA file ready for bootload sequence");
    if (onProgressCallback) onProgressCallback("error_no_file", 0);
    return;
  }

  if (transmissionInProgress) {
    g_Logger.Write(LogLevel::Warn, LogCategory::SER, "AncitFotaSerialHandler::executeBootloadSequence",
                     "Bootload sequence already in progress");
    if (onProgressCallback) onProgressCallback("error_transmission_busy", 0);
    return;
  }

  g_Logger.Write(LogLevel::Debug, LogCategory::SER, "AncitFotaSerialHandler::executeBootloadSequence",
                   "Starting bootload sequence");

  transmissionInProgress = true;
  bool sequenceFailed = false;

#if 0  
  // Step 1: Send SBOOT command
  currentBootloadStep = BOOTLOAD_BOOT;
  if (onProgressCallback) onProgressCallback("bootload_boot", BOOTLOAD_BOOT);
  if (!sendBootloaderCommand("SBOOT")) {
    sequenceFailed = true;
    goto sequence_cleanup;
  }

  // Step 2: Send SPROGRAM command
  currentBootloadStep = BOOTLOAD_PROGRAM;
  if (onProgressCallback) onProgressCallback("bootload_program", BOOTLOAD_PROGRAM);
  if (!sendBootloaderCommand("SPROGRAM")) {
    sequenceFailed = true;
    goto sequence_cleanup;
  }
#endif

  // Step 3: Send SREC file (reuse existing transmission logic)
  currentBootloadStep = BOOTLOAD_FILE_TRANSFER;
  if (onProgressCallback) onProgressCallback("bootload_file_transfer", BOOTLOAD_FILE_TRANSFER);
  transmitFotaFile(); // This handles the file transmission with line-by-line OK responses

#if 0  
  // Step 4: Send SVERIFY command
  currentBootloadStep = BOOTLOAD_VERIFY;
  if (onProgressCallback) onProgressCallback("bootload_verify", BOOTLOAD_VERIFY);
  if (!sendBootloaderCommand("SVERIFY")) {
    sequenceFailed = true;
    goto sequence_cleanup;
  }

  // Step 5: Send SRESET command
  currentBootloadStep = BOOTLOAD_RESET;
  if (onProgressCallback) onProgressCallback("bootload_reset", BOOTLOAD_RESET);
  if (!sendBootloaderCommand("SRESET")) {
    sequenceFailed = true;
    goto sequence_cleanup;
  }
#endif

sequence_cleanup:
  transmissionInProgress = false;
  currentBootloadStep = 0;

  if (sequenceFailed) {
    g_Logger.Write(LogLevel::Error, LogCategory::SER, "AncitFotaSerialHandler::executeBootloadSequence",
                     "Bootload sequence failed");
    if (onProgressCallback) onProgressCallback("bootload_failed", 0);
    netLed->setState(LedMode::BLINK_ONCE, LedColor::RED);
  } else {
    g_Logger.Write(LogLevel::Debug, LogCategory::SER, "AncitFotaSerialHandler::executeBootloadSequence",
                     "Bootload sequence completed successfully");
    if (onProgressCallback) onProgressCallback("bootload_complete", 0);
    netLed->setState(LedMode::BLINK_ONCE, LedColor::GREEN);

    // Mark transmission as complete in FOTA handler
    fotaHandler.setTransmissionComplete();
  }
}

bool AncitFotaSerialHandler::waitForOkResponse() {
  unsigned long startTime = millis();
  String response = "";

  while (millis() - startTime < FOTA_TIMEOUT_MS) {
    if (FOTA_SERIAL_HW.available()) {
      char c = FOTA_SERIAL_HW.read();
      response += c;

      // Check for OK response
      if (response.endsWith("AOK") ) {
        g_Logger.Write(LogLevel::Debug, LogCategory::SER, "AncitFotaSerialHandler::waitForOkResponse",
                         "Received OK response: %s", response.c_str());
        return true;
      }

      // Limit response buffer size
      if (response.length() > 20) {
        response = response.substring(response.length() - 10);
      }
    }
    delay(10);
  }

  g_Logger.Write(LogLevel::Error, LogCategory::SER, "AncitFotaSerialHandler::waitForOkResponse",
                   "Timeout waiting for OK response. Last response: %s", response.c_str());
  return false;
}

bool AncitFotaSerialHandler::isTransmissionInProgress() {
  return transmissionInProgress;
}

int AncitFotaSerialHandler::getCurrentLine() const {
  return currentLine;
}

int AncitFotaSerialHandler::getCurrentBootloadStep() const {
  return currentBootloadStep;
}
