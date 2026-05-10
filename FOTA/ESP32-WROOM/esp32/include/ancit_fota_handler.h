#ifndef _HEADER_ANCIT_FOTA_HANDLER_H_
#define _HEADER_ANCIT_FOTA_HANDLER_H_

#include <Arduino.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>

// FOTA Configuration
#define FOTA_FOLDER "/fota"               // FOTA folder
#define FOTA_TEMP_FILE "/fota/temp.srec"  // Temporary file for FOTA data
#define DEFAULT_CHUNK_SIZE 4096           // Default 4KB chunk size
#define MAX_FOTA_FILE_SIZE (220 * 1024)   // Maximum FOTA file size: 200KB
#define MAX_CHUNKS (MAX_FOTA_FILE_SIZE / DEFAULT_CHUNK_SIZE + 1)  // Calculate max chunks based on file size and chunk size

// Debug Configuration
#define FOTA_DEBUG_PRESERVE_FILE 1        // Set to 1 to preserve temp file for debugging, 0 to delete on init/reset

enum FotaState {
  FOTA_IDLE,
  FOTA_RECEIVING,
  FOTA_READY_TO_TRANSMIT
};

class FotaHandler {
private:
  File fotaFile;  // SPIFFS file handle for temporary storage
  int expectedTotalChunks;
  int receivedChunks;
  int expectedFileSize;
  int chunkSize;  // Chunk size from metadata
  FotaState fotaState;
  bool receivedChunkFlags[MAX_CHUNKS];  // Track which chunks received

public:
  FotaHandler();
  void init();
  void handleMessage(char* topic, byte* payload, unsigned int length);
  bool isReadyToTransmit();
  void setTransmissionComplete();
  File& getFile();
  int getFileSize();
  void reset();

  // Getters for status reporting
  FotaState getState() const;
  int getReceivedChunks() const;
  int getExpectedChunks() const;

private:
  void handleMetadata(String payloadStr);
  void handleChunk(String topicStr, byte* payload, unsigned int length);
  void handleComplete();
  void publishChunkAck(int chunkNum, bool success);
};

extern FotaHandler fotaHandler;

// MQTT Base topic for all FOTA operations
#define MQTT_BASE_TOPIC_FOTA "SmartWheelsNS"

// MQTT Subscription topics (wildcard patterns for listening)
extern const char* topic_fota;
extern const char* topic_data;
extern const char* topic_smartwheels;

// MQTT Topic Definitions for publishing ACKs
extern const char TOPIC_FOTA_ACK[];

// MQTT Integration functions
void fota_init_topics(const String& deviceId);
void fota_setup_params(void);
void fota_init_serial_handler(void);
void fota_publish_status(void);
void fota_subscribe_topics(void);
void fota_mqtt_init(void);
void fota_handle_mqtt_message(char* topic, byte* payload, unsigned int length);

#endif  //_HEADER_ANCIT_FOTA_HANDLER_H_