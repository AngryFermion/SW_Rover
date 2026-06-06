/*
 * Multiple Message Filtering Example
 *
 * This example demonstrates advanced message filtering capabilities
 * of the ESP32_CAN library. It shows how to filter for specific
 * message IDs and handle different types of messages.
 *
 * Hardware Requirements:
 * - ESP32 board
 * - CAN transceiver (e.g., SN65HVD230)
 * - CAN bus with devices sending messages with IDs: 0x201, 0x202, 0x203
 *
 * Connections:
 * - ESP32 GPIO5 -> CAN TX
 * - ESP32 GPIO4 -> CAN RX
 *
 * Author: ESP32 Creators
 * Date: September 2025
 */

#include "ESP32_CAN.h"

// Create CAN instance
ESP32_CAN can_bus(5, 4);

// Message counters
uint16_t temperature_count = 0;
uint16_t pressure_count = 0;
uint16_t humidity_count = 0;
uint16_t unknown_count = 0;

// Handle temperature sensor data (ID: 0x201)
void handleTemperatureData(const can_message_t* message) {
    temperature_count++;

    if (message->length >= 2) {
        // Assuming temperature is sent as uint16_t (little endian, °C * 10)
        uint16_t temp_raw = (message->data[1] << 8) | message->data[0];
        float temperature = temp_raw / 10.0;

        Serial.print("[TEMP] ");
        Serial.print(temperature);
        Serial.print("°C (Count: ");
        Serial.print(temperature_count);
        Serial.println(")");
    } else {
        Serial.println("[TEMP] Invalid data length");
    }
}

// Handle pressure sensor data (ID: 0x202)
void handlePressureData(const can_message_t* message) {
    pressure_count++;

    if (message->length >= 4) {
        // Assuming pressure is sent as uint32_t (little endian, Pa)
        uint32_t pressure_raw = (message->data[3] << 24) | (message->data[2] << 16) |
                               (message->data[1] << 8) | message->data[0];
        float pressure_kpa = pressure_raw / 1000.0;

        Serial.print("[PRESS] ");
        Serial.print(pressure_kpa);
        Serial.print(" kPa (Count: ");
        Serial.print(pressure_count);
        Serial.println(")");
    } else {
        Serial.println("[PRESS] Invalid data length");
    }
}

// Handle humidity sensor data (ID: 0x203)
void handleHumidityData(const can_message_t* message) {
    humidity_count++;

    if (message->length >= 1) {
        // Assuming humidity is sent as uint8_t (0-100%)
        uint8_t humidity = message->data[0];

        Serial.print("[HUMID] ");
        Serial.print(humidity);
        Serial.print("% (Count: ");
        Serial.print(humidity_count);
        Serial.println(")");
    } else {
        Serial.println("[HUMID] Invalid data length");
    }
}

// Main message dispatcher
void onMessageReceived(const can_message_t* message) {
    // Print raw message info
    Serial.print("RX: ID=0x");
    Serial.print(message->id, HEX);
    Serial.print(" [");
    for (int i = 0; i < message->length; i++) {
        if (i > 0) Serial.print(" ");
        if (message->data[i] < 0x10) Serial.print("0");
        Serial.print(message->data[i], HEX);
    }
    Serial.print("] ");

    // Route to specific handlers
    switch (message->id) {
        case 0x201:
            handleTemperatureData(message);
            break;

        case 0x202:
            handlePressureData(message);
            break;

        case 0x203:
            handleHumidityData(message);
            break;

        default:
            unknown_count++;
            Serial.print("[UNKNOWN] ID: 0x");
            Serial.print(message->id, HEX);
            Serial.print(" (Count: ");
            Serial.print(unknown_count);
            Serial.println(")");
            break;
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("ESP32 CAN Multiple Message Filtering Example");
    Serial.println("============================================");

    // Initialize CAN bus
    if (!can_bus.begin(CAN_SPEED_500KBPS)) {
        Serial.println("ERROR: CAN initialization failed!");
        while (1) delay(1000);
    }

    Serial.println("CAN bus initialized at 500 kbps");

    // Method 1: Filter for specific sensor IDs
    Serial.println("Setting up message filters...");
    uint32_t sensor_ids[] = {0x201, 0x202, 0x203};
    if (can_bus.setMultipleFilter(sensor_ids, 3)) {
        Serial.println("✓ Multiple ID filter configured for:");
        Serial.println("  - 0x201: Temperature sensor");
        Serial.println("  - 0x202: Pressure sensor");
        Serial.println("  - 0x203: Humidity sensor");
    } else {
        Serial.println("✗ Failed to set multiple filter");
    }

    // Alternative Method 2: Range filter (uncomment to use instead)
    /*
    if (can_bus.setRangeFilter(0x200, 0x20F)) {
        Serial.println("✓ Range filter configured for IDs 0x200-0x20F");
    }
    */

    // Alternative Method 3: Accept all messages (uncomment to use instead)
    /*
    can_bus.clearFilter();
    Serial.println("✓ No filter - accepting all messages");
    */

    // Set message callback
    can_bus.setReceiveCallback(onMessageReceived);

    Serial.println();
    Serial.println("Listening for sensor messages...");
    Serial.println("Expected message formats:");
    Serial.println("  0x201: [TempLow TempHigh] - Temperature in °C * 10");
    Serial.println("  0x202: [P0 P1 P2 P3] - Pressure in Pa (little endian)");
    Serial.println("  0x203: [Humidity] - Humidity 0-100%");
    Serial.println();
}

void loop() {
    // Process incoming messages
    can_bus.checkMessages();

    // Send test data every 5 seconds for demonstration
    static unsigned long lastTestTime = 0;
    if (millis() - lastTestTime >= 5000) {
        Serial.println("--- Sending test messages ---");

        // Send temperature data (25.6°C)
        uint16_t temp = 256;  // 25.6°C * 10
        uint8_t temp_data[] = {(uint8_t)(temp & 0xFF), (uint8_t)(temp >> 8)};
        can_bus.sendStandardFrame(0x201, temp_data, 2);

        delay(100);

        // Send pressure data (101325 Pa = 1 atm)
        uint32_t pressure = 101325;
        uint8_t press_data[] = {
            (uint8_t)(pressure & 0xFF),
            (uint8_t)((pressure >> 8) & 0xFF),
            (uint8_t)((pressure >> 16) & 0xFF),
            (uint8_t)((pressure >> 24) & 0xFF)
        };
        can_bus.sendStandardFrame(0x202, press_data, 4);

        delay(100);

        // Send humidity data (65%)
        uint8_t humid_data[] = {65};
        can_bus.sendStandardFrame(0x203, humid_data, 1);

        lastTestTime = millis();
    }

    // Print statistics every 15 seconds
    static unsigned long lastStatsTime = 0;
    if (millis() - lastStatsTime >= 15000) {
        Serial.println();
        Serial.println("=== Message Statistics ===");
        Serial.print("Temperature messages: ");
        Serial.println(temperature_count);
        Serial.print("Pressure messages: ");
        Serial.println(pressure_count);
        Serial.print("Humidity messages: ");
        Serial.println(humidity_count);
        Serial.print("Unknown messages: ");
        Serial.println(unknown_count);
        Serial.print("Total errors: ");
        Serial.println(can_bus.getErrorCount());
        Serial.println();

        lastStatsTime = millis();
    }

    delay(10);
}