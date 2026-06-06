# ESP32_CAN Library

A comprehensive and easy-to-use CAN bus library for ESP32 microcontrollers using the built-in TWAI (Two-Wire Automotive Interface) driver.

## Features

- ✅ **Multiple CAN Speeds**: 125k, 250k, 500k, 1M bps
- ✅ **Flexible GPIO Configuration**: Configurable TX/RX pins
- ✅ **Advanced Message Filtering**: Single ID, multiple ID, range, and custom mask filtering
- ✅ **Non-blocking Operations**: Asynchronous message reception with callbacks
- ✅ **Error Handling**: Comprehensive error reporting and diagnostics
- ✅ **ESP32 Compatible**: Works with all ESP32 variants (ESP32, ESP32-S2, ESP32-S3, ESP32-C3)
- ✅ **Standard & Extended Frames**: Support for both 11-bit and 29-bit CAN IDs
- ✅ **Memory Efficient**: Optimized for embedded applications

## Quick Start

### Installation

1. Copy the `ESP32_CAN` folder to your project's `lib` directory
2. Include the library in your code:

```cpp
#include "ESP32_CAN.h"
```

### Basic Usage

```cpp
#include "ESP32_CAN.h"

// Create CAN instance (TX pin, RX pin)
ESP32_CAN can_bus(5, 4);

void onMessageReceived(const can_message_t* message) {
    Serial.print("Received ID: 0x");
    Serial.println(message->id, HEX);
}

void setup() {
    Serial.begin(115200);

    // Initialize CAN at 500 kbps
    if (!can_bus.begin(CAN_SPEED_500KBPS)) {
        Serial.println("CAN initialization failed!");
        return;
    }

    // Set callback for received messages
    can_bus.setReceiveCallback(onMessageReceived);

    Serial.println("CAN initialized successfully!");
}

void loop() {
    // Check for incoming messages
    can_bus.checkMessages();

    // Send a message every second
    static unsigned long lastTx = 0;
    if (millis() - lastTx > 1000) {
        uint8_t data[] = {0x01, 0x02, 0x03, 0x04};
        can_bus.sendStandardFrame(0x123, data, 4);
        lastTx = millis();
    }
}
```

## API Reference

### Constructor

```cpp
ESP32_CAN(int tx_pin = 5, int rx_pin = 4)
```

Creates a new ESP32_CAN instance with specified GPIO pins.

**Parameters:**
- `tx_pin`: GPIO pin for CAN TX (default: 5)
- `rx_pin`: GPIO pin for CAN RX (default: 4)

### Initialization

#### `begin(can_speed_t speed)`

```cpp
bool begin(can_speed_t speed = CAN_SPEED_500KBPS)
```

Initializes the CAN bus with specified speed.

**Parameters:**
- `speed`: CAN bus speed (CAN_SPEED_125KBPS, CAN_SPEED_250KBPS, CAN_SPEED_500KBPS, CAN_SPEED_1MBPS)

**Returns:** `true` if successful, `false` otherwise

#### `end()`

```cpp
bool end()
```

Stops the CAN bus and releases resources.

**Returns:** `true` if successful, `false` otherwise

### Message Transmission

#### `sendMessage()`

```cpp
bool sendMessage(uint32_t id, const uint8_t* data, uint8_t length, bool extended = false)
bool sendMessage(const can_message_t* message)
```

Sends a CAN message with specified parameters.

**Parameters:**
- `id`: CAN message ID
- `data`: Pointer to data bytes (max 8 bytes)
- `length`: Number of data bytes (0-8)
- `extended`: Use extended frame format (29-bit ID)

**Returns:** `true` if successful, `false` otherwise

#### `sendStandardFrame()` / `sendExtendedFrame()`

```cpp
bool sendStandardFrame(uint32_t id, const uint8_t* data, uint8_t length)
bool sendExtendedFrame(uint32_t id, const uint8_t* data, uint8_t length)
```

Convenience methods for sending standard (11-bit) or extended (29-bit) frames.

### Message Reception

#### `receiveMessage()`

```cpp
bool receiveMessage(can_message_t* message, uint32_t timeout_ms = 0)
```

Receives a CAN message (blocking with timeout).

**Parameters:**
- `message`: Pointer to message structure to fill
- `timeout_ms`: Timeout in milliseconds (0 = non-blocking)

**Returns:** `true` if message received, `false` otherwise

#### `setReceiveCallback()`

```cpp
void setReceiveCallback(can_receive_callback_t callback)
```

Sets callback function for asynchronous message reception.

**Parameters:**
- `callback`: Function pointer to callback (`void callback(const can_message_t* message)`)

#### `checkMessages()`

```cpp
void checkMessages()
```

Processes incoming messages and calls registered callback. Call this in your main loop.

### Message Filtering

#### Single ID Filter

```cpp
bool setStandardFilter(uint32_t filter_id, uint32_t filter_mask = 0x7FF)
bool setExtendedFilter(uint32_t filter_id, uint32_t filter_mask = 0x1FFFFFFF)
```

Accept only messages with specific ID.

**Example:**
```cpp
// Accept only messages with ID 0x123
can_bus.setStandardFilter(0x123);
```

#### Multiple ID Filter

```cpp
bool setMultipleFilter(const uint32_t* ids, uint8_t count, bool extended = false)
```

Accept messages from a list of specific IDs.

**Example:**
```cpp
uint32_t sensor_ids[] = {0x201, 0x202, 0x203};
can_bus.setMultipleFilter(sensor_ids, 3);
```

#### Range Filter

```cpp
bool setRangeFilter(uint32_t id_start, uint32_t id_end, bool extended = false)
```

Accept messages with IDs within a specified range.

**Example:**
```cpp
// Accept IDs from 0x100 to 0x1FF
can_bus.setRangeFilter(0x100, 0x1FF);
```

#### Custom Filter

```cpp
bool setFilter(uint32_t filter_id, uint32_t filter_mask, bool extended = false)
```

Advanced filtering with custom mask.

**Example:**
```cpp
// Accept 0x120-0x12F (mask out last 4 bits)
can_bus.setFilter(0x120, 0x7F0);
```

#### Clear Filter

```cpp
bool clearFilter()
```

Remove all filters and accept all messages.

### Configuration

#### `setPins()`

```cpp
bool setPins(int tx_pin, int rx_pin)
```

Change GPIO pins (only when CAN is stopped).

#### `setSpeed()`

```cpp
bool setSpeed(can_speed_t speed)
```

Change CAN speed (only when CAN is stopped).

### Status and Diagnostics

#### `getStatus()` / `printStatus()`

```cpp
bool getStatus()
void printStatus()
```

Get or print detailed CAN bus status information.

#### `getErrorCount()`

```cpp
uint32_t getErrorCount()
```

Returns total error count (TX + RX errors).

#### `isInitialized()`

```cpp
bool isInitialized() const
```

Check if CAN bus is initialized and running.

## Message Structure

```cpp
typedef struct {
    uint32_t id;           // CAN ID
    uint8_t data[8];      // Data bytes (max 8)
    uint8_t length;       // Data length (0-8)
    bool extended;        // Extended frame format
    bool rtr;            // Remote transmission request
} can_message_t;
```

## CAN Speed Constants

```cpp
typedef enum {
    CAN_SPEED_125KBPS = 0,
    CAN_SPEED_250KBPS,
    CAN_SPEED_500KBPS,
    CAN_SPEED_1MBPS
} can_speed_t;
```

## Usage Examples

### Example 1: Basic Communication

```cpp
#include "ESP32_CAN.h"

ESP32_CAN can_bus(5, 4);

void setup() {
    Serial.begin(115200);
    can_bus.begin(CAN_SPEED_500KBPS);
}

void loop() {
    // Send periodic message
    static unsigned long lastTx = 0;
    if (millis() - lastTx > 1000) {
        uint8_t data[] = {0xAA, 0xBB, 0xCC, 0xDD};
        can_bus.sendStandardFrame(0x123, data, 4);
        lastTx = millis();
    }

    // Check for incoming messages
    can_message_t message;
    if (can_bus.receiveMessage(&message, 0)) {
        Serial.print("Received: ID=0x");
        Serial.print(message.id, HEX);
        Serial.print(", Data=");
        for (int i = 0; i < message.length; i++) {
            Serial.print(message.data[i], HEX);
            Serial.print(" ");
        }
        Serial.println();
    }
}
```

### Example 2: Multiple Sensors with Filtering

```cpp
#include "ESP32_CAN.h"

ESP32_CAN can_bus(5, 4);

void onSensorData(const can_message_t* message) {
    switch (message->id) {
        case 0x201:
            Serial.println("Temperature sensor data");
            break;
        case 0x202:
            Serial.println("Pressure sensor data");
            break;
        case 0x203:
            Serial.println("Humidity sensor data");
            break;
    }
}

void setup() {
    Serial.begin(115200);
    can_bus.begin(CAN_SPEED_500KBPS);

    // Filter for sensor messages only
    uint32_t sensor_ids[] = {0x201, 0x202, 0x203};
    can_bus.setMultipleFilter(sensor_ids, 3);

    can_bus.setReceiveCallback(onSensorData);
}

void loop() {
    can_bus.checkMessages();
    delay(10);
}
```

### Example 3: Range-based Filtering

```cpp
#include "ESP32_CAN.h"

ESP32_CAN can_bus(5, 4);

void onControlMessage(const can_message_t* message) {
    Serial.print("Control message ID: 0x");
    Serial.println(message->id, HEX);

    // Process control commands
    if (message->length >= 2) {
        uint8_t command = message->data[0];
        uint8_t value = message->data[1];
        Serial.print("Command: 0x");
        Serial.print(command, HEX);
        Serial.print(", Value: ");
        Serial.println(value);
    }
}

void setup() {
    Serial.begin(115200);
    can_bus.begin(CAN_SPEED_500KBPS);

    // Accept control messages in range 0x300-0x3FF
    can_bus.setRangeFilter(0x300, 0x3FF);

    can_bus.setReceiveCallback(onControlMessage);
}

void loop() {
    can_bus.checkMessages();

    // Send status every 5 seconds
    static unsigned long lastStatus = 0;
    if (millis() - lastStatus > 5000) {
        uint8_t status[] = {0x01, 0x00, 0x64}; // Running, no errors, 100% battery
        can_bus.sendStandardFrame(0x400, status, 3);
        lastStatus = millis();
    }
}
```

## Hardware Requirements

### CAN Transceiver

The ESP32 has a built-in CAN controller but requires an external CAN transceiver IC. Common options:

- **SN65HVD230**: 3.3V CAN transceiver (recommended)
- **MCP2551**: 5V CAN transceiver
- **TJA1050**: High-speed CAN transceiver

### Wiring Example (SN65HVD230)

```
ESP32     SN65HVD230
-----     ----------
GPIO5  -> CTX (TX)
GPIO4  -> CRX (RX)
3.3V   -> VCC
GND    -> GND
       -> CANH (to CAN bus)
       -> CANL (to CAN bus)
```

### CAN Bus Termination

- CAN bus requires 120Ω termination resistors at both ends
- Use twisted pair cable for reliable communication
- Maximum bus length depends on speed:
  - 1 Mbps: 25 meters
  - 500 kbps: 100 meters
  - 250 kbps: 250 meters
  - 125 kbps: 500 meters

## Troubleshooting

### Common Issues

**1. CAN initialization fails**
- Check GPIO pin connections
- Verify CAN transceiver power supply
- Ensure CAN bus has proper termination

**2. No messages received**
- Check CAN bus wiring (CANH/CANL)
- Verify other devices are transmitting
- Check message filters are configured correctly

**3. High error count**
- Check CAN bus termination (120Ω at each end)
- Verify cable quality and length
- Check for incorrect baud rate

**4. Messages not sending**
- Verify CAN bus has other active nodes
- Check if bus is in error state (`printStatus()`)
- Ensure proper acknowledgment from other nodes

### Debug Commands

```cpp
// Print detailed status
can_bus.printStatus();

// Check error count
uint32_t errors = can_bus.getErrorCount();
Serial.print("Error count: ");
Serial.println(errors);

// Verify initialization
if (!can_bus.isInitialized()) {
    Serial.println("CAN not initialized!");
}
```

## License

This library is released under the MIT License. See LICENSE file for details.

## Contributing

Contributions are welcome! Please feel free to submit pull requests or open issues for bugs and feature requests.

## Author

ESP32 Creators - September 2025