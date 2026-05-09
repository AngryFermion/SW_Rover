/*
 * Basic CAN Communication Example
 *
 * This example demonstrates basic CAN bus communication using the ESP32_CAN library.
 * It sends periodic messages and receives any incoming messages.
 *
 * Hardware Requirements:
 * - ESP32 board
 * - CAN transceiver (e.g., SN65HVD230)
 * - CAN bus with other devices
 *
 * Connections:
 * - ESP32 GPIO5 -> CAN TX
 * - ESP32 GPIO4 -> CAN RX
 * - CAN transceiver connected to CAN bus
 *
 * Author: ESP32 Creators
 * Date: September 2025
 */

#include "ESP32_CAN.h"

// Create CAN instance (TX pin, RX pin)
ESP32_CAN can_bus(5, 4);

// Message received callback
void onMessageReceived(const can_message_t* message) {
    Serial.print("Received CAN message - ID: 0x");
    Serial.print(message->id, HEX);
    Serial.print(", Length: ");
    Serial.print(message->length);
    Serial.print(", Data: ");

    for (int i = 0; i < message->length; i++) {
        if (message->data[i] < 0x10) Serial.print("0");
        Serial.print(message->data[i], HEX);
        Serial.print(" ");
    }

    if (message->extended) {
        Serial.print(" [Extended Frame]");
    }

    Serial.println();
}

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("ESP32 CAN Basic Communication Example");
    Serial.println("=====================================");

    // Initialize CAN bus at 500 kbps
    if (!can_bus.begin(CAN_SPEED_500KBPS)) {
        Serial.println("ERROR: CAN initialization failed!");
        Serial.println("Check your wiring and CAN transceiver");
        while (1) {
            delay(1000);
        }
    }

    Serial.println("CAN bus initialized successfully!");
    Serial.println("Speed: 500 kbps");
    Serial.println("TX Pin: GPIO5, RX Pin: GPIO4");
    Serial.println();

    // Set callback for received messages
    can_bus.setReceiveCallback(onMessageReceived);

    Serial.println("Starting communication...");
    Serial.println("Sending periodic messages and listening for responses");
    Serial.println();
}

void loop() {
    // Process incoming CAN messages
    can_bus.checkMessages();

    // Send a test message every 2 seconds
    static unsigned long lastTxTime = 0;
    static uint8_t counter = 0;

    if (millis() - lastTxTime >= 2000) {
        // Prepare test data
        uint8_t data[4];
        data[0] = 0xAA;                    // Start marker
        data[1] = counter++;               // Incrementing counter
        data[2] = (millis() >> 8) & 0xFF;  // High byte of timestamp
        data[3] = millis() & 0xFF;         // Low byte of timestamp

        // Send standard frame
        if (can_bus.sendStandardFrame(0x123, data, 4)) {
            Serial.print("Sent message #");
            Serial.print(counter - 1);
            Serial.print(" - Data: ");
            for (int i = 0; i < 4; i++) {
                if (data[i] < 0x10) Serial.print("0");
                Serial.print(data[i], HEX);
                Serial.print(" ");
            }
            Serial.println();
        } else {
            Serial.println("ERROR: Failed to send message");
        }

        lastTxTime = millis();
    }

    // Print status every 10 seconds
    static unsigned long lastStatusTime = 0;
    if (millis() - lastStatusTime >= 10000) {
        Serial.println("--- CAN Bus Status ---");
        can_bus.printStatus();
        Serial.println();
        lastStatusTime = millis();
    }

    delay(10);  // Small delay for stability
}