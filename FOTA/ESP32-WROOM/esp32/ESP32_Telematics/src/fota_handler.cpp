#include "fota_handler.h"
#include "led_handler.h"
#include "utils.h"

FotaHandler fotaHandler;

FotaHandler::FotaHandler() {
  fotaBufferIndex = 0;
  expectedTotalChunks = 0;
  receivedChunks = 0;
  fotaState = FOTA_IDLE;
}

void FotaHandler::init() {
  memset(fotaBuffer, 0, sizeof(fotaBuffer));
}

void FotaHandler::handleMessage(char* topic, byte* payload, unsigned int length) {
  String topicStr = String(topic);
  DebugLog.Write(LogLevel::Info, LogCategory::MQTT, "handleMessage", "FOTA topic: " + topicStr);

  String payloadStr = "";
  for (int i = 0; i < length; i++) {
    payloadStr += (char)payload[i];
  }
  DebugLog.Write(LogLevel::Info, LogCategory::MQTT, "handleMessage", "FOTA payload length: " + String(payloadStr.length()));

  if (topicStr.indexOf("metadata") >= 0) {
    handleMetadata(payloadStr);
  } else if (topicStr.indexOf("chunk") >= 0) {
    handleChunk(payloadStr);
  } else if (topicStr.indexOf("complete") >= 0) {
    handleComplete();
  }
}

void FotaHandler::handleMetadata(String payloadStr) {
  DebugLog.Write(LogLevel::Info, LogCategory::MQTT, "handleMetadata", "FOTA metadata received");

  DynamicJsonDocument doc(METADATA_JSON_SIZE);
  DeserializationError error = deserializeJson(doc, payloadStr);

  if (error) {
    DebugLog.Write(LogLevel::Error, LogCategory::MQTT, "handleMetadata", "Failed to parse metadata JSON: " + String(error.c_str()));
    return;
  }

  expectedTotalChunks = doc["total_chunks"];
  receivedChunks = 0;
  fotaBufferIndex = 0;
  memset(fotaBuffer, 0, sizeof(fotaBuffer));
  fotaState = FOTA_RECEIVING;

  DebugLog.Write(LogLevel::Info, LogCategory::MQTT, "handleMetadata", "FOTA transfer started - expecting " + String(expectedTotalChunks) + " chunks");

  ledHandler.flashGreen(1, 200);
}

void FotaHandler::handleChunk(String payloadStr) {
  if (fotaState != FOTA_RECEIVING) {
    DebugLog.Write(LogLevel::Warn, LogCategory::MQTT, "handleChunk", "Received chunk but no transfer in progress");
    return;
  }

  DynamicJsonDocument doc(CHUNK_JSON_SIZE);
  DeserializationError error = deserializeJson(doc, payloadStr);

  if (error) {
    DebugLog.Write(LogLevel::Error, LogCategory::MQTT, "handleChunk", "Failed to parse chunk JSON: " + String(error.c_str()));
    return;
  }

  int chunkNum = doc["chunk_num"];
  String chunkData = doc["data"];

  if (fotaBufferIndex + chunkData.length() < sizeof(fotaBuffer)) {
    memcpy(fotaBuffer + fotaBufferIndex, chunkData.c_str(), chunkData.length());
    fotaBufferIndex += chunkData.length();
    receivedChunks++;

    DebugLog.Write(LogLevel::Debug, LogCategory::MQTT, "handleChunk", "Received chunk " + String(chunkNum) + " (" + String(receivedChunks) + "/" + String(expectedTotalChunks) + ") - Buffer: " + String(fotaBufferIndex) + " bytes");
  } else {
    DebugLog.Write(LogLevel::Error, LogCategory::MQTT, "handleChunk", "ERROR: Buffer overflow! Chunk " + String(chunkNum) + " would exceed buffer size");
  }
}

void FotaHandler::handleComplete() {
  DebugLog.Write(LogLevel::Info, LogCategory::MQTT, "handleComplete", "FOTA transfer complete signal received");

  if (receivedChunks == expectedTotalChunks) {
    DebugLog.Write(LogLevel::Info, LogCategory::MQTT, "handleComplete", "FOTA file assembled successfully!");
    DebugLog.Write(LogLevel::Info, LogCategory::MQTT, "handleComplete", "Total size: " + String(fotaBufferIndex) + " bytes");

    String preview = "";
    for (int i = 0; i < min(100, fotaBufferIndex); i++) {
      preview += fotaBuffer[i];
    }
    DebugLog.Write(LogLevel::Debug, LogCategory::MQTT, "handleComplete", "File preview: " + preview + "...");

    ledHandler.flashGreen(6, 100);

    fotaState = FOTA_READY_TO_TRANSMIT;

  } else {
    DebugLog.Write(LogLevel::Error, LogCategory::MQTT, "handleComplete", "FOTA transfer incomplete! Received " + String(receivedChunks) + "/" + String(expectedTotalChunks) + " chunks");
    fotaState = FOTA_IDLE;
  }
}

bool FotaHandler::isReadyToTransmit() {
  return fotaState == FOTA_READY_TO_TRANSMIT;
}

void FotaHandler::setTransmissionComplete() {
  fotaState = FOTA_IDLE;
}

char* FotaHandler::getBuffer() {
  return fotaBuffer;
}

int FotaHandler::getBufferSize() {
  return fotaBufferIndex;
}

void FotaHandler::reset() {
  fotaBufferIndex = 0;
  expectedTotalChunks = 0;
  receivedChunks = 0;
  fotaState = FOTA_IDLE;
  memset(fotaBuffer, 0, sizeof(fotaBuffer));
}