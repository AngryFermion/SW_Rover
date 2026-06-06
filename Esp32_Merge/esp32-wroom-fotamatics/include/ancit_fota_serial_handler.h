#ifndef _HEADER_ANCIT_FOTA_SERIAL_HANDLER_H_
#define _HEADER_ANCIT_FOTA_SERIAL_HANDLER_H_

#include <Arduino.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>
#include "ancit_pins.h"
#include "ancit_fota_handler.h"

// FOTA Serial Configuration
#define FOTA_TRANSMISSION_DELAY 50      // Delay between lines in ms
#define FOTA_TIMEOUT_MS 2000           // Timeout waiting for OK response
#define FOTA_MAX_RETRIES 3             // Max retries per line

// Bootload sequence steps
enum BootloadSequence {
  BOOTLOAD_BOOT = 1,
  BOOTLOAD_PROGRAM = 2,
  BOOTLOAD_FILE_TRANSFER = 3,
  BOOTLOAD_VERIFY = 4,
  BOOTLOAD_RESET = 5
};

class AncitFotaSerialHandler {
private:
  bool transmissionInProgress;
  int totalLines;
  int currentLine;
  int currentBootloadStep;

  bool waitForOkResponse();
  void sendLine(const String& line);
  bool sendBootloaderCommand(const String& command);

public:
  AncitFotaSerialHandler();
  void init();
  void transmitFotaFile();
  bool isTransmissionInProgress();
  void startTransmission();
  void executeBootloadSequence();
  int getCurrentLine() const;
  int getCurrentBootloadStep() const;

  // Callback for progress reporting (to be called by main FOTA handler)
  void (*onProgressCallback)(const String& status, int lineCount);
};

extern AncitFotaSerialHandler ancitFotaSerialHandler;

#endif // _HEADER_ANCIT_FOTA_SERIAL_HANDLER_H_