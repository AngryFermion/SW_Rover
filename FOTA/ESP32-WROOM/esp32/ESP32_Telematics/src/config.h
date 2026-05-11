#ifndef CONFIG_H
#define CONFIG_H

// Feature Enable/Disable
#define ENABLE_MQTT 1
#define ENABLE_FOTA 1
#define ENABLE_CAN 0
#define USE_AWS_IOT 1  // Set to 1 for AWS IoT Core, 0 for public MQTT broker

#define BOARD_TYPE 1
// Tri-color LED pins
#if BOARD_TYPE == 0
#define RED_LED 23
#define GREEN_LED 22
#define BLUE_LED 21
#define FOTA_SERIAL_RX_PIN 18                 
#define FOTA_SERIAL_TX_PIN 19                
#else 
#define RED_LED 26
#define GREEN_LED 25
#define BLUE_LED 2
#define FOTA_SERIAL_RX_PIN 16
#define FOTA_SERIAL_TX_PIN 17
#endif

// Serial Configuration
#define DEBUG_SERIAL_BAUD_RATE 115200
#define DEBUG_SERIAL_RX_PIN 3
#define DEBUG_SERIAL_TX_PIN 1

#define FOTA_SERIAL_BAUD_RATE 115200

// Active low LED defines
#define LED_ON HIGH
#define LED_OFF LOW

// WiFi credentials
extern const char* ssid;
extern const char* password;

// MQTT Configuration
extern const char* mqtt_server;
extern const int mqtt_port;
extern const char* mqtt_username;
extern const char* mqtt_password;
extern const char* mqtt_client_id;

// MQTT Topics
extern const char* topic_fota;
extern const char* topic_data;
extern const char* topic_smartwheels;

// FOTA Configuration
#define FOTA_BUFFER_SIZE 73728   // 72KB static buffer
#define CHUNK_JSON_SIZE (1024*4) + 512  // 4KB + 512 bytes for JSON parsing
#define METADATA_JSON_SIZE 4096  // 4KB for metadata JSON


// Timing Configuration
#define LED_UPDATE_FAST 100      // 100ms for fast blinking
#define LED_UPDATE_SLOW 1000     // 1000ms for slow blinking
#define STATUS_UPDATE_INTERVAL 5000  // 5 seconds
#define MQTT_KEEPALIVE 60        // 60 seconds
#define FOTA_TIMEOUT_MS 5000     // 5 seconds for FOTA response
#define TRANSMISSION_DELAY 25    // 25ms delay between lines

// NTP Configuration
#define NTP_OFFSET_SECONDS 19800  // UTC+5:30 for India
#define NTP_SERVER "pool.ntp.org"

// MQTT Buffer Configuration
#define MQTT_BUFFER_SIZE 8192    // 8KB MQTT buffer

#endif