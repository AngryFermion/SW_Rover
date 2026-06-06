#ifndef _HEADER_ANCIT_SERVER_H_
#define _HEADER_ANCIT_SERVER_H_
//Constatnts

//Variables
extern bool file_send_inprogress;

//Functions
String get_content_type(String filename); // convert the file extension to the MIME type

/* ESP Server functions*/
void setup_server(void);
bool server_handle_file_read(AsyncWebServerRequest *request); // send the right file to the client (if it exists)

// ESP url functions
void server_handle_root(AsyncWebServerRequest *request);
void server_update_Wifi_config(AsyncWebServerRequest *request);
void server_get_device_config(AsyncWebServerRequest *request);
void server_reset_device(AsyncWebServerRequest *request);
void server_get_active_modbus_driver(AsyncWebServerRequest *request);
void server_find_device(AsyncWebServerRequest *request);
void server_get_device_id(AsyncWebServerRequest *request);
void server_save_dev_comm_config(AsyncWebServerRequest *request);
void server_save_registration(AsyncWebServerRequest *request);
void ancit_wifi_scan_networks(AsyncWebServerRequest *request);
void ancit_wifi_initiate_scan(AsyncWebServerRequest *request);
void ancit_wifi_get_scan_result(AsyncWebServerRequest *request);
void server_init_wifi_scan(AsyncWebServerRequest *request);
bool check_mode(void);
void parse_bytes(const char *str, char sep, byte *bytes, int maxBytes, int base);
void server_list_modbus_drivers(AsyncWebServerRequest *request);
void server_get_wifi_scan_result(AsyncWebServerRequest *request);
void server_ping_device(AsyncWebServerRequest *request);
void server_load_display(AsyncWebServerRequest *request);
void server_save_display(AsyncWebServerRequest *request);

#endif