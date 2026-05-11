#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>
#include "SMART_LOGGER.h"

String getTimestamp();

// Global debug logger instance
extern SMART_LOGGER DebugLog;

#endif