# BT_LOGGER - Bluetooth Logger Library

A lightweight, feature-rich logging system for ESP32 projects with configurable log levels and categories.

## Features

- ✅ **6 Log Levels**: Trace, Debug, Info, Warn, Error, Fatal
- ✅ **13 Categories**: BLE, WiFi, System, Setup, MODBUS, SPIFFS, FS, Server, MQTT, CAN, IO, Device, Others
- ✅ **Timestamp Support**: All logs include HH:MM:SS timestamps
- ✅ **Dynamic Control**: Change log levels via BLE commands
- ✅ **Printf-style Formatting**: Supports formatted strings
- ✅ **Category Filtering**: Enable/disable specific log categories
- ✅ **Minimal Footprint**: Optimized for embedded systems
- ✅ **Version Management**: Built-in version tracking

## Installation

### PlatformIO
Copy the `BT_LOGGER` folder to your project's `lib/` directory.

### Arduino IDE
Copy the `BT_LOGGER` folder to your Arduino libraries folder.

## Quick Start

```cpp
#include <BT_LOGGER.h>

// Create logger instance with Info level
BT_LOGGER DebugLog(LogLevel::Info);

void setup() {
  Serial.begin(115200);

  // Print library version
  BT_LOGGER::PrintVersion();

  // Basic logging
  DebugLog.Write(LogLevel::Info, LogCategory::SETUP, "setup", "System initialized");

  // Formatted logging
  int temperature = 25;
  DebugLog.Write(LogLevel::Debug, LogCategory::SYSTEM, "loop", "Temperature: %d°C", temperature);
}
```

## Log Levels

| Level | Value | Description |
|-------|-------|-------------|
| Trace | 0 | Most verbose - all messages |
| Debug | 1 | Debug information |
| Info | 2 | General information (default) |
| Warn | 3 | Warnings |
| Error | 4 | Errors |
| Fatal | 5 | Critical errors only |

## Log Categories

- `LogCategory::BLE` - Bluetooth Low Energy
- `LogCategory::WIFI` - WiFi operations
- `LogCategory::SYSTEM` - System-level logs
- `LogCategory::SETUP` - Setup/initialization
- `LogCategory::MODBUS` - MODBUS communication
- `LogCategory::SPIFFS` - SPIFFS filesystem
- `LogCategory::FS` - General filesystem
- `LogCategory::SERVER` - Server operations
- `LogCategory::MQTT` - MQTT messaging
- `LogCategory::CAN` - CAN bus
- `LogCategory::IO` - Input/Output operations
- `LogCategory::DEVICE` - Device-specific
- `LogCategory::OTHERS` - Miscellaneous

## API Reference

### Constructor
```cpp
BT_LOGGER();                    // Default: Trace level
BT_LOGGER(LogLevel level);      // Specific level
```

### Logging Methods
```cpp
// String message
void Write(LogLevel level, LogCategory category, String method, String message);

// Formatted message (printf-style)
void Write(LogLevel level, LogCategory category, String method, const char* format, ...);
```

### Configuration
```cpp
void EnableConsole();           // Enable console output
void DisableConsole();          // Disable console output
void SetLogLevel(LogLevel level); // Change log level dynamically
```

### Status
```cpp
bool IsConsoleEnabled();        // Check if console is enabled
LogLevel GetLogLevel();         // Get current log level
```

### Version
```cpp
static const char* GetVersion();  // Get version string
static void PrintVersion();       // Print version to Serial
```

## Dynamic Log Level Control (BLE)

The logger supports dynamic log level changes via BLE commands:

```json
{"log_level": "debug"}
```

Supported values: `trace`, `debug`, `info`, `warn`, `error`, `fatal`

## Log Output Format

```
HH:MM:SS|LEVEL|CATEGORY|METHOD|MESSAGE
```

Example:
```
14:32:15|INFO|BLE|setup|Starting BLE...
14:32:16|DEBUG|IO|onWrite|Red LED: 128
14:32:17|ERROR|BLE|onWrite|JSON parse error: Invalid input
```

## Version History

See [CHANGELOG.md](CHANGELOG.md) for detailed version history.

## Current Version

**v1.0.0** - Initial release

## License

Copyright © 2025 Narayana Swamy

## Author

Narayana Swamy

## Contributing

Contributions are welcome! Please submit pull requests or open issues on the project repository.

## Support

For questions or issues, please open an issue on the project repository.
