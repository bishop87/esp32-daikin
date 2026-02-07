#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>

// Simple logging macro
// Usage: LOG("Message"); or LOG("Value: %d", value);
// For now, wrapping Serial.printf
#define LOG(...)                                                               \
  Serial.printf(__VA_ARGS__);                                                  \
  Serial.println()
#define LOG_RAW(...) Serial.printf(__VA_ARGS__)

#endif // LOGGER_H
