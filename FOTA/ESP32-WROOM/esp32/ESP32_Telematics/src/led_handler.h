#ifndef LED_HANDLER_H
#define LED_HANDLER_H

#include <Arduino.h>
#include "config.h"

enum LEDState {
  PROJECT_SETUP,
  WIFI_CONNECTED,
  WIFI_GOT_IP,
  WIFI_DISCONNECTED
};

class LEDHandler {
private:
  LEDState currentState;
  unsigned long lastLEDUpdate;
  bool ledOn;

public:
  LEDHandler();
  void init();
  void updateLEDState();
  void setState(LEDState state);
  LEDState getState();
  String getLEDStateText(LEDState state);
  void flashGreen(int times = 1, int duration = 100);
  void flashBlue(int times = 1, int duration = 200);
  void flashRed(int times = 1, int duration = 200);
};

extern LEDHandler ledHandler;

#endif