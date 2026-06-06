/**
 * mqtt_config.h — SmartWheels ESP32-WROOM Telematics
 */

#ifndef MQTT_CONFIG_H
#define MQTT_CONFIG_H

// ===== BROKER =====
#define MQTT_BROKER_ADDRESS  "broker.hivemq.com"
#define MQTT_BROKER_PORT     1883
#define MQTT_CLIENT_ID       "smartwheels-wroom-ecu1"
#define MQTT_USERNAME        ""
#define MQTT_PASSWORD        ""

// ===== CONNECTION =====
#define MQTT_KEEP_ALIVE              60
#define MQTT_CLEAN_SESSION           true
#define MQTT_CONNECTION_TIMEOUT      10000
#define MQTT_RECONNECT_DELAY         5000
#define MQTT_MAX_RECONNECT_ATTEMPTS  5

// ===== TOPICS =====
#define MQTT_TOPIC_ROOT          "SmartKit/"
#define MQTT_TOPIC_CAN_SIGNALS   "Ultra"
#define MQTT_TOPIC_DEVICE_HEALTH "device_health"
#define MQTT_SUB_TOPIC           "SmartKit/Control"

// ===== QoS =====
#define MQTT_QOS_CAN_SIGNALS    1
#define MQTT_QOS_DEVICE_HEALTH  0

// ===== PUBLISH =====
#define MQTT_PUBLISH_CAN_IMMEDIATE    true
#define MQTT_PUBLISH_HEALTH_INTERVAL  5000
#define MQTT_RETAIN_CAN_SIGNALS       false
#define MQTT_RETAIN_DEVICE_HEALTH     false

// ===== BUFFERING =====
#define MQTT_ENABLE_BUFFERING    true
#define MQTT_BUFFER_SIZE         50
#define MQTT_BUFFER_DROP_OLDEST  true

// ===== DEVICE INFO =====
#define MQTT_DEVICE_ID        "WROOM_001"
#define MQTT_DEVICE_TYPE      "telematics"
#define MQTT_FIRMWARE_VERSION "1.0.0"

// ===== JSON BUFFERS =====
#define MQTT_JSON_BUFFER_SIZE        512
#define MQTT_JSON_CAN_BUFFER_SIZE    256
#define MQTT_JSON_HEALTH_BUFFER_SIZE 512

// ===== DEBUG =====
#define MQTT_DEBUG_ENABLED  1

#endif // MQTT_CONFIG_H
