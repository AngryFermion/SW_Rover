#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <config_reader.h>
#include <main.h>
#include <ancit_device.h>
#include <AncitRgbLed.h>
// #include <ancit_nvs.h>
#include <ancit_server.h>
// #include <ancit_wifi.h>
#include <ancit_device.h>
#include <ancit_wifi_manager.h>
#include "BT_LOGGER.h"

extern EventGroupHandle_t WifiManager_event_group;

bool file_send_inprogress = false;

// Global timestamp for display data updates
String g_display_timestamp = "";

extern uint8_t calibrateId;
extern uint16_t prevRawValue;

// Narayan - this is not implemented yet - need to check this...
/* webserver object on port 80 */
AsyncWebServer server(80);

void setup_server(void) {
  /* Enable ESP Server function and start webserver */
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    server_handle_root(request);
    // server_update_Wifi_config(request);
  });

  server.on("/updateWifiConfig", HTTP_POST, [](AsyncWebServerRequest *request) {
    server_update_Wifi_config(request);
  });

  server.on("/getDeviceConfig", HTTP_POST, [](AsyncWebServerRequest *request) {
    server_get_device_config(request);
  });


  server.on("/resetDevice", HTTP_POST, [](AsyncWebServerRequest *request) {
    server_reset_device(request);
  });

  server.on("/ping", HTTP_POST, [](AsyncWebServerRequest *request) {
    server_ping_device(request);
  });
  
  server.on("/getWifiDevices", HTTP_POST, [](AsyncWebServerRequest *request) {
    // Narayan - this function is not implemented yet
    //  ancit_wifi_scan_networks(request);
  });

  // Start a WiFi scan (returns list of networks)
  server.on("/initWifiScan", HTTP_POST, [](AsyncWebServerRequest *request) {
    server_init_wifi_scan(request);
  });

  server.on("/getWifiScanResult", HTTP_POST,
            [](AsyncWebServerRequest *request) {
              server_get_wifi_scan_result(request);
            }); 


  server.on("/saveDevCommConfig", HTTP_POST,
            [](AsyncWebServerRequest *request) {
              server_save_dev_comm_config(request);
            });

  server.on("/saveRegistration", HTTP_POST, [](AsyncWebServerRequest *request) {
    server_save_registration(request);
  });

  server.on("/listModbusDrivers", HTTP_POST,
            [](AsyncWebServerRequest *request) {
              server_list_modbus_drivers(request);
            });

  server.on("/getActiveModbusDriver", HTTP_POST,
            [](AsyncWebServerRequest *request) {
              server_get_active_modbus_driver(request);
            });


  server.on("/findDevice", HTTP_POST, [](AsyncWebServerRequest *request) {
    server_find_device(request);
  });

  server.on("/getDeviceId", HTTP_POST, [](AsyncWebServerRequest *request) {
    server_get_device_id(request);
  });

  server.on("/getDeviceParams", HTTP_POST, [](AsyncWebServerRequest *request) {
    String response;
    serializeJson(device_doc, response);
    request->send(200, "application/json", response);
  });

  server.on("/load_display", HTTP_GET, [](AsyncWebServerRequest *request) {
    server_load_display(request);
  });

  server.on("/save_display", HTTP_POST, [](AsyncWebServerRequest *request) {
    server_save_display(request);
  });

  server.on("/check_display_timestamp", HTTP_GET, [](AsyncWebServerRequest *request) {
    JsonDocument doc;
    JsonObject retVal = doc.to<JsonObject>();
    retVal["timestamp"] = g_display_timestamp;

    String response;
    serializeJson(retVal, response);
    request->send(200, "application/json", response);
  });

  server.onNotFound([](AsyncWebServerRequest *request) {
    if (!server_handle_file_read(request)) {
      request->send(404, "text/plain",
                    "404: Not Found");  // otherwise, respond with a 404
                                        // (Not Found) error
    } else {
      // Narayan - change this, put this function separately...
    }
  });

  server.begin();
}

bool server_handle_file_read(
    AsyncWebServerRequest
        *request) {  // send the right file to the client (if it exists)
  String path = request->url().c_str();

  if (path.endsWith("/")) {
    path += "views/index.html";  // If a folder is requested, send the index
                                 // file
  }
  String contentType = get_content_type(path);  // Get the MIME type
  String pathWithGz = path + ".gz";
  bool useGzipVersion = SPIFFS.exists(pathWithGz) && !SPIFFS.exists(path);

  if (useGzipVersion || SPIFFS.exists(path)) {  // If the file exists, either as a compressed
                                                 // archive, or normal

    if (useGzipVersion) {  // Only use compressed version if original doesn't exist
      path += ".gz";       // Use the compressed version

      g_Logger.Write(LogLevel::Info, LogCategory::SERVER,
                       "server_handle_file_read", "Sending compressed: %s",
                       path.c_str());

      AsyncWebServerResponse *response =
          request->beginResponse(SPIFFS, path, contentType, false);

      if (response != NULL) {
        response->addHeader("Content-Encoding", "gzip");
        response->addHeader("Cache-Control", "public, max-age=86400"); // Cache for 24 hours
        request->send(response);
      } else {
        g_Logger.Write(LogLevel::Error, LogCategory::SERVER,
                       "server_handle_file_read", "Failed to create response for: %s",
                       path.c_str());
        request->send(404, "text/plain", "File not found");
      }

    } else {
      g_Logger.Write(LogLevel::Info, LogCategory::SERVER,
                       "server_handle_file_read", "Sending uncompressed: %s",
                       path.c_str());

      AsyncWebServerResponse *response =
          request->beginResponse(SPIFFS, path, contentType, false);
      if (response != NULL) {
        response->addHeader("Cache-Control", "public, max-age=86400"); // Cache for 24 hours
        request->send(response);
      } else {
        g_Logger.Write(LogLevel::Error, LogCategory::SERVER,
                       "server_handle_file_read", "Failed to create response for: %s",
                       path.c_str());
        request->send(404, "text/plain", "File not found");
      }
    }

    return true;
  } else {
    g_Logger.Write(LogLevel::Error, LogCategory::SERVER, "server_handle_file_read",
                     "File Not Found: %s", path.c_str());
    return false;  // If the file doesn't exist, return false
  }
  return false;
}

String get_content_type(String filename) {
  if (filename.endsWith(".htm"))
    return "text/html";
  else if (filename.endsWith(".html"))
    return "text/html";
  else if (filename.endsWith(".css"))
    return "text/css";
  else if (filename.endsWith(".js"))
    return "application/javascript";
  else if (filename.endsWith(".png"))
    return "image/png";
  else if (filename.endsWith(".gif"))
    return "image/gif";
  else if (filename.endsWith(".jpg"))
    return "image/jpeg";
  else if (filename.endsWith(".ico"))
    return "image/x-icon";
  else if (filename.endsWith(".xml"))
    return "text/xml";
  else if (filename.endsWith(".pdf"))
    return "application/x-pdf";
  else if (filename.endsWith(".zip"))
    return "application/x-zip";
  else if (filename.endsWith(".gz"))
    return "application/x-gzip";
  else if (filename.endsWith(".json"))
    return "application/json";
  return "text/plain";
}

// This function funtions returns true  in AP mode
bool check_config_mode() {
  // Narayan - remove this line in production...
  // return true;
  if (WiFi.getMode() == WIFI_MODE_AP) {
    return true;
  } else {
    return false;
  }
}

void parse_bytes(const char *str, char sep, byte *bytes, int maxBytes,
                 int base) {
  for (int i = 0; i < maxBytes; i++) {
    bytes[i] = strtoul(str, NULL, base);  // Convert byte
    str = strchr(str, sep);               // Find next separator
    if (str == NULL || *str == '\0') {
      break;  // No more separators, exit
    }
    str++;  // Point to next character after separator
  }
}

/* This handling root file */
void server_handle_root(AsyncWebServerRequest *request) {
  String path = "/views/index.html";
  String contentType = "text/html";

  if (SPIFFS.exists(path)) {
    file_send_inprogress = true;
    request->send(SPIFFS, path, contentType);
    file_send_inprogress = false;
  } else {
    request->send(200, "text/plain", "DisplayBoard.html not found...");
  }
}

#if 1  // narayan - this is not implemented yet
void server_update_Wifi_config(AsyncWebServerRequest *request) {
  // allocate the memory for the document
  JsonDocument doc;
  String response;
  String prtStr = "";

  // create an object
  JsonObject retVal = doc.to<JsonObject>();
  retVal["result"] = 0;
  retVal["message"] = "Unknown response...";

  // Check if all arguments are presents,
  // else return...
  if (request->hasArg("wifi_config")) {
    g_Logger.Write(LogLevel::Info, LogCategory::SERVER,
                     "server_update_Wifi_config", "wifi_config: %s",
                     request->arg("wifi_config").c_str());
  } else {
    retVal["result"] = 0;
    retVal["message"] = "Invalid parameters....";

    serializeJson(retVal, response);
    request->send(200, "text/json", response);
    return;
  }

  // if request from active mode, do not configure...
  if (check_config_mode() == true) {
    if (request->arg("wifi_test").equals("1")) {
      deserializeJson(wifi_doc, request->arg("wifi_config"));
      // Set the wifi test mode event bit
      xEventGroupSetBits(WifiManager_event_group, EVENT_WIFI_TEST_MODE_BIT);
      retVal["result"] = 1;
      retVal["message"] = "Wifi testing initiated...";
    } else {
      File file = SPIFFS.open("/config/wifi.json", "w");

      if (!file) {
        retVal["result"] = 0;
        retVal["message"] = "Wifi - Error opening file for writing";
      }

      int bytesWritten = file.print(request->arg("wifi_config"));
      file.close();

      if (bytesWritten > 0) {
        retVal["result"] = 1;
        retVal["message"] = "Wifi configuration Saved...";
      } else {
        retVal["result"] = 1;
        retVal["message"] = "Wifi - File write failed...";
      }
    }
  } else {
    retVal["result"] = 1;
    retVal["message"] = "Unable to configure in active mode...";
  }

  serializeJson(retVal, response);
  g_Logger.Write(LogLevel::Info, LogCategory::SERVER, "server_update_Wifi_config",
                   "response: %s", response.c_str());

  request->send(200, "text/json", response);
}
#endif

void server_get_device_config(AsyncWebServerRequest *request) {
  String deviceConfig;
  serializeJson(device_doc, deviceConfig);
  request->send(200, "text/json", deviceConfig);
}


// Scan nearby WiFi networks and return JSON list
void server_init_wifi_scan(AsyncWebServerRequest *request) {
  JsonDocument doc;
  JsonObject ret = doc.to<JsonObject>();


  ret["result"] = 1;
  ret["message"] = "Scan started";

  String resp;
  serializeJson(ret, resp);
  
  g_Logger.Write(LogLevel::Info, LogCategory::SERVER, "server_init_wifi_scan",
                   "Scan request received, starting scan...");
  // Set up callback to trigger scan only after response is fully sent
  request->onDisconnect([](){
    xEventGroupSetBits(WifiManager_event_group, EVENT_WIFI_SCAN_REQUEST);
  });
  
  request->send(200, "application/json", resp);
}

void server_reset_device(AsyncWebServerRequest *request) {
  // allocate the memory for the document
  JsonDocument doc;
  String response;

  // create an object
  JsonObject retVal = doc.to<JsonObject>();
  retVal["result"] = 0;
  retVal["message"] = "Device is being restarted..";

  serializeJson(retVal, response);
  request->send(200, "text/json", response);

  g_Logger.Write(LogLevel::Warn, LogCategory::SERVER, "server_reset_device",
                   "Device is being restarted by client..");

  // params.drc++;
  // ancit_nvs_commit_val("params", "drc", params.drc);

  // user restart from front end, restart in active mode...
  // ancit_nvs_commit_val("params", "forced_ap", FORCED_AP_NVS_FALSE);

  // restart device
  ancit_device_restart();
}

void server_get_active_modbus_driver(AsyncWebServerRequest *request) {
  // allocate the memory for the document
  JsonDocument doc;
  String response;

  JsonDocument mmd_doc;

  // create an object
  JsonObject retVal = doc.to<JsonObject>();

  retVal["result"] = 0;
  retVal["message"] = "Unknown response...";

  if ((request->hasArg("app_key"))) {
    retVal["message"] = "Modbus driver functionality has been removed";
  } else {
    retVal["message"] = "Invalid parameters....";
  }

  if (retVal["result"] == 0) {
    serializeJson(retVal, response);
    g_Logger.Write(LogLevel::Debug, LogCategory::SERVER, "get_mmds",
                     "response: %150s", response.c_str());
    request->send(200, "text/json", response);
  }
}

void server_find_device(AsyncWebServerRequest *request) {
  JsonDocument doc;
  String response;

  // create an object
  JsonObject retVal = doc.to<JsonObject>();

  retVal["result"] = 0;
  retVal["message"] = "Unknown response...";

  if ((request->hasArg("app_key"))) {
    if (request->arg("app_key").equals("FoTA")) {
      String fName = "/config/device.json";

      AsyncWebServerResponse *response =
          request->beginResponse(SPIFFS, fName, "text/json", false);
      if (response != NULL) {
        retVal["result"] = 1;
        file_send_inprogress = true;
        request->send(response);
        file_send_inprogress = false;
      } else {
        retVal["message"] = "Unable to send driver file....";
      }
    }
  }

  if (retVal["result"] == 0) {
    serializeJson(retVal, response);
    g_Logger.Write(LogLevel::Debug, LogCategory::SERVER, "get_mmds",
                     "response: %150s", response.c_str());
    request->send(200, "text/json", response);
  }
}

void server_get_device_id(AsyncWebServerRequest *request) {
  JsonDocument doc;
  String response;

  // create an object
  JsonObject retVal = doc.to<JsonObject>();

  retVal["result"] = 0;
  retVal["message"] = "Unknown response...";

  if ((request->hasArg("app_key"))) {
    if (request->arg("app_key").equals("FoTA")) {
      retVal["result"] = 1;
      retVal["message"] = "Response ok...";

      retVal["dev_id"] = device_doc["dev_id"];

      retVal["logo"] = registration_doc["oem"]["logo"];
      retVal["product_name"] = registration_doc["oem"]["product_name"];
      retVal["product_version"] = registration_doc["oem"]["product_version"];
      retVal["footer_text"] = registration_doc["oem"]["footer_text"];
      retVal["www"] = registration_doc["oem"]["www"];

      retVal["max_rtu_drivers"] = registration_doc["meti"]["max_rtu_drivers"];
      retVal["hw_version"] = registration_doc["meti"]["hw_version"];
      retVal["fw_version"] = registration_doc["meti"]["fw_version"];
      retVal["variant"] = registration_doc["meti"]["variant"];
    }
  }

  serializeJson(retVal, response);
  g_Logger.Write(LogLevel::Debug, LogCategory::SERVER, "get_dev_id",
                   "response: %s", response.c_str());
  request->send(200, "text/json", response);
}

void server_save_dev_comm_config(AsyncWebServerRequest *request) {
  // allocate the memory for the document
  JsonDocument doc;
  String response;
  String prtStr = "";

  // create an object
  JsonObject retVal = doc.to<JsonObject>();
  retVal["result"] = 0;
  retVal["message"] = "Unknown response...";

  // Check if all arguments are presents,
  // else return...
  if (request->hasArg("dev_comm_config") && request->hasArg("device_mode")) {
    g_Logger.Write(LogLevel::Debug, LogCategory::SERVER,
                     "server_save_dev_comm_config", "device_config: %s",
                     request->arg("dev_comm_config").c_str());
  } else {
    retVal["result"] = 0;
    retVal["message"] = "Invalid parameters....";

    serializeJson(retVal, response);
    request->send(200, "text/json", response);
    return;
  }

  // if request from active mode, do not configure...
  // exception, if device registration mode, allow configuration
  if (check_config_mode() == true || request->arg("device_mode") == "1") {
    File file = SPIFFS.open("/config/devComm.json", "w");

    if (!file) {
      retVal["result"] = 0;
      retVal["message"] = "Error opening file for writing !!";
    }

    int bytesWritten = file.print(request->arg("dev_comm_config"));
    file.close();

    if (bytesWritten > 0) {
      retVal["result"] = 1;
      retVal["message"] = "Device configuration Saved...";
    } else {
      retVal["result"] = 0;
      retVal["message"] = "Device file write failed...";
    }
  } else {
    retVal["result"] = 0;
    retVal["message"] = "Unable to configure in active mode....";
  }

  serializeJson(retVal, response);
  g_Logger.Write(LogLevel::Debug, LogCategory::SERVER,
                   "server_save_dev_comm_config", "response: %s",
                   response.c_str());

  request->send(200, "text/json", response);
}

void server_save_registration(AsyncWebServerRequest *request) {
  // allocate the memory for the document
  JsonDocument doc;
  String response;
  String prtStr = "";
  int bytesWritten = 0;

  // create an object
  JsonObject retVal = doc.to<JsonObject>();
  retVal["result"] = 0;
  retVal["message"] = "Unknown response...";

  // Check if all arguments are presents,
  // else return...
  if (request->hasArg("registration") && request->hasArg("device_mode")) {
    g_Logger.Write(LogLevel::Debug, LogCategory::SERVER,
                     "server_save_registration", "registration: %s",
                     request->arg("registration").c_str());
  } else {
    retVal["result"] = 0;
    retVal["message"] = "Invalid parameters....";

    serializeJson(retVal, response);
    request->send(200, "text/json", response);
    return;
  }

  // if request from active mode, do not configure...
  // exception, if device registration mode, allow configuration
  if (check_config_mode() == true || request->arg("device_mode") == "1") {
    File file = SPIFFS.open("/config/registration.json", "w");

    if (!file) {
      retVal["result"] = 0;
      retVal["message"] = "Error opening file for writing !!";
    } else {
      bytesWritten = file.print(request->arg("registration"));
    }

    file.close();

    if (bytesWritten > 0) {
      retVal["result"] = 1;
      retVal["message"] = "Registration configuration Saved...";
    } else {
      retVal["result"] = 0;
      retVal["message"] = "Registration file write failed...";
    }
  } else {
    retVal["result"] = 0;
    retVal["message"] = "Unable to save in active mode....";
  }

  serializeJson(retVal, response);
  g_Logger.Write(LogLevel::Debug, LogCategory::SERVER, "server_save_registration",
                   "response: %s", response.c_str());

  request->send(200, "text/json", response);
}

void server_list_modbus_drivers(AsyncWebServerRequest *request) {
  JsonDocument doc;
  JsonArray result = doc.to<JsonArray>();

  g_Logger.Write(LogLevel::Debug, LogCategory::SERVER,
                   "server_list_modbus_drivers", "Listing Modbus drivers...");
  File dir = SPIFFS.open("/drivers");
  if (!dir) {
    g_Logger.Write(LogLevel::Error, LogCategory::SERVER,
                     "server_list_modbus_drivers",
                     "Failed to open drivers directory");
    request->send(200, "application/json", "[]");
    return;
  }

  g_Logger.Write(LogLevel::Debug, LogCategory::SERVER,
                   "server_list_modbus_drivers", "Opened drivers directory");

  File file = dir.openNextFile();
  while (file) {
    String filename = file.name();
    g_Logger.Write(LogLevel::Debug, LogCategory::SERVER,
                     "server_list_modbus_drivers", "Found file: %s",
                     filename.c_str());
    if (filename.startsWith("mmd_") && filename.endsWith(".json")) {
      g_Logger.Write(LogLevel::Debug, LogCategory::SERVER,
                       "server_list_modbus_drivers",
                       "Processing driver file: %s", filename.c_str());
      String fileContent;
      while (file.available()) {
        fileContent += (char)file.read();
      }
      JsonDocument driverDoc;
      DeserializationError err = deserializeJson(driverDoc, fileContent);
      String meter, name;
      if (!err) {
        meter = driverDoc["meter"] | "";
        name = driverDoc["name"] | "";
      } else {
        meter = "";
        name = "";
      }
      JsonObject item = result.add<JsonObject>();
      item["json_filename"] = file.name();
      item["meter"] = meter;
      item["name"] = name;
    }
    file = dir.openNextFile();
  }
  dir.close();

  String response;
  serializeJson(result, response);
  request->send(200, "application/json", response);
}

void server_get_wifi_scan_result(AsyncWebServerRequest *request) {
  String response;
  extern JsonDocument cached_scan_result;

  serializeJson(cached_scan_result, response);
  request->send(200, "application/json", response);
}

void server_ping_device(AsyncWebServerRequest *request) {
  request->send(200, "text/plain", "OK");
}

void server_load_display(AsyncWebServerRequest *request) {
  JsonDocument doc;
  String response;
  JsonObject retVal = doc.to<JsonObject>();

  retVal["result"] = 0;
  retVal["message"] = "Unknown response...";

  String fName = "/config/display_board.json";

  // Check if file exists
  if (SPIFFS.exists(fName)) {
    // Read the file content
    File file = SPIFFS.open(fName, "r");
    if (file) {
      file_send_inprogress = true;

      String fileContent = "";
      while (file.available()) {
        fileContent += (char)file.read();
      }
      file.close();

      // Try to parse the file content as JSON
      JsonDocument fileDoc;
      DeserializationError error = deserializeJson(fileDoc, fileContent);

      if (!error) {
        // Successfully parsed - wrap it in the expected response format
        retVal["result"] = 1;
        retVal["message"] = "Display config loaded successfully";
        retVal["data"] = fileDoc;

        // Update global timestamp from loaded data
        if (fileDoc["timestamp"].is<const char*>()) {
          g_display_timestamp = fileDoc["timestamp"].as<String>();
        }
      } else {
        // Failed to parse - return raw content for backward compatibility
        retVal["result"] = 1;
        retVal["message"] = "Display config loaded (raw format)";
        retVal["data"] = fileContent;
      }

      serializeJson(retVal, response);
      g_Logger.Write(LogLevel::Debug, LogCategory::SERVER, "server_load_display",
                       "response length: %d", response.length());
      request->send(200, "application/json", response);
      file_send_inprogress = false;
    } else {
      retVal["message"] = "Unable to read display config file...";
      serializeJson(retVal, response);
      g_Logger.Write(LogLevel::Debug, LogCategory::SERVER, "server_load_display",
                       "response: %s", response.c_str());
      request->send(200, "application/json", response);
    }
  } else {
    retVal["message"] = "Display config not found...";
    serializeJson(retVal, response);
    g_Logger.Write(LogLevel::Debug, LogCategory::SERVER, "server_load_display",
                     "response: %s", response.c_str());
    request->send(200, "application/json", response);
  }
}

void server_save_display(AsyncWebServerRequest *request) {
  JsonDocument doc;
  String response;
  JsonObject retVal = doc.to<JsonObject>();
  retVal["result"] = 0;
  retVal["message"] = "Unknown response...";

  if (request->hasArg("display_data")) {
    g_Logger.Write(LogLevel::Info, LogCategory::SERVER,
                     "server_save_display", "display_data received");
  } else {
    retVal["result"] = 0;
    retVal["message"] = "Invalid parameters....";

    serializeJson(retVal, response);
    request->send(200, "application/json", response);
    return;
  }

  File file = SPIFFS.open("/config/display_board.json", "w");

  if (!file) {
    retVal["result"] = 0;
    retVal["message"] = "Error opening file for writing";
  } else {
    int bytesWritten = file.print(request->arg("display_data"));
    file.close();

    if (bytesWritten > 0) {
      retVal["result"] = 1;
      retVal["message"] = "Display configuration saved...";

      // Update global timestamp from saved data
      JsonDocument savedDoc;
      DeserializationError error = deserializeJson(savedDoc, request->arg("display_data"));
      if (!error && savedDoc["timestamp"].is<const char*>()) {
        g_display_timestamp = savedDoc["timestamp"].as<String>();
      }
    } else {
      retVal["result"] = 0;
      retVal["message"] = "Display file write failed...";
    }
  }

  serializeJson(retVal, response);
  g_Logger.Write(LogLevel::Info, LogCategory::SERVER, "server_save_display",
                   "response: %s", response.c_str());

  request->send(200, "application/json", response);
} 