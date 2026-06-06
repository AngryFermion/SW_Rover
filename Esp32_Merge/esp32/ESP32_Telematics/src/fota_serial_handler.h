#ifndef FOTA_SERIAL_HANDLER_H
#define FOTA_SERIAL_HANDLER_H

#include <Arduino.h>
#include "config.h"

class FotaSerialHandler {
public:
  FotaSerialHandler();
  void init();
  void transmitFotaData(char* buffer, int bufferSize);

private:
  void publishProgress(const String& status, int lineCount = 0);
};

extern FotaSerialHandler fotaSerialHandler;

#endif