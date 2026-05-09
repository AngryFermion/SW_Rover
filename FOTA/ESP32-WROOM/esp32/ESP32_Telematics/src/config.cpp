#include "config.h"

// WiFi credentials
const char* ssid = "ancit";
const char* password = "ancit123";

#if USE_AWS_IOT
// AWS IoT Core Configuration
const char* mqtt_server = "abyj343g32dcn-ats.iot.us-east-1.amazonaws.com";
const int mqtt_port = 8883;  // TLS port
const char* mqtt_username = "";  // Not used with certificates
const char* mqtt_password = "";  // Not used with certificates
const char* mqtt_client_id = "sdv-vehicle-001";  // AWS Thing name

// AWS MQTT Topics
const char* topic_fota = "sdv/vehicles/sdv-vehicle-001/fota/#";
const char* topic_data = "sdv/vehicles/sdv-vehicle-001/data/#";
const char* topic_smartwheels = "sdv/vehicles/sdv-vehicle-001/#";

#else
// Public MQTT Broker Configuration
const char* mqtt_server = "broker.emqx.io";//"test.mosquitto.org";
const int mqtt_port = 1883;//1884;
const char* mqtt_username = "rw";
const char* mqtt_password = "readwrite";
const char* mqtt_client_id = "ESP32_Tele_Demo";

// MQTT Topics
const char* topic_fota = "SmartWheels-Ancit/telematics/fota/#";
const char* topic_data = "SmartWheels-Ancit/telematics/data/#";
const char* topic_smartwheels = "SmartWheels-Ancit/#";
#endif