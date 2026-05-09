/*
 * CAN Gateway Example
 *
 * This example demonstrates how to create a CAN gateway that receives
 * messages from one CAN network and forwards them to another, or
 * processes them for logging/monitoring purposes.
 *
 * Features:
 * - Message forwarding between networks
 * - Message logging and monitoring
 * - Filtering and routing
 * - Error handling and diagnostics
 *
 * Hardware Requirements:
 * - ESP32 board
 * - Two CAN transceivers for dual CAN interface (optional)
 * - Or single CAN interface for monitoring/logging
 *
 * Connections:
 * - ESP32 GPIO5 -> CAN TX
 * - ESP32 GPIO4 -> CAN RX
 *
 * Author: ESP32 Creators
 * Date: September 2025
 */

#include "ESP32_CAN.h"

// Create CAN interface
ESP32_CAN can_network(5, 4);

// Gateway statistics
struct GatewayStats {
    uint32_t messages_received;
    uint32_t messages_forwarded;
    uint32_t messages_filtered;
    uint32_t errors;
    uint32_t uptime_seconds;
} stats = {0};

// Message types for routing
enum MessageType {
    TYPE_SENSOR_DATA,
    TYPE_CONTROL_CMD,
    TYPE_STATUS_INFO,
    TYPE_DIAGNOSTIC,
    TYPE_UNKNOWN
};

// Get message type based on CAN ID
MessageType getMessageType(uint32_t can_id) {
    if (can_id >= 0x100 && can_id <= 0x1FF) {
        return TYPE_SENSOR_DATA;
    } else if (can_id >= 0x200 && can_id <= 0x2FF) {
        return TYPE_CONTROL_CMD;
    } else if (can_id >= 0x300 && can_id <= 0x3FF) {
        return TYPE_STATUS_INFO;
    } else if (can_id >= 0x700 && can_id <= 0x7FF) {
        return TYPE_DIAGNOSTIC;
    }
    return TYPE_UNKNOWN;
}

// Get message type name
const char* getMessageTypeName(MessageType type) {
    switch (type) {
        case TYPE_SENSOR_DATA: return "SENSOR";
        case TYPE_CONTROL_CMD: return "CONTROL";
        case TYPE_STATUS_INFO: return "STATUS";
        case TYPE_DIAGNOSTIC: return "DIAGNOSTIC";
        default: return "UNKNOWN";
    }
}

// Log message to serial in human-readable format
void logMessage(const can_message_t* message, const char* direction) {
    MessageType type = getMessageType(message->id);

    Serial.print("[");
    Serial.print(direction);
    Serial.print("] ");
    Serial.print(getMessageTypeName(type));
    Serial.print(" | ID:0x");
    Serial.print(message->id, HEX);
    Serial.print(" | Len:");
    Serial.print(message->length);
    Serial.print(" | Data:");

    for (int i = 0; i < message->length; i++) {
        Serial.print(" ");
        if (message->data[i] < 0x10) Serial.print("0");
        Serial.print(message->data[i], HEX);
    }

    if (message->extended) {
        Serial.print(" [EXT]");
    }

    // Add timestamp
    Serial.print(" | T:");
    Serial.print(millis());

    Serial.println();
}

// Process sensor data messages
void processSensorData(const can_message_t* message) {
    switch (message->id) {
        case 0x101: // Temperature sensor
            if (message->length >= 2) {
                int16_t temp = (message->data[1] << 8) | message->data[0];
                Serial.print("  → Temperature: ");
                Serial.print(temp / 10.0);
                Serial.println("°C");
            }
            break;

        case 0x102: // Pressure sensor
            if (message->length >= 4) {
                uint32_t pressure = (message->data[3] << 24) | (message->data[2] << 16) |
                                   (message->data[1] << 8) | message->data[0];
                Serial.print("  → Pressure: ");
                Serial.print(pressure / 1000.0);
                Serial.println(" kPa");
            }
            break;

        case 0x103: // Humidity sensor
            if (message->length >= 1) {
                Serial.print("  → Humidity: ");
                Serial.print(message->data[0]);
                Serial.println("%");
            }
            break;

        default:
            Serial.print("  → Unknown sensor ID: 0x");
            Serial.println(message->id, HEX);
            break;
    }
}

// Process control command messages
void processControlCommand(const can_message_t* message) {
    if (message->length >= 2) {
        uint8_t device_id = message->data[0];
        uint8_t command = message->data[1];

        Serial.print("  → Device ");
        Serial.print(device_id);
        Serial.print(", Command: 0x");
        Serial.print(command, HEX);

        if (message->length >= 3) {
            Serial.print(", Value: ");
            Serial.print(message->data[2]);
        }

        Serial.println();

        // Echo acknowledgment (simulate gateway forwarding)
        uint8_t ack_data[] = {device_id, command, 0x00}; // 0x00 = ACK
        can_network.sendStandardFrame(0x380, ack_data, 3);
        stats.messages_forwarded++;
    }
}

// Main message handler
void onCanMessageReceived(const can_message_t* message) {
    stats.messages_received++;

    // Log the message
    logMessage(message, "RX");

    // Determine message type and process accordingly
    MessageType type = getMessageType(message->id);

    switch (type) {
        case TYPE_SENSOR_DATA:
            processSensorData(message);
            // Forward to monitoring system (example)
            stats.messages_forwarded++;
            break;

        case TYPE_CONTROL_CMD:
            processControlCommand(message);
            break;

        case TYPE_STATUS_INFO:
            Serial.print("  → Status from node: ");
            Serial.println(message->data[0]);
            break;

        case TYPE_DIAGNOSTIC:
            Serial.print("  → Diagnostic code: 0x");
            Serial.println(message->data[0], HEX);
            break;

        case TYPE_UNKNOWN:
            stats.messages_filtered++;
            Serial.println("  → Filtered (unknown type)");
            break;
    }

    Serial.println();
}

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("ESP32 CAN Gateway Example");
    Serial.println("=========================");

    // Initialize CAN interface
    if (!can_network.begin(CAN_SPEED_500KBPS)) {
        Serial.println("ERROR: CAN initialization failed!");
        while (1) delay(1000);
    }

    Serial.println("CAN Gateway initialized successfully!");
    Serial.println("Speed: 500 kbps");
    Serial.println();

    // Configure filtering for gateway operation
    // Accept all messages for gateway functionality
    can_network.clearFilter();

    // Set message callback
    can_network.setReceiveCallback(onCanMessageReceived);

    Serial.println("Gateway running - monitoring all CAN traffic");
    Serial.println("Message routing:");
    Serial.println("  0x100-0x1FF: Sensor data (logged + forwarded)");
    Serial.println("  0x200-0x2FF: Control commands (processed + ACK)");
    Serial.println("  0x300-0x3FF: Status information (logged)");
    Serial.println("  0x700-0x7FF: Diagnostic messages (logged)");
    Serial.println("  Others: Filtered");
    Serial.println();
}

void loop() {
    // Process incoming messages
    can_network.checkMessages();

    // Send periodic gateway heartbeat
    static unsigned long lastHeartbeat = 0;
    if (millis() - lastHeartbeat >= 5000) {
        // Send gateway status (ID: 0x3F0)
        uint8_t status_data[] = {
            0x01,                                    // Gateway online
            (uint8_t)(stats.messages_received & 0xFF),      // Message count (low byte)
            (uint8_t)((stats.messages_received >> 8) & 0xFF), // Message count (high byte)
            (uint8_t)(can_network.getErrorCount() & 0xFF)   // Error count
        };

        can_network.sendStandardFrame(0x3F0, status_data, 4);
        lastHeartbeat = millis();
    }

    // Send test messages for demonstration
    static unsigned long lastTestMsg = 0;
    if (millis() - lastTestMsg >= 10000) {
        Serial.println("--- Sending test messages ---");

        // Simulate temperature sensor
        uint16_t temp = 235; // 23.5°C
        uint8_t temp_data[] = {(uint8_t)(temp & 0xFF), (uint8_t)(temp >> 8)};
        can_network.sendStandardFrame(0x101, temp_data, 2);

        delay(100);

        // Simulate control command
        uint8_t ctrl_data[] = {0x05, 0x02, 0x80}; // Device 5, Command 2, Value 128
        can_network.sendStandardFrame(0x250, ctrl_data, 3);

        lastTestMsg = millis();
    }

    // Print statistics every 30 seconds
    static unsigned long lastStatsTime = 0;
    if (millis() - lastStatsTime >= 30000) {
        stats.uptime_seconds = millis() / 1000;

        Serial.println("=== Gateway Statistics ===");
        Serial.print("Uptime: ");
        Serial.print(stats.uptime_seconds);
        Serial.println(" seconds");
        Serial.print("Messages received: ");
        Serial.println(stats.messages_received);
        Serial.print("Messages forwarded: ");
        Serial.println(stats.messages_forwarded);
        Serial.print("Messages filtered: ");
        Serial.println(stats.messages_filtered);
        Serial.print("CAN errors: ");
        Serial.println(can_network.getErrorCount());

        // Calculate message rate
        if (stats.uptime_seconds > 0) {
            float msg_rate = (float)stats.messages_received / stats.uptime_seconds;
            Serial.print("Average message rate: ");
            Serial.print(msg_rate);
            Serial.println(" msg/sec");
        }

        Serial.println();
        lastStatsTime = millis();
    }

    delay(10);
}