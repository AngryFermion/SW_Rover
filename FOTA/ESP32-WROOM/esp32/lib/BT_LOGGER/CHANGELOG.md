# Changelog

All notable changes to the BT_LOGGER library will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.0.0] - 2025-10-02

### Added
- Initial release of BT_LOGGER (Bluetooth Logger)
- Six configurable log levels: Trace, Debug, Info, Warn, Error, Fatal
- 13 log categories: SYSTEM, SETUP, WIFI, MODBUS, SPIFFS, FS, SERVER, LIVE, MQTT, CAN, IO, OTHERS, DEVICE, BLE
- Timestamp support for all log messages
- Dynamic log level changes via BLE commands
- Printf-style formatted logging
- Category-based filtering
- Console enable/disable functionality
- Version management system
- GetVersion() and PrintVersion() methods

### Features
- Lightweight and optimized for ESP32
- Minimal memory footprint
- Thread-safe logging
- Color-coded log levels (when used with compatible terminals)
- Integration with BLE for remote log level control

### Compatibility
- ESP32 platform
- Arduino framework
- PlatformIO support
