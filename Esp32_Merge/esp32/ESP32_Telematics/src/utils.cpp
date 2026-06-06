#include "utils.h"
#include <time.h>

String getTimestamp() {
  time_t now = time(0);
  struct tm* timeinfo = localtime(&now);
  char buffer[20];
  strftime(buffer, sizeof(buffer), "%H:%M:%S", timeinfo);
  return String(buffer);
}