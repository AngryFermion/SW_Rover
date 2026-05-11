#ifndef CONFIG_READER_H
#define CONFIG_READER_H

#include <ArduinoJson.h>

// External JSON documents
extern JsonDocument device_doc;
extern JsonDocument wifi_doc;
extern JsonDocument devComm_doc;
extern JsonDocument registration_doc;

// Public API
void read_device_file(void);
void read_wifi_file(void);
void read_devComm_file(void);
void read_registration_file(void);

#endif // CONFIG_READER_H
