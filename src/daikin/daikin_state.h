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

  // Swing state
  bool swingV = false;
  bool swingH = false;

  // Decodes a raw S21 frame
  void decodeFrame(const uint8_t *frame, size_t len);

  // Send a command to set the state
  void setDaikinState(bool power, uint8_t mode, float temp, uint8_t fan);
  
  // Send a command to set the swing state
  void setSwing(bool v, bool h);

private:
  float parseInvertedDecimal(const uint8_t *ptr, size_t len);
};

extern DaikinState State;

#endif // DAIKIN_STATE_H
