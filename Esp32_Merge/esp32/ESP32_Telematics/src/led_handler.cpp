#include "led_handler.h"

LEDHandler ledHandler;

LEDHandler::LEDHandler() {
  currentState = PROJECT_SETUP;
  lastLEDUpdate = 0;
  ledOn = false;
}

void LEDHandler::init() {
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(BLUE_LED, OUTPUT);

  digitalWrite(RED_LED, LED_OFF);
  digitalWrite(GREEN_LED, LED_OFF);
  digitalWrite(BLUE_LED, LED_OFF);
}

void LEDHandler::updateLEDState() {
  unsigned long currentTime = millis();

  switch(currentState) {
    case PROJECT_SETUP:
      if(currentTime - lastLEDUpdate >= LED_UPDATE_FAST) {
        ledOn = !ledOn;
        digitalWrite(RED_LED, ledOn ? LED_ON : LED_OFF);
        digitalWrite(GREEN_LED, LED_OFF);
        digitalWrite(BLUE_LED, LED_OFF);
        lastLEDUpdate = currentTime;
      }
      break;

    case WIFI_CONNECTED:
      if(currentTime - lastLEDUpdate >= LED_UPDATE_SLOW) {
        ledOn = !ledOn;
        digitalWrite(RED_LED, LED_OFF);
        digitalWrite(GREEN_LED, LED_OFF);
        digitalWrite(BLUE_LED, ledOn ? LED_ON : LED_OFF);
        lastLEDUpdate = currentTime;
      }
      break;

    case WIFI_GOT_IP:
      digitalWrite(RED_LED, LED_OFF);
      digitalWrite(GREEN_LED, LED_OFF);
      digitalWrite(BLUE_LED, LED_ON);
      break;

    case WIFI_DISCONNECTED:
      if(currentTime - lastLEDUpdate >= LED_UPDATE_FAST) {
        ledOn = !ledOn;
        digitalWrite(RED_LED, LED_OFF);
        digitalWrite(GREEN_LED, LED_OFF);
        digitalWrite(BLUE_LED, ledOn ? LED_ON : LED_OFF);
        lastLEDUpdate = currentTime;
      }
      break;
  }
}

void LEDHandler::setState(LEDState state) {
  currentState = state;
}

LEDState LEDHandler::getState() {
  return currentState;
}

String LEDHandler::getLEDStateText(LEDState state) {
  switch(state) {
    case PROJECT_SETUP: return "PROJECT_SETUP";
    case WIFI_CONNECTED: return "WIFI_CONNECTED";
    case WIFI_GOT_IP: return "WIFI_GOT_IP";
    case WIFI_DISCONNECTED: return "WIFI_DISCONNECTED";
    default: return "UNKNOWN";
  }
}

void LEDHandler::flashGreen(int times, int duration) {
  for (int i = 0; i < times; i++) {
    digitalWrite(GREEN_LED, LED_ON);
    delay(duration);
    digitalWrite(GREEN_LED, LED_OFF);
    if (i < times - 1) delay(duration);
  }
}

void LEDHandler::flashBlue(int times, int duration) {
  for (int i = 0; i < times; i++) {
    digitalWrite(BLUE_LED, LED_ON);
    delay(duration);
    digitalWrite(BLUE_LED, LED_OFF);
    if (i < times - 1) delay(duration);
  }
}

void LEDHandler::flashRed(int times, int duration) {
  for (int i = 0; i < times; i++) {
    digitalWrite(RED_LED, LED_ON);
    delay(duration);
    digitalWrite(RED_LED, LED_OFF);
    if (i < times - 1) delay(duration);
  }
}