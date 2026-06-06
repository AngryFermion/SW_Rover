#ifndef _HEADER_ANCIT_HTTP_CLIENT_H_
#define _HEADER_ANCIT_HTTP_CLIENT_H_

#include <Arduino.h>
#include <ArduinoJson.h>

struct HttpConfig {
  bool enabled;
  String data_format;
  String url;
  int port;
  int tx_interval;
  String auth_type;
  String username;
  String password;
  String token;
  String content_type;
};


extern HttpConfig http_config;

void HttpClient_LoadFromJson(JsonDocument& doc);
void HttpClient_ApplicationTask(void* param);
void HttpClient_CreateTask(void);
// void HttpClient_PostData(); 

#endif //_HEADER_ANCIT_HTTP_CLIENT_H_