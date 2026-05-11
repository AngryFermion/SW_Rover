# ESP32_CAN Library API Reference

Complete API documentation for the ESP32_CAN library.

## Table of Contents

- [Class Overview](#class-overview)
- [Data Types](#data-types)
- [Constructor](#constructor)
- [Initialization Methods](#initialization-methods)
- [Message Transmission](#message-transmission)
- [Message Reception](#message-reception)
- [Message Filtering](#message-filtering)
- [Configuration Methods](#configuration-methods)
- [Status and Diagnostics](#status-and-diagnostics)
- [Utility Methods](#utility-methods)
- [Error Codes](#error-codes)

## Class Overview

```cpp
class ESP32_CAN
```

Main class for ESP32 CAN bus communication using the built-in TWAI driver.

## Data Types

### `can_speed_t`

Enumeration for CAN bus speeds:

```cpp
typedef enum {
    CAN_SPEED_125KBPS = 0,
    CAN_SPEED_250KBPS,
    CAN_SPEED_500KBPS,
    CAN_SPEED_1MBPS
} can_speed_t;
```

### `can_message_t`

Structure for CAN messages:

```cpp
typedef struct {
    uint32_t id;                    // CAN ID (11-bit or 29-bit)
    uint8_t data[8];               // Data bytes (maximum 8)
    uint8_t length;                // Data length (0-8)
    bool extended;                 // Extended frame format (29-bit ID)
    bool rtr;                      // Remote transmission request
} can_message_t;
```

### `can_receive_callback_t`

Function pointer type for message callbacks:

```cpp
typedef void (*can_receive_callback_t)(const can_message_t* message);
```

## Constructor

### `ESP32_CAN(int tx_pin, int rx_pin)`

**Description:** Creates a new ESP32_CAN instance.

**Parameters:**
- `tx_pin` (int): GPIO pin for CAN TX (default: 5)
- `rx_pin` (int): GPIO pin for CAN RX (default: 4)

**Example:**
```cpp
ESP32_CAN can_bus(5, 4);  // TX on GPIO5, RX on GPIO4
ESP32_CAN can_bus;        // Use default pins (5, 4)
```

## Initialization Methods

### `begin(can_speed_t speed)`

**Description:** Initializes the CAN bus with specified speed.

**Parameters:**
- `speed` (can_speed_t): CAN bus speed

**Returns:** `bool` - `true` if successful, `false` otherwise

**Example:**
```cpp
if (!can_bus.begin(CAN_SPEED_500KBPS)) {
    Serial.println("CAN initialization failed!");
}
```

### `end()`

**Description:** Stops the CAN bus and releases resources.

**Returns:** `bool` - `true` if successful, `false` otherwise

**Example:**
```cpp
can_bus.end();
```

### `isInitialized()`

**Description:** Checks if CAN bus is initialized and running.

**Returns:** `bool` - `true` if initialized, `false` otherwise

**Example:**
```cpp
if (can_bus.isInitialized()) {
    Serial.println("CAN is running");
}
```

## Message Transmission

### `sendMessage(uint32_t id, const uint8_t* data, uint8_t length, bool extended)`

**Description:** Sends a CAN message with specified parameters.

**Parameters:**
- `id` (uint32_t): CAN message ID
- `data` (const uint8_t*): Pointer to data bytes
- `length` (uint8_t): Number of data bytes (0-8)
- `extended` (bool): Use extended frame format (default: false)

**Returns:** `bool` - `true` if successful, `false` otherwise

**Example:**
```cpp
uint8_t data[] = {0x01, 0x02, 0x03, 0x04};
can_bus.sendMessage(0x123, data, 4, false);
```

### `sendMessage(const can_message_t* message)`

**Description:** Sends a CAN message using message structure.

**Parameters:**
- `message` (const can_message_t*): Pointer to message structure

**Returns:** `bool` - `true` if successful, `false` otherwise

**Example:**
```cpp
can_message_t msg;
msg.id = 0x123;
msg.length = 2;
msg.data[0] = 0xAA;
msg.data[1] = 0xBB;
msg.extended = false;
can_bus.sendMessage(&msg);
```

### `sendStandardFrame(uint32_t id, const uint8_t* data, uint8_t length)`

**Description:** Convenience method for sending standard frames (11-bit ID).

**Parameters:**
- `id` (uint32_t): CAN message ID (11-bit)
- `data` (const uint8_t*): Pointer to data bytes
- `length` (uint8_t): Number of data bytes (0-8)

**Returns:** `bool` - `true` if successful, `false` otherwise

**Example:**
```cpp
uint8_t data[] = {0x01, 0x02};
can_bus.sendStandardFrame(0x123, data, 2);
```

### `sendExtendedFrame(uint32_t id, const uint8_t* data, uint8_t length)`

**Description:** Convenience method for sending extended frames (29-bit ID).

**Parameters:**
- `id` (uint32_t): CAN message ID (29-bit)
- `data` (const uint8_t*): Pointer to data bytes
- `length` (uint8_t): Number of data bytes (0-8)

**Returns:** `bool` - `true` if successful, `false` otherwise

**Example:**
```cpp
uint8_t data[] = {0x01, 0x02, 0x03};
can_bus.sendExtendedFrame(0x12345678, data, 3);
```

## Message Reception

### `receiveMessage(can_message_t* message, uint32_t timeout_ms)`

**Description:** Receives a CAN message (blocking with timeout).

**Parameters:**
- `message` (can_message_t*): Pointer to message structure to fill
- `timeout_ms` (uint32_t): Timeout in milliseconds (0 = non-blocking)

**Returns:** `bool` - `true` if message received, `false` otherwise

**Example:**
```cpp
can_message_t message;
if (can_bus.receiveMessage(&message, 1000)) {
    Serial.print("Received ID: 0x");
    Serial.println(message.id, HEX);
}
```

### `hasMessage()`

**Description:** Checks if messages are available in receive queue.

**Returns:** `bool` - `true` if messages available, `false` otherwise

**Example:**
```cpp
if (can_bus.hasMessage()) {
    can_message_t message;
    can_bus.receiveMessage(&message, 0);
}
```

### `setReceiveCallback(can_receive_callback_t callback)`

**Description:** Sets callback function for asynchronous message reception.

**Parameters:**
- `callback` (can_receive_callback_t): Function pointer to callback

**Example:**
```cpp
void onMessage(const can_message_t* msg) {
    Serial.print("ID: 0x");
    Serial.println(msg->id, HEX);
}

can_bus.setReceiveCallback(onMessage);
```

### `checkMessages()`

**Description:** Processes incoming messages and calls registered callback.

**Note:** Call this in your main loop for callback-based reception.

**Example:**
```cpp
void loop() {
    can_bus.checkMessages();
    // Other code...
}
```

## Message Filtering

### `setFilter(uint32_t filter_id, uint32_t filter_mask, bool extended)`

**Description:** Sets custom message filter with mask.

**Parameters:**
- `filter_id` (uint32_t): Base filter ID
- `filter_mask` (uint32_t): Filter mask (1 = must match, 0 = don't care)
- `extended` (bool): Extended frame format (default: false)

**Returns:** `bool` - `true` if successful, `false` otherwise

**Example:**
```cpp
// Accept IDs 0x120-0x12F (mask out last 4 bits)
can_bus.setFilter(0x120, 0x7F0);
```

### `setStandardFilter(uint32_t filter_id, uint32_t filter_mask)`

**Description:** Sets filter for standard frames (11-bit ID).

**Parameters:**
- `filter_id` (uint32_t): Filter ID
- `filter_mask` (uint32_t): Filter mask (default: 0x7FF = exact match)

**Returns:** `bool` - `true` if successful, `false` otherwise

**Example:**
```cpp
// Accept only messages with ID 0x123
can_bus.setStandardFilter(0x123);

// Accept IDs 0x100-0x10F
can_bus.setStandardFilter(0x100, 0x7F0);
```

### `setExtendedFilter(uint32_t filter_id, uint32_t filter_mask)`

**Description:** Sets filter for extended frames (29-bit ID).

**Parameters:**
- `filter_id` (uint32_t): Filter ID
- `filter_mask` (uint32_t): Filter mask (default: 0x1FFFFFFF = exact match)

**Returns:** `bool` - `true` if successful, `false` otherwise

**Example:**
```cpp
// Accept only messages with extended ID 0x12345678
can_bus.setExtendedFilter(0x12345678);
```

### `setRangeFilter(uint32_t id_start, uint32_t id_end, bool extended)`

**Description:** Sets filter to accept IDs within a range.

**Parameters:**
- `id_start` (uint32_t): Start of ID range (inclusive)
- `id_end` (uint32_t): End of ID range (inclusive)
- `extended` (bool): Extended frame format (default: false)

**Returns:** `bool` - `true` if successful, `false` otherwise

**Example:**
```cpp
// Accept IDs from 0x100 to 0x1FF
can_bus.setRangeFilter(0x100, 0x1FF);
```

### `setMultipleFilter(const uint32_t* ids, uint8_t count, bool extended)`

**Description:** Sets filter to accept specific list of IDs.

**Parameters:**
- `ids` (const uint32_t*): Array of CAN IDs
- `count` (uint8_t): Number of IDs in array
- `extended` (bool): Extended frame format (default: false)

**Returns:** `bool` - `true` if successful, `false` otherwise

**Note:** Due to hardware limitations, may accept additional IDs that match the calculated mask pattern.

**Example:**
```cpp
uint32_t sensor_ids[] = {0x201, 0x202, 0x203, 0x204};
can_bus.setMultipleFilter(sensor_ids, 4);
```

### `clearFilter()`

**Description:** Removes all filters and accepts all messages.

**Returns:** `bool` - `true` if successful, `false` otherwise

**Example:**
```cpp
can_bus.clearFilter();  // Accept all messages
```

### `isFilterEnabled()`

**Description:** Checks if message filtering is enabled.

**Returns:** `bool` - `true` if filter enabled, `false` otherwise

**Example:**
```cpp
if (can_bus.isFilterEnabled()) {
    Serial.println("Message filtering is active");
}
```

## Configuration Methods

### `setPins(int tx_pin, int rx_pin)`

**Description:** Changes GPIO pins for CAN interface.

**Parameters:**
- `tx_pin` (int): GPIO pin for CAN TX
- `rx_pin` (int): GPIO pin for CAN RX

**Returns:** `bool` - `true` if successful, `false` otherwise

**Note:** Can only be called when CAN is stopped.

**Example:**
```cpp
can_bus.end();
can_bus.setPins(32, 34);
can_bus.begin(CAN_SPEED_500KBPS);
```

### `setSpeed(can_speed_t speed)`

**Description:** Changes CAN bus speed.

**Parameters:**
- `speed` (can_speed_t): New CAN bus speed

**Returns:** `bool` - `true` if successful, `false` otherwise

**Note:** Can only be called when CAN is stopped.

**Example:**
```cpp
can_bus.end();
can_bus.setSpeed(CAN_SPEED_1MBPS);
can_bus.begin();
```

## Status and Diagnostics

### `getStatus()`

**Description:** Gets current CAN bus status.

**Returns:** `bool` - `true` if bus is running normally, `false` otherwise

**Example:**
```cpp
if (!can_bus.getStatus()) {
    Serial.println("CAN bus error detected");
}
```

### `printStatus()`

**Description:** Prints detailed CAN bus status to Serial.

**Example:**
```cpp
can_bus.printStatus();
```

**Sample Output:**
```
=== CAN Status ===
State: RUNNING
Messages to TX: 0
Messages to RX: 2
TX Error Count: 0
RX Error Count: 0
TX Failed Count: 0
RX Missed Count: 0
RX Overrun Count: 0
Arbitration Lost Count: 0
Bus Error Count: 0
==================
```

### `getErrorCount()`

**Description:** Gets total error count (TX + RX errors).

**Returns:** `uint32_t` - Total error count

**Example:**
```cpp
uint32_t errors = can_bus.getErrorCount();
if (errors > 0) {
    Serial.print("Total errors: ");
    Serial.println(errors);
}
```

## Utility Methods

### `speedToString(can_speed_t speed)` (Static)

**Description:** Converts CAN speed enum to string.

**Parameters:**
- `speed` (can_speed_t): CAN speed enum

**Returns:** `const char*` - Speed string

**Example:**
```cpp
Serial.print("CAN Speed: ");
Serial.println(ESP32_CAN::speedToString(CAN_SPEED_500KBPS));
// Output: "CAN Speed: 500 kbps"
```

### `errorToString(esp_err_t error)` (Static)

**Description:** Converts ESP error code to string.

**Parameters:**
- `error` (esp_err_t): ESP error code

**Returns:** `const char*` - Error string

**Example:**
```cpp
esp_err_t result = /* some ESP function */;
if (result != ESP_OK) {
    Serial.print("Error: ");
    Serial.println(ESP32_CAN::errorToString(result));
}
```

## Error Codes

Common ESP error codes returned by the library:

| Error Code | Description |
|------------|-------------|
| `ESP_OK` | Success |
| `ESP_ERR_INVALID_ARG` | Invalid argument |
| `ESP_ERR_INVALID_STATE` | Invalid state |
| `ESP_ERR_TIMEOUT` | Operation timeout |
| `ESP_ERR_NO_MEM` | Out of memory |
| `ESP_FAIL` | Generic failure |

## Usage Patterns

### Pattern 1: Polling for Messages

```cpp
void loop() {
    can_message_t message;
    if (can_bus.receiveMessage(&message, 0)) {
        // Process message
        processMessage(&message);
    }

    // Other loop code...
}
```

### Pattern 2: Callback-based Reception

```cpp
void onMessage(const can_message_t* msg) {
    // Process message in callback
    processMessage(msg);
}

void setup() {
    can_bus.begin(CAN_SPEED_500KBPS);
    can_bus.setReceiveCallback(onMessage);
}

void loop() {
    can_bus.checkMessages();  // Triggers callbacks
    // Other loop code...
}
```

### Pattern 3: Periodic Transmission

```cpp
void loop() {
    static unsigned long lastTx = 0;

    if (millis() - lastTx >= 1000) {  // Every second
        uint8_t data[] = {0x01, 0x02};
        can_bus.sendStandardFrame(0x123, data, 2);
        lastTx = millis();
    }

    can_bus.checkMessages();
}
```

### Pattern 4: Error Handling

```cpp
void setup() {
    if (!can_bus.begin(CAN_SPEED_500KBPS)) {
        Serial.println("CAN init failed - check wiring");
        while (1) delay(1000);
    }
}

void loop() {
    can_bus.checkMessages();

    // Check for errors periodically
    static unsigned long lastErrorCheck = 0;
    if (millis() - lastErrorCheck >= 5000) {
        uint32_t errors = can_bus.getErrorCount();
        if (errors > 100) {
            Serial.println("High error count - check bus");
            can_bus.printStatus();
        }
        lastErrorCheck = millis();
    }
}
```

## Performance Considerations

- **Message Rate**: ESP32 can handle thousands of messages per second
- **Filtering**: Use hardware filters when possible for better performance
- **Callbacks**: Keep callback functions short and fast
- **Polling**: Use non-blocking `receiveMessage()` calls in main loop
- **Memory**: Each message uses ~16 bytes of RAM
- **CPU**: TWAI driver uses minimal CPU when properly configured

## Thread Safety

- The ESP32_CAN library is **not thread-safe**
- Use from single task/thread only
- For multi-task applications, implement appropriate synchronization
- FreeRTOS mutexes can be used if needed

---

*For more examples and detailed usage, see the examples folder and README.md*