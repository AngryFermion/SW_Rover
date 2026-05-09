#include "uart_bootloader.h"
#include "fota_serial_handler.h"
#include "config.h"
#include "utils.h"
#include "SMART_LOGGER.h"

extern SMART_LOGGER DebugLog;

UartBootloader uartBootloader;

UartBootloader::UartBootloader() {
}

#define BOOT_DEBUG_MODE 0

BootloaderResult UartBootloader::executeBootloaderSequence(char* srecBuffer, int bufferSize) {
#if BOOT_DEBUG_MODE == 0
  if (srecBuffer == nullptr) {
    publishProgressStub("error_null_buffer");
    return BOOTLOADER_ERROR_INVALID_PARAMETERS;
  }

  if (bufferSize <= 0) {
    publishProgressStub("error_invalid_buffer_size");
    return BOOTLOADER_ERROR_INVALID_PARAMETERS;
  }
#endif

  publishProgressStub("bootloader_sequence_started");

  // VH: Commented out for testing 
  // Step 1: Send SBOOT command
  // BootloaderResult result = sendCommandAndWaitOK("SBOOT");
  // if (result != BOOTLOADER_SUCCESS) {
  //   publishProgressStub("boot_cmd_failed");
  //   return result;
  // }
  // publishProgressStub("boot_cmd_successful");
  // delay(BOOTLOADER_SBOOT_DELAY_MS); // 1 second delay after SBOOT command

  // // Step 2: Send SPROGRAM command
  // result = sendCommandAndWaitOK("SPROGRAM");
  // if (result != BOOTLOADER_SUCCESS) {
  //   publishProgressStub("program_cmd_failed");
  //   return result;
  // }
  // publishProgressStub("program_cmd_successful");
  // delay(BOOTLOADER_COMMAND_DELAY_MS); // 10ms delay after SPROGRAM command

  // Step 3: Send SREC lines using existing FotaSerialHandler
  publishProgressStub("srec_transmission_started");
  fotaSerialHandler.transmitFotaData(srecBuffer, bufferSize);

  // Note: We assume transmitFotaData succeeded if it returns normally
  // In future, this could be enhanced to check return value from transmitFotaData
  publishProgressStub("srec_transmission_completed");

  // // Step 4: Send SVERIFY command
  // result = sendCommandAndWaitOK("SVERIFY");
  // if (result != BOOTLOADER_SUCCESS) {
  //   publishProgressStub("verify_cmd_failed");
  //   return result;
  // }
  // publishProgressStub("verify_cmd_successful");
  // delay(BOOTLOADER_COMMAND_DELAY_MS); // 10ms delay after SVERIFY command

  // // Step 5: Send SRESET command
  // result = sendCommandAndWaitOK("SRESET");
  // if (result != BOOTLOADER_SUCCESS) {
  //   publishProgressStub("reset_cmd_failed");
  //   return result;
  // }
  // publishProgressStub("reset_cmd_successful");
  // delay(BOOTLOADER_COMMAND_DELAY_MS); // 10ms delay after SRESET command

  publishProgressStub("bootloader_sequence_completed");
  return BOOTLOADER_SUCCESS;
}

BootloaderResult UartBootloader::sendCommandAndWaitOK(const String& command) {
  // Send command
  Serial1.println(command);

  // Wait for OK response
  unsigned long startTime = millis();
  String response = "";

  while (millis() - startTime < FOTA_TIMEOUT_MS) {
    if (Serial1.available()) {
      char c = Serial1.read();
      response += c;

      if (response.endsWith("OK\r\n") || response.endsWith("OK\n")) {
        return BOOTLOADER_SUCCESS;
      }

      // Keep response buffer manageable
      if (response.length() > 20) {
        response = response.substring(response.length() - 10);
      }
    }
    delay(10);
  }

  // Timeout occurred - determine which command failed
  if (command == "SBOOT") {
    return BOOTLOADER_ERROR_SBOOT_FAILED;
  } else if (command == "SPROGRAM") {
    return BOOTLOADER_ERROR_SPROGRAM_FAILED;
  } else if (command == "SVERIFY") {
    return BOOTLOADER_ERROR_SVERIFY_FAILED;
  } else if (command == "SRESET") {
    return BOOTLOADER_ERROR_SRESET_FAILED;
  }

  return BOOTLOADER_ERROR_SBOOT_FAILED; // Default fallback
}

void UartBootloader::publishProgressStub(const String& status) {
  DebugLog.Write(LogLevel::Info, LogCategory::SYSTEM, "UartBootloader", status);
}