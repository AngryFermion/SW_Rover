/**
 * can_handler.h - CAN Bus Message Handler for I/O Expander Project
 *
 * This module handles all CAN bus communication for the PCF8575 I/O expander project.
 * Manages message reception, transmission, and processing for specific CAN IDs.
 *
 * Supported CAN Message IDs:
 * - 0x123: Dummy test data transmission (TX)
 * - 0x124: Dummy control message (RX)
 *
 * Author: Narayana Swamy
 * Date: September 2025
 */

#ifndef CAN_HANDLER_H
#define CAN_HANDLER_H

#include <Arduino.h>
#include "config.h"

#if ENABLE_CAN
#include "SMART_CAN.h"
#include "SMART_LOGGER.h"
#include "log_categories.h"

// CAN configuration constants
#define CAN_TX_PIN 14        // GPIO14 for CAN transmit
#define CAN_RX_PIN 13        // GPIO13 for CAN receive
#define CAN_SPEED CAN_SPEED_500KBPS
#define CAN_TX_INTERVAL 100  // Transmission interval in milliseconds

// CAN Message IDs
#define CAN_ID_IO_DATA      0x123  // Dummy test data (outgoing)
#define CAN_ID_IO_OUTPUT    0x124  // Dummy control message (incoming)

// Function declarations
bool initializeCAN();
void shutdownCAN();
bool sendDummyData();
void processCANMessages();
bool isCANReady();

// External logger instance (defined in main.cpp)
extern SMART_LOGGER* myLogger;

// Message handler functions
void handleDummyControl(const can_message_t* message);

// Main CAN message callback
void onCANMessageReceived(const can_message_t* message);

// Utility functions
void printCANStatus();
uint32_t getCANErrorCount();

#endif // ENABLE_CAN

#endif // CAN_HANDLER_H