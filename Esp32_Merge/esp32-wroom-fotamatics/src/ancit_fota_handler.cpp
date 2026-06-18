#include "ancit_fota_handler.h"
#include "ancit_fota_serial_handler.h"
#include "AncitRgbLed.h"
#include "ancit_mqtt_client.h"
#include "ancit_device.h"
#include <config_reader.h>
#include <BT_LOGGER.h>

// Global FOTA handler instance
FotaHandler fotaHandler;

// AWS IoT Topic constants (for subscription)
const char* topic_fota = "sdv/vehicles/sdv-vehicle-001/fota/#";
const char* topic_data = "sdv/vehicles/sdv-vehicle-001/data/#";
const char* topic_smartwheels = "sdv/vehicles/sdv-vehicle-001/#";

// Dynamic MQTT topics - built at runtime using device ID
static String TOPIC_SERVER_FOTA_METADATA;
static String TOPIC_SERVER_FOTA_CHUNK;
static String TOPIC_SERVER_FOTA_COMPLETE;
static String TOPIC_SERVER_FOTA_BOOTLOAD;
static String TOPIC_DEVICE_FOTA_ACK;
static String TOPIC_DEVICE_FOTA_PROGRESS;
static String TOPIC_DEVICE_STATUS;
static String TOPIC_SERVER_ALL;

// Device ID variable - to be set from device configuration
static String DEVICE_ID = "sdv-vehicle-001";  // AWS IoT Thing name

// Function to check if using AWS IoT (defined in ancit_mqtt_client.cpp)
extern bool useEmbeddedAWSCerts;

// Function to initialize MQTT topics with device ID
void fota_init_topics(const String& deviceId) {
  // Use AWS IoT Thing name for AWS, or custom device ID for other brokers
  if (useEmbeddedAWSCerts) {
    DEVICE_ID = "sdv-vehicle-001";  // AWS IoT Thing name

    // AWS IoT topic structure: sdv/vehicles/<thing-name>/fota/...
    TOPIC_SERVER_FOTA_METADATA = "sdv/vehicles/" + DEVICE_ID + "/fota/metadata";
    TOPIC_SERVER_FOTA_CHUNK = "sdv/vehicles/" + DEVICE_ID + "/fota/chunk";
    TOPIC_SERVER_FOTA_COMPLETE = "sdv/vehicles/" + DEVICE_ID + "/fota/complete";
    TOPIC_SERVER_FOTA_BOOTLOAD = "sdv/vehicles/" + DEVICE_ID + "/fota/bootload";

    // Device → Server topics (AWS IoT format)
    TOPIC_DEVICE_FOTA_ACK = "sdv/vehicles/" + DEVICE_ID + "/fota/ack";
    TOPIC_DEVICE_FOTA_PROGRESS = "sdv/vehicles/" + DEVICE_ID + "/fota/progress";
    TOPIC_DEVICE_STATUS = "sdv/vehicles/" + DEVICE_ID + "/status";

    // Subscription topic (already subscribed in ancit_mqtt_client.cpp)
    TOPIC_SERVER_ALL = "sdv/vehicles/" + DEVICE_ID + "/#";

    g_Logger.Write(LogLevel::Info, LogCategory::FOTA, "fota_init_topics",
                     "AWS IoT topics initialized for Thing: %s", DEVICE_ID.c_str());
  } else {
    // Use custom device ID for non-AWS brokers
    DEVICE_ID = "ABCD";

    // Build Server → Device topics (SmartWheelsNS format)
    TOPIC_SERVER_FOTA_METADATA = String(MQTT_BASE_TOPIC_FOTA) + "/server/" + DEVICE_ID + "/fota/metadata";
    TOPIC_SERVER_FOTA_CHUNK = String(MQTT_BASE_TOPIC_FOTA) + "/server/" + DEVICE_ID + "/fota/chunk";
    TOPIC_SERVER_FOTA_COMPLETE = String(MQTT_BASE_TOPIC_FOTA) + "/server/" + DEVICE_ID + "/fota/complete";
    TOPIC_SERVER_FOTA_BOOTLOAD = String(MQTT_BASE_TOPIC_FOTA) + "/server/" + DEVICE_ID + "/fota/bootload";

    // Build Device → Server topics
    TOPIC_DEVICE_FOTA_ACK = String(MQTT_BASE_TOPIC_FOTA) + "/device/" + DEVICE_ID + "/fota/ack";
    TOPIC_DEVICE_FOTA_PROGRESS = String(MQTT_BASE_TOPIC_FOTA) + "/device/" + DEVICE_ID + "/fota/progress";
    TOPIC_DEVICE_STATUS = String(MQTT_BASE_TOPIC_FOTA) + "/device/" + DEVICE_ID + "/status";

    // Build subscription topic
    TOPIC_SERVER_ALL = String(MQTT_BASE_TOPIC_FOTA) + "/server/" + DEVICE_ID + "/#";

    g_Logger.Write(LogLevel::Debug, LogCategory::FOTA, "fota_init_topics",
                     "MQTT topics initialized for device ID: %s", DEVICE_ID.c_str());
  }
}

FotaHandler::FotaHandler() {
  expectedTotalChunks = 0;
  receivedChunks = 0;
  expectedFileSize = 0;
  chunkSize = DEFAULT_CHUNK_SIZE;
  fotaState = FOTA_IDLE;
}

void FotaHandler::init() {
  // Create FOTA folder if it doesn't exist
  if (!SPIFFS.exists(FOTA_FOLDER)) {
    File fotaDir = SPIFFS.open(FOTA_FOLDER, "w");
    if (fotaDir) {
      fotaDir.close();
      g_Logger.Write(LogLevel::Debug, LogCategory::FOTA, "FotaHandler::init",
                       "Created FOTA folder: %s", FOTA_FOLDER);
    } else {
      g_Logger.Write(LogLevel::Error, LogCategory::FOTA, "FotaHandler::init",
                       "Failed to create FOTA folder: %s", FOTA_FOLDER);
    }
  }

  // Clear any existing temporary file (controlled by debug flag)
#if FOTA_DEBUG_PRESERVE_FILE == 0
  if (SPIFFS.exists(FOTA_TEMP_FILE)) {
    SPIFFS.remove(FOTA_TEMP_FILE);
    g_Logger.Write(LogLevel::Debug, LogCategory::FOTA, "FotaHandler::init",
                     "Removed existing temp file: %s", FOTA_TEMP_FILE);
  }
#else
  if (SPIFFS.exists(FOTA_TEMP_FILE)) {
    g_Logger.Write(LogLevel::Debug, LogCategory::FOTA, "FotaHandler::init",
                     "Preserving existing temp file for debugging: %s", FOTA_TEMP_FILE);
  }
#endif

  memset(receivedChunkFlags, false, sizeof(receivedChunkFlags));
  g_Logger.Write(LogLevel::Debug, LogCategory::FOTA, "FotaHandler::init",
                   "FOTA Handler initialized with SPIFFS storage (max %d chunks, max %dKB file size)",
                   MAX_CHUNKS, MAX_FOTA_FILE_SIZE / 1024);
}

void FotaHandler::handleMessage(char* topic, byte* payload, unsigned int length) {
  String topicStr = String(topic);

  if (topicStr.indexOf("metadata") >= 0) {
    String payloadStr = "";
    for (int i = 0; i < length; i++) {
      payloadStr += (char)payload[i];
    }
    handleMetadata(payloadStr);
  } else if (topicStr.indexOf("chunk") >= 0) {
    handleChunk(topicStr, payload, length);
  } else if (topicStr.indexOf("complete") >= 0) {
    handleComplete();
  }
}

void FotaHandler::handleMetadata(String payloadStr) {
  g_Logger.Write(LogLevel::Info, LogCategory::FOTA, "FotaHandler::handleMetadata",
                   "FOTA metadata received");

  // If we're in IDLE or READY_TO_TRANSMIT state, reset to accept new transfer
  if (fotaState == FOTA_IDLE || fotaState == FOTA_READY_TO_TRANSMIT) {
    g_Logger.Write(LogLevel::Info, LogCategory::FOTA, "FotaHandler::handleMetadata",
                     "Resetting FOTA handler for new transfer");
    reset();
  }

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, payloadStr);

  if (error) {
    g_Logger.Write(LogLevel::Error, LogCategory::FOTA, "FotaHandler::handleMetadata",
                     "Failed to parse metadata JSON: %s", error.c_str());
    return;
  }

  expectedTotalChunks = doc["total_chunks"];
  expectedFileSize = doc["content_length"];
  chunkSize = doc["chunk_size"] | DEFAULT_CHUNK_SIZE;  // Use metadata chunk size or default
  receivedChunks = 0;

  // Validate chunk count
  if (expectedTotalChunks > MAX_CHUNKS) {
    g_Logger.Write(LogLevel::Error, LogCategory::FOTA, "FotaHandler::handleMetadata",
                     "Too many chunks: %d (max %d)", expectedTotalChunks, MAX_CHUNKS);
    return;
  }

  memset(receivedChunkFlags, false, sizeof(receivedChunkFlags));

  // Close any existing file and create new one
  if (fotaFile) {
    fotaFile.close();
  }

  // Remove existing temp file
  if (SPIFFS.exists(FOTA_TEMP_FILE)) {
    SPIFFS.remove(FOTA_TEMP_FILE);
  }

  // Create new temp file for writing
  fotaFile = SPIFFS.open(FOTA_TEMP_FILE, "w");
  if (!fotaFile) {
    g_Logger.Write(LogLevel::Error, LogCategory::FOTA, "FotaHandler::handleMetadata",
                     "Failed to create FOTA temp file");
    return;
  }

  fotaState = FOTA_RECEIVING;

  g_Logger.Write(LogLevel::Info, LogCategory::FOTA, "FotaHandler::handleMetadata",
                   "FOTA transfer started - expecting %d chunks (%d bytes, chunk size: %d)",
                   expectedTotalChunks, expectedFileSize, chunkSize);

  // Flash green LED to indicate FOTA start
  netLed->setState(LedMode::BLINK_ONCE, LedColor::GREEN);
}

  /**
   * @brief Handles incoming FOTA chunk data and stores it to SPIFFS
   *
   * This function processes individual firmware chunks received via MQTT:
   * - Extracts chunk number from topic (e.g., "SmartWheelsNS/fota/chunk/5")
   * - Validates chunk number against expected range
   * - Prevents duplicate chunk processing using receivedChunkFlags
   * - Writes chunk data to correct position in SPIFFS temp file
   * - Sends ACK response back to sender with success/failure status
   * - Tracks progress (receivedChunks vs expectedTotalChunks)
   *
   * @param topic MQTT topic containing chunk number (format: "...fota/chunk/{number}")
   * @param payload Raw chunk data bytes to be written to file
   * @param length Size of chunk data in bytes
   *
   * @note Function assumes FOTA session was initialized via handleMetadata()
   * @note Duplicate chunks are silently ignored to prevent file corruption
   * @note Each chunk is written to calculated file position based on chunk size
   */
void FotaHandler::handleChunk(String topicStr, byte* payload, unsigned int length) {
  if (fotaState != FOTA_RECEIVING) {
    g_Logger.Write(LogLevel::Warn, LogCategory::FOTA, "FotaHandler::handleChunk",
                     "Received chunk but no transfer in progress");
    return;
  }

  if (!fotaFile) {
    g_Logger.Write(LogLevel::Error, LogCategory::FOTA, "FotaHandler::handleChunk",
                     "FOTA file not open");
    return;
  }

  // Extract chunk number from topic (e.g., "SmartWheelsNS/fota/chunk/5" -> chunkNum = 5)
  int lastSlashIndex = topicStr.lastIndexOf('/');
  if (lastSlashIndex == -1) {
    g_Logger.Write(LogLevel::Error, LogCategory::FOTA, "FotaHandler::handleChunk",
                     "Invalid chunk topic format: %s", topicStr.c_str());
    return;
  }

  int chunkNum = topicStr.substring(lastSlashIndex + 1).toInt();

  // Check if chunk number is valid and not already received
  if (chunkNum < 0 || chunkNum >= MAX_CHUNKS) {
    g_Logger.Write(LogLevel::Error, LogCategory::FOTA, "FotaHandler::handleChunk",
                     "Invalid chunk number: %d (max %d)", chunkNum, MAX_CHUNKS);
    publishChunkAck(chunkNum, false);
    return;
  }

  if (receivedChunkFlags[chunkNum]) {
    // Silently ignore duplicates - no logging to reduce spam
    return;
  }

  // Calculate expected position in file for this chunk
  size_t expectedPosition = chunkNum * chunkSize;

  // Seek to the correct position in the file
  if (!fotaFile.seek(expectedPosition)) {
    g_Logger.Write(LogLevel::Error, LogCategory::FOTA, "FotaHandler::handleChunk",
                     "Failed to seek to position %d for chunk %d", expectedPosition, chunkNum);
    publishChunkAck(chunkNum, false);
    return;
  }

  // Write chunk data to file
  size_t bytesWritten = fotaFile.write(payload, length);
  if (bytesWritten != length) {
    g_Logger.Write(LogLevel::Error, LogCategory::FOTA, "FotaHandler::handleChunk",
                     "Write error: wrote %d/%d bytes for chunk %d", bytesWritten, length, chunkNum);
    publishChunkAck(chunkNum, false);
    return;
  }

  receivedChunks++;
  receivedChunkFlags[chunkNum] = true;

  g_Logger.Write(LogLevel::Info, LogCategory::FOTA, "FotaHandler::handleChunk",
                   "Chunk %d (%d/%d) - %d bytes written to position %d",
                   chunkNum, receivedChunks, expectedTotalChunks, length, expectedPosition);

  // Send acknowledgement for successful chunk reception
  publishChunkAck(chunkNum, true);
}

void FotaHandler::handleComplete() {
  g_Logger.Write(LogLevel::Info, LogCategory::FOTA, "FotaHandler::handleComplete",
                   "FOTA transfer complete signal received");

  if (receivedChunks == expectedTotalChunks) {
    // Close the file to finalize writes
    if (fotaFile) {
      fotaFile.close();
    }

    // Verify file exists and get actual size
    if (SPIFFS.exists(FOTA_TEMP_FILE)) {
      File verifyFile = SPIFFS.open(FOTA_TEMP_FILE, "r");
      if (verifyFile) {
        size_t actualSize = verifyFile.size();
        verifyFile.close();

        g_Logger.Write(LogLevel::Info, LogCategory::FOTA, "FotaHandler::handleComplete",
                         "FOTA file assembled successfully!");
        g_Logger.Write(LogLevel::Info, LogCategory::FOTA, "FotaHandler::handleComplete",
                         "File size: %d bytes (expected: %d bytes)", actualSize, expectedFileSize);

        // Flash green LED multiple times to indicate success
        // net_led_set_state(LED_COLOR_GREEN, LED_MODE_BLINK_FAST);

        fotaState = FOTA_READY_TO_TRANSMIT;
      } else {
        g_Logger.Write(LogLevel::Error, LogCategory::FOTA, "FotaHandler::handleComplete",
                         "Failed to verify FOTA file");
        fotaState = FOTA_IDLE;
      }
    } else {
      g_Logger.Write(LogLevel::Error, LogCategory::FOTA, "FotaHandler::handleComplete",
                       "FOTA temp file does not exist");
      fotaState = FOTA_IDLE;
    }
  } else {
    g_Logger.Write(LogLevel::Error, LogCategory::FOTA, "FotaHandler::handleComplete",
                     "FOTA transfer incomplete! Received %d/%d chunks",
                     receivedChunks, expectedTotalChunks);

    // Close and cleanup incomplete file
    if (fotaFile) {
      fotaFile.close();
    }
#if FOTA_DEBUG_PRESERVE_FILE == 0
    if (SPIFFS.exists(FOTA_TEMP_FILE)) {
      SPIFFS.remove(FOTA_TEMP_FILE);
    }
#else
    g_Logger.Write(LogLevel::Debug, LogCategory::FOTA, "FotaHandler::handleComplete",
                     "Preserving incomplete FOTA file for debugging: %s", FOTA_TEMP_FILE);
#endif

    fotaState = FOTA_IDLE;
  }
}

bool FotaHandler::isReadyToTransmit() {
  // Check if state is already set to ready
  if (fotaState == FOTA_READY_TO_TRANSMIT) {
    return true;
  }

  // Auto-detect existing FOTA file and set state accordingly
  if (SPIFFS.exists(FOTA_TEMP_FILE)) {
    File checkFile = SPIFFS.open(FOTA_TEMP_FILE, "r");
    if (checkFile && checkFile.size() > 0) {
      checkFile.close();

      // Set state to ready if file exists and has content
      fotaState = FOTA_READY_TO_TRANSMIT;

      g_Logger.Write(LogLevel::Debug, LogCategory::FOTA, "FotaHandler::isReadyToTransmit",
                       "Auto-detected existing FOTA file (%d bytes), state set to READY_TO_TRANSMIT",
                       getFileSize());

      return true;
    }
    if (checkFile) checkFile.close();
  }

  return false;
}

void FotaHandler::setTransmissionComplete() {
  // Close file if still open
  if (fotaFile) {
    fotaFile.close();
  }

  // Keep temp file for inspection/debugging - do not delete
  // if (SPIFFS.exists(FOTA_TEMP_FILE)) {
  //   SPIFFS.remove(FOTA_TEMP_FILE);
  // }

  fotaState = FOTA_IDLE;
  g_Logger.Write(LogLevel::Debug, LogCategory::FOTA, "FotaHandler::setTransmissionComplete",
                   "FOTA transmission marked as complete, temp file preserved at %s", FOTA_TEMP_FILE);
}

File& FotaHandler::getFile() {
  // Reopen file for reading if ready to transmit
  if (fotaState == FOTA_READY_TO_TRANSMIT && !fotaFile) {
    fotaFile = SPIFFS.open(FOTA_TEMP_FILE, "r");
  }
  return fotaFile;
}

int FotaHandler::getFileSize() {
  if (SPIFFS.exists(FOTA_TEMP_FILE)) {
    File sizeFile = SPIFFS.open(FOTA_TEMP_FILE, "r");
    if (sizeFile) {
      int size = sizeFile.size();
      sizeFile.close();
      return size;
    }
  }
  return 0;
}

FotaState FotaHandler::getState() const {
  return fotaState;
}

int FotaHandler::getReceivedChunks() const {
  return receivedChunks;
}

int FotaHandler::getExpectedChunks() const {
  return expectedTotalChunks;
}

void FotaHandler::reset() {
  // Close file if open
  if (fotaFile) {
    fotaFile.close();
  }

  // Remove temp file (controlled by debug flag)
#if FOTA_DEBUG_PRESERVE_FILE == 0
  if (SPIFFS.exists(FOTA_TEMP_FILE)) {
    SPIFFS.remove(FOTA_TEMP_FILE);
  }
#endif

  expectedTotalChunks = 0;
  receivedChunks = 0;
  expectedFileSize = 0;
  chunkSize = DEFAULT_CHUNK_SIZE;
  fotaState = FOTA_IDLE;
  memset(receivedChunkFlags, false, sizeof(receivedChunkFlags));

  g_Logger.Write(LogLevel::Info, LogCategory::FOTA, "FotaHandler::reset",
                   "FOTA handler reset%s",
#if FOTA_DEBUG_PRESERVE_FILE == 1
                   ", temp file preserved for debugging"
#else
                   ", temp file cleaned up"
#endif
                   );
}

void FotaHandler::publishChunkAck(int chunkNum, bool success) {
  if (!MqttClient_IsConnected()) {
    return;  // Skip ACK if not connected
  }

  JsonDocument ackDoc;
  ackDoc["chunk"] = chunkNum;
  ackDoc["status"] = success ? "ok" : "error";
  ackDoc["timestamp"] = millis();

  String ackPayload;
  serializeJson(ackDoc, ackPayload);

  bool published = MqttClient_Publish(TOPIC_DEVICE_FOTA_ACK.c_str(), ackPayload.c_str(), MQTT_NO_RETAIN);

  if (published) {
    g_Logger.Write(LogLevel::Debug, LogCategory::MQTT, "FotaHandler::publishChunkAck",
                     "ACK sent for chunk %d: %s", chunkNum, success ? "OK" : "ERROR");
  } else {
    g_Logger.Write(LogLevel::Error, LogCategory::MQTT, "FotaHandler::publishChunkAck",
                     "ACK FAILED for chunk %d — publish returned false", chunkNum);
  }
}

// MQTT Integration functions
void fota_serial_progress_callback(const String& status, int lineCount) {
  if (!MqttClient_IsConnected()) {
    return;  // Skip if MQTT not connected
  }

  JsonDocument progressDoc;
  progressDoc["status"] = status;
  progressDoc["timestamp"] = millis();

  // Check if this is a bootload sequence step (status starts with "bootload_")
  if (status.startsWith("bootload_") && lineCount > 0) {
    progressDoc["sequence"] = lineCount;  // Use lineCount as sequence number for bootload steps
  } else if (lineCount > 0) {
    progressDoc["line"] = lineCount;      // Use as line number for file transfer
  }

  String progressPayload;
  serializeJson(progressDoc, progressPayload);

  MqttClient_Publish(TOPIC_DEVICE_FOTA_PROGRESS.c_str(), progressPayload.c_str(), MQTT_NO_RETAIN);

  g_Logger.Write(LogLevel::Debug, LogCategory::MQTT, "fota_serial_progress_callback",
                   "Published progress: %s%s", status.c_str(),
                   lineCount > 0 ? (" line " + String(lineCount)).c_str() : "");
}

void fota_setup_params(void) {
  extern JsonDocument device_doc;

  // Get device ID from configuration
  String deviceId = device_doc["dev_id"].as<String>();
  if (deviceId.isEmpty()) {
    deviceId = "UNKNOWN";
    g_Logger.Write(LogLevel::Warn, LogCategory::FOTA, "fota_setup_params",
                     "Device ID not found in config, using fallback: %s", deviceId.c_str());
  }

  g_Logger.Write(LogLevel::Debug, LogCategory::FOTA, "fota_setup_params",
                   "Setting up FOTA parameters for device: %s", deviceId.c_str());

  // Initialize MQTT topics with device ID
  fota_init_topics(deviceId);

  // Initialize FOTA handler
  fotaHandler.init();
}

void fota_init_serial_handler(void) {
  // One-time initialization of serial handler
  static bool serialInitialized = false;

  if (!serialInitialized) {
    g_Logger.Write(LogLevel::Debug, LogCategory::FOTA, "fota_init_serial_handler",
                     "Initializing FOTA serial handler (one-time)");

    // Initialize FOTA serial handler
    ancitFotaSerialHandler.init();

    // Set up progress callback for serial handler
    ancitFotaSerialHandler.onProgressCallback = fota_serial_progress_callback;

    serialInitialized = true;
  } else {
    g_Logger.Write(LogLevel::Debug, LogCategory::FOTA, "fota_init_serial_handler",
                     "FOTA serial handler already initialized, skipping");
  }
}

void fota_publish_status(void) {
  // Publishing disabled for now - subscribe only mode
  g_Logger.Write(LogLevel::Debug, LogCategory::FOTA, "fota_publish_status",
                   "FOTA status publishing disabled (subscribe only mode)");

  /*
  extern MqttConfig mqtt_config;

  if (!MqttClient_IsConnected()) {
    g_Logger.Write(LogLevel::Error, LogCategory::FOTA, "fota_publish_status",
                     "MQTT client not connected, cannot publish status");
    return;
  }

  JsonDocument statusDoc;
  statusDoc["state"] = fotaHandler.getState();
  statusDoc["buffer_size"] = fotaHandler.getBufferSize();
  statusDoc["received_chunks"] = fotaHandler.getReceivedChunks();
  statusDoc["expected_chunks"] = fotaHandler.getExpectedChunks();
  statusDoc["ready_to_transmit"] = fotaHandler.isReadyToTransmit();

  String payload;
  serializeJson(statusDoc, payload);

  String fullTopic = String(TOPIC_FOTA_STATUS) + "/" + mqtt_config.clientId;

  MqttClient_Publish(fullTopic.c_str(), payload.c_str(), MQTT_NO_RETAIN);

  g_Logger.Write(LogLevel::Debug, LogCategory::FOTA, "fota_publish_status",
                   "Published FOTA status to %s (%d bytes)",
                   fullTopic.c_str(), payload.length());
  */
}

void fota_subscribe_topics(void) {
  if (!MqttClient_IsConnected()) {
    g_Logger.Write(LogLevel::Error, LogCategory::MQTT, "fota_subscribe_topics",
                     "MQTT client not connected, cannot subscribe");
    return;
  }

  if (TOPIC_SERVER_ALL.isEmpty()) {
    g_Logger.Write(LogLevel::Error, LogCategory::MQTT, "fota_subscribe_topics",
                     "Topics not initialized! Call fota_init_topics() first");
    return;
  }

  // Skip subscription for AWS IoT (already subscribed in ancit_mqtt_client.cpp)
  if (useEmbeddedAWSCerts) {
    g_Logger.Write(LogLevel::Info, LogCategory::MQTT, "fota_subscribe_topics",
                     "AWS IoT - subscription already handled in connection callback");
    return;
  }

  // Subscribe to all server topics for this device (non-AWS brokers)
  bool success = MqttClient_Subscribe(TOPIC_SERVER_ALL.c_str());
  if (success) {
    g_Logger.Write(LogLevel::Info, LogCategory::MQTT, "fota_subscribe_topics",
                     "Subscribed: %s", TOPIC_SERVER_ALL.c_str());
  }
}

void fota_mqtt_init(void) {
  // Setup FOTA parameters first (initializes topics)
  fota_setup_params();

  // Subscribe to FOTA topics (after topics are initialized)
  fota_subscribe_topics();

  // Initialize serial handler (one-time only)
  fota_init_serial_handler();

  g_Logger.Write(LogLevel::Debug, LogCategory::FOTA, "fota_mqtt_init",
                   "FOTA MQTT initialization completed (subscribe only)");
}

void fota_handle_mqtt_message(char* topic, byte* payload, unsigned int length) {
  String topicStr = String(topic);

  // Check if this is a message for this device
  // AWS IoT format: sdv/vehicles/sdv-vehicle-001/fota/...
  // Non-AWS format: SmartWheelsNS/server/ABCD/fota/...
  bool isForThisDevice = false;
  if (useEmbeddedAWSCerts) {
    // AWS IoT: check if topic contains the device ID
    isForThisDevice = (topicStr.indexOf("sdv/vehicles/" + DEVICE_ID + "/") >= 0);
  } else {
    // Non-AWS: check for /server/<device_id>/ pattern
    isForThisDevice = (topicStr.indexOf("/server/" + DEVICE_ID + "/") >= 0);
  }

  if (isForThisDevice) {
    // Handle FOTA bootload commands for transmission
    if (topicStr.equals(TOPIC_SERVER_FOTA_BOOTLOAD)) {
      String command = "";
      for (unsigned int i = 0; i < length; i++) {
        command += (char)payload[i];
      }
      command.trim();

      g_Logger.Write(LogLevel::Debug, LogCategory::FOTA, "fota_handle_mqtt_message",
                       "Received FOTA bootload command: %s", command.c_str());

      if (command == "serial") {
        if (ancitFotaSerialHandler.isTransmissionInProgress()) {
          g_Logger.Write(LogLevel::Warn, LogCategory::FOTA, "fota_handle_mqtt_message",
                           "Bootload sequence already in progress, ignoring command");
          return;
        }
        ancitFotaSerialHandler.executeBootloadSequence();
      } else if (command == "can") {
        g_Logger.Write(LogLevel::Debug, LogCategory::FOTA, "fota_handle_mqtt_message",
                         "CAN transport not yet implemented");
      } else {
        g_Logger.Write(LogLevel::Warn, LogCategory::FOTA, "fota_handle_mqtt_message",
                         "Unknown transport: %s", command.c_str());
      }
    } else {
      // Handle regular FOTA messages (metadata, chunks, complete)
      fotaHandler.handleMessage(topic, payload, length);
    }
  }
}
