#ifndef DAIKIN_STATE_H
#define DAIKIN_STATE_H

#include <Arduino.h>

struct DaikinState {
  float targetTemp = 0.0;
  float roomTemp = 0.0;
  float outsideTemp = 0.0;

  bool power = false;
  uint8_t mode = 0; // To be defined
  uint8_t fan = 0;  // To be defined

  // Decodes a raw S21 frame (STX included or excluded? Let's assume Payload
  // only or Full Frame) Better to pass pointer to Payload and Length?
  // s21_driver passes Full Frame usually?
  // Let's pass the payload (after STX, before ETX) for easier parsing?
  // Or full frame to verify checksum?
  // Let's stick to Full Frame for robustness.
  // Decodes a raw S21 frame
  void decodeFrame(const uint8_t *frame, size_t len);

  // Send a command to set the state
  void setDaikinState(bool power, uint8_t mode, float temp, uint8_t fan);

private:
  float parseInvertedDecimal(const uint8_t *ptr, size_t len);
};

extern DaikinState State;

#endif // DAIKIN_STATE_H
