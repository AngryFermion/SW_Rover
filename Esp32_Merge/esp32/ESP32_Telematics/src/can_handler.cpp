/**
 * can_handler.cpp - CAN Bus Message Handler Implementation
 *
 * Author: Narayana Swamy
 * Date: September 2025
 */

#include "can_handler.h"
#include "config.h"

#if ENABLE_CAN
// Global CAN bus instance
static SMART_CAN can_bus(CAN_TX_PIN, CAN_RX_PIN);
static unsigned long lastCANTx = 0;
static unsigned long lastTxErrorLog = 0;

// Initialize CAN bus system
bool initializeCAN() {
    myLogger->Write(LogLevel::Info, LOG_CAT_CAN, "initializeCAN", "Initializing CAN Bus Handler...");

    if (!can_bus.begin(CAN_SPEED)) {
        myLogger->Write(LogLevel::Error, LOG_CAT_CAN, "initializeCAN", "CAN initialization failed!");
        return false;
    }

    // Set callback for received messages
    can_bus.setReceiveCallback(onCANMessageReceived);
    myLogger->Write(LogLevel::Info, LOG_CAT_CAN, "initializeCAN", "CAN Bus Handler ready!");

    return true;
}

// Shutdown CAN bus
void shutdownCAN() {
    can_bus.end();
    myLogger->Write(LogLevel::Info, LOG_CAT_CAN, "shutdownCAN", "CAN Bus Handler shutdown");
}

// Send dummy test data via CAN
bool sendDummyData() {
    unsigned long currentTime = millis();

    if ((currentTime - lastCANTx >= CAN_TX_INTERVAL)) {
        // Skip sending if CAN is not properly initialized/connected
        if (!can_bus.isInitialized()) {
            lastCANTx = currentTime;
            return false;
        }

        uint8_t data[4];
        data[0] = 0xAA;  // Dummy byte 1
        data[1] = 0x55;  // Dummy byte 2
        data[2] = (uint8_t)(currentTime & 0xFF);         // Counter low byte
        data[3] = (uint8_t)((currentTime >> 8) & 0xFF);  // Counter high byte

        bool success = can_bus.sendStandardFrame(CAN_ID_IO_DATA, data, 4);

        if (success) {
            // Only log TX on long intervals (>2 seconds) or when Debug level is enabled
            if ((currentTime - lastCANTx) > 2000) {
                myLogger->Write(LogLevel::Debug, LOG_CAT_DEVICE, "sendDummyData",
                              "CAN TX [Dummy] - Interval: %lums, Counter: 0x%04X",
                              currentTime - lastCANTx, currentTime & 0xFFFF);
            }
        } else {
            // Only log TX failures every 5 seconds to avoid spam
            if ((currentTime - lastTxErrorLog) >= 5000) {
                myLogger->Write(LogLevel::Warn, LOG_CAT_DEVICE, "sendDummyData",
                              "CAN TX failed for dummy data");
                lastTxErrorLog = currentTime;
            }
        }

        lastCANTx = currentTime;
        return success;
    }

    return true; // Not time to send yet, but not an error
}

// Process incoming CAN messages
void processCANMessages() {
    can_bus.checkMessages();
}

// Check if CAN is ready
bool isCANReady() {
    return can_bus.isInitialized();
}


// Handle dummy control messages (0x124)
void handleDummyControl(const can_message_t* message) {
    if (message->length >= 2) {
        // Simple dummy RX handler - just log the received data
        myLogger->Write(LogLevel::Info, LOG_CAT_DEVICE, "handleIOOutputControl",
                       "RX Control: Byte0=0x%02X, Byte1=0x%02X",
                       message->data[0], message->data[1]);
    }
}

// Main CAN message dispatcher
void onCANMessageReceived(const can_message_t* message) {
    // Create hex data string for logging
    char hexData[32] = "";
    for (int i = 0; i < message->length; i++) {
        char temp[4];
        sprintf(temp, "%s%02X", (i > 0) ? " " : "", message->data[i]);
        strcat(hexData, temp);
    }

    // Log raw message with optimized format
    myLogger->Write(LogLevel::Debug, LOG_CAT_CAN, "onCANMessageReceived",
                   "RX ID: 0x%03X [%s]", message->id, hexData);

    // Route to specific handler based on CAN ID
    switch (message->id) {
        case CAN_ID_IO_OUTPUT:
            handleDummyControl(message);
            break;
        default:
            myLogger->Write(LogLevel::Warn, LOG_CAT_CAN, "onCANMessageReceived",
                           "Unhandled CAN ID: 0x%03X [%s]", message->id, hexData);
            break;
    }
}

// Print CAN status information
void printCANStatus() {
    if (can_bus.isInitialized()) {
        myLogger->Write(LogLevel::Info, LOG_CAT_CAN, "printCANStatus", "CAN Bus Status:");
        can_bus.printStatus();
    } else {
        myLogger->Write(LogLevel::Warn, LOG_CAT_CAN, "printCANStatus", "CAN Bus not initialized");
    }
}

// Get CAN error count
uint32_t getCANErrorCount() {
    return can_bus.getErrorCount();
}

#endif // ENABLE_CAN