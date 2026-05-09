#include "SMART_LOGGER.h"
#include "config.h"
#include "led_handler.h"
#include "utils.h"
#include "wifi_handler.h"
#include <Arduino.h>

#if ENABLE_MQTT
# include "mqtt_handler.h"
#endif

#if ENABLE_FOTA
# include "fota_handler.h"
# include "fota_serial_handler.h"
# include "uart_bootloader.h"
#endif

#if ENABLE_CAN
# include "can_handler.h"
#endif

unsigned long	lastStatusUpdate = 0;
SMART_LOGGER	DebugLog(LogLevel::Info);

#if ENABLE_CAN
// Global logger pointer for CAN handler
SMART_LOGGER* myLogger = &DebugLog;

#endif

void	setup(void)
{
	Serial.begin(DEBUG_SERIAL_BAUD_RATE, SERIAL_8N1, DEBUG_SERIAL_RX_PIN,
		DEBUG_SERIAL_TX_PIN);
	// #if ENABLE_FOTA
	// 	Serial1.begin(FOTA_SERIAL_BAUD_RATE, SERIAL_8N1, FOTA_SERIAL_RX_PIN,
	// 		FOTA_SERIAL_TX_PIN);
	// #endif
	delay(500);
	DebugLog.Write(LogLevel::Info, LogCategory::SETUP, "setup",
		"SmartWheels Telematics and FOTA System");
	ledHandler.init();
	DebugLog.Write(LogLevel::Info, LogCategory::SETUP, "setup",
		"Project Setup Complete");
	wifiHandler.init();
	wifiHandler.begin();
#if ENABLE_MQTT
	mqttHandler.init();
#endif
#if ENABLE_FOTA
	fotaHandler.init();
	fotaSerialHandler.init();
#endif
#if ENABLE_CAN
	if (initializeCAN()) {
		DebugLog.Write(LogLevel::Info, LogCategory::SETUP, "setup", "CAN Handler initialized successfully");
	} else {
		DebugLog.Write(LogLevel::Error, LogCategory::SETUP, "setup", "CAN Handler initialization failed");
	}
#endif
}

void	loop(void)
{
	String	statusMsg;

	ledHandler.updateLEDState();
#if ENABLE_MQTT
	mqttHandler.handleConnection();
#endif
#if ENABLE_FOTA
	bool debugMode = false; // Set to true to force bootloader execution for testing
	if (fotaHandler.isReadyToTransmit() || debugMode == true)
	{
		char* buffer = fotaHandler.getBuffer();
		int bufferSize = fotaHandler.getBufferSize();

		DebugLog.Write(LogLevel::Info, LogCategory::SYSTEM, "loop", "Buffer: " + String((long)buffer, HEX) + ", Size: " + String(bufferSize));

		BootloaderResult result = uartBootloader.executeBootloaderSequence(buffer, bufferSize);

		if (result == BOOTLOADER_SUCCESS) {
			DebugLog.Write(LogLevel::Info, LogCategory::SYSTEM, "loop", "Bootloader sequence completed successfully");
		} else {
			DebugLog.Write(LogLevel::Error, LogCategory::SYSTEM, "loop", "Bootloader sequence failed with error code: " + String(result));
		}

		fotaHandler.setTransmissionComplete();
	}
#endif
#if ENABLE_CAN
	// Process incoming CAN messages
	processCANMessages();

	// Send dummy CAN message every 100ms
	sendDummyData();
#endif
	if (millis() - lastStatusUpdate >= STATUS_UPDATE_INTERVAL)
	{
		statusMsg = "LED State: "
			+ ledHandler.getLEDStateText(ledHandler.getState()) + ", WiFi: "
			+ wifiHandler.getWiFiStatusText(WiFi.status());
#if ENABLE_MQTT
		statusMsg += ", MQTT: "
			+ mqttHandler.getMQTTStateText(mqttHandler.isConnected() ? 0 : -1);
#endif
#if ENABLE_CAN
		statusMsg += ", CAN: " + String(isCANReady() ? "Ready" : "Not Ready");
#endif
		DebugLog.Write(LogLevel::Info, LogCategory::SYSTEM, "loop", statusMsg);
		lastStatusUpdate = millis();
	}
	delay(10);
}
