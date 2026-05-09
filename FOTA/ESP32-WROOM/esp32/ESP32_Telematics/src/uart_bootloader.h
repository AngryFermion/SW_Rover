#ifndef UART_BOOTLOADER_H
#define UART_BOOTLOADER_H

#include <Arduino.h>

// Bootloader command delays
#define BOOTLOADER_SBOOT_DELAY_MS     1000  // 1 second delay after SBOOT command
#define BOOTLOADER_COMMAND_DELAY_MS   10    // 10ms delay after other commands

enum BootloaderResult {
  BOOTLOADER_SUCCESS = 0,
  BOOTLOADER_ERROR_SBOOT_FAILED,
  BOOTLOADER_ERROR_SPROGRAM_FAILED,
  BOOTLOADER_ERROR_SREC_TRANSMISSION_FAILED,
  BOOTLOADER_ERROR_SVERIFY_FAILED,
  BOOTLOADER_ERROR_SRESET_FAILED,
  BOOTLOADER_ERROR_INVALID_PARAMETERS
};

class UartBootloader {
public:
  UartBootloader();

  BootloaderResult executeBootloaderSequence(char* srecBuffer, int bufferSize);

private:
  BootloaderResult sendCommandAndWaitOK(const String& command);
  void publishProgressStub(const String& status);
};

extern UartBootloader uartBootloader;

#endif