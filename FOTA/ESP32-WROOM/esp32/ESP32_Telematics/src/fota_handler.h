#ifndef FOTA_HANDLER_H
#define FOTA_HANDLER_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include "config.h"

enum FotaState {
  FOTA_IDLE,
  FOTA_RECEIVING,
  FOTA_READY_TO_TRANSMIT
};

class FotaHandler {
private:
  char fotaBuffer[FOTA_BUFFER_SIZE];
  int fotaBufferIndex;
  int expectedTotalChunks;
  int receivedChunks;
  FotaState fotaState;

public:
  FotaHandler();
  void init();
  void handleMessage(char* topic, byte* payload, unsigned int length);
  bool isReadyToTransmit();
  void setTransmissionComplete();
  char* getBuffer();
  int getBufferSize();
  void reset();

private:
  void handleMetadata(String payloadStr);
  void handleChunk(String payloadStr);
  void handleComplete();
};

extern FotaHandler fotaHandler;

#endif