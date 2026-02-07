#include "daikin_state.h"
#include "../system/config.h"
#include "../system/logger.h"
#include "s21_driver.h"

DaikinState State;

// Helper to parse Daikin's weird inverted text numbers
// Format: "570+" -> "+075" -> 7.5
// Format: "091+" -> "+190" -> 19.0
float DaikinState::parseInvertedDecimal(const uint8_t *ptr, size_t len) {
  if (len < 4)
    return 0.0;

  char buf[8];
  // Reorder bytes: 3, 2, 1, 0  (Assuming input is [0,1,2,3])
  buf[0] = ptr[3]; // Sign
  buf[1] = ptr[2];
  buf[2] = ptr[1];
  buf[3] = ptr[0];
  buf[4] = '\0';

  // Parse as integer
  int raw = atoi(buf);

  // Convert to float (divide by 10 for 0.1C resolution usually)
  return raw / 10.0;
}

void DaikinState::decodeFrame(const uint8_t *frame, size_t len) {
  // Basic validation
  if (len < 5)
    return;
  if (frame[0] != 0x02)
    return; // Not STX

  // Extract Type
  uint8_t type1 = frame[1];
  uint8_t type2 = frame[2];

  // Handle specific packets
  if (type1 == 'S') {   // Status Response
    if (type2 == 'H') { // Room Temperature
      // Payload at index 3?
      // Frame: 02 S H [0 9 1 +] CS 03
      if (len >= 8) {
        float val = parseInvertedDecimal(&frame[3], 4);
        this->roomTemp = val;
        LOG("Parsed Room Temp (SH): %.1f C", val);
      }
    } else if (type2 == 'a') { // Outside Temperature (or Power?)
      // Frame: 02 S a [5 7 0 +] CS 03
      if (len >= 8) {
        float val = parseInvertedDecimal(&frame[3], 4);
        this->outsideTemp = val + OUTSIDE_TEMP_OFFSET;
        LOG("Parsed Outside Temp (Sa): %.1f C (raw: %.1f)", this->outsideTemp,
            val);
      }
    }
  } else if (type1 == 'G') { // Handle 'G' Packets
    // Log raw for debugging
    Serial.printf("RX Packet G%c: ", type2);
    for (size_t i = 3; i < len - 2; i++) {
      if (frame[i] < 0x10)
        Serial.print("0");
      Serial.print(frame[i], HEX);
      Serial.print(" ");
    }
    Serial.println();

    // Decode G1: Power, Mode, Temp, Fan
    if (type2 == '1' && len >= 7) {
      // Byte 0: Power ('1' = ON, '0' = OFF)
      this->power = (frame[3] == '1');
      LOG("Parsed Power (G1): %s", this->power ? "ON" : "OFF");

      uint8_t modeChar = frame[4]; // Byte 1 of payload
      switch (modeChar) {
      case '0':
        this->mode = 0;
        LOG("Parsed Mode (G1): Auto (0)?");
        break;
      case '1':
        this->mode = 1;
        LOG("Parsed Mode (G1): Auto");
        break;
      case '2':
        this->mode = 2;
        LOG("Parsed Mode (G1): Dry");
        break;
      case '3':
        this->mode = 3;
        LOG("Parsed Mode (G1): Cool");
        break;
      case '4':
        this->mode = 4;
        LOG("Parsed Mode (G1): Heat");
        break;
      case '6':
        this->mode = 6;
        LOG("Parsed Mode (G1): Fan");
        break;
      default:
        LOG("Parsed Mode (G1): Unknown (%c)", modeChar);
        break;
      }

      // Byte 2: Target Temp
      uint8_t tempRaw = frame[5];
      float tempC = (tempRaw - 32) / 1.8;
      this->targetTemp = tempC;
      LOG("Parsed Target (G1): %.1f C (Raw: %d F)", tempC, tempRaw);

      // Byte 3: Fan Speed
      uint8_t fanRaw = frame[6];
      if (fanRaw >= 0x30 && fanRaw <= 0x39) {
        this->fan = fanRaw - 0x32;
        LOG("Parsed Fan (G1): %d (Raw: %02X)", this->fan, fanRaw);
      } else if (fanRaw == 0x41) {
        this->fan = 10;
        LOG("Parsed Fan (G1): Auto (Raw: A)");
      } else if (fanRaw == 0x42) {
        this->fan = 11;
        LOG("Parsed Fan (G1): Silent (Raw: B)");
      } else {
        LOG("Parsed Fan (G1): Unknown (Raw: %02X)", fanRaw);
      }
    }
  }
}

// Send a command to set the state
// Payload Structure: 'D' '1' [Power] [Mode] [TempF] [Fan]
void DaikinState::setDaikinState(bool power, uint8_t mode, float temp,
                                 uint8_t fan) {
  uint8_t payload[6];

  // Header
  payload[0] = 'D';
  payload[1] = '1';

  // Byte 0: Power (Hypothesis: '1'=ON, '0'=OFF)
  payload[2] = power ? '1' : '0';

  // Mode
  // 1=Auto, 2=Dry, 3=Cool, 4=Heat, 6=Fan
  char modeChar = '0';
  switch (mode) {
  case 0:
    modeChar = '0';
    break; // Auto?
  case 1:
    modeChar = '1';
    break;
  case 2:
    modeChar = '2';
    break;
  case 3:
    modeChar = '3';
    break;
  case 4:
    modeChar = '4';
    break;
  case 6:
    modeChar = '6';
    break;
  default:
    modeChar = '3';
    break; // Default Cool
  }
  payload[3] = modeChar;

  // Temp (Celsius -> Fahrenheit Integer)
  int tempF = (int)((temp * 1.8) + 32);
  payload[4] = (uint8_t)tempF;

  // Fan
  // 1-5 -> '3'-'7'
  // 10 -> 'A' (Auto)
  // 11 -> 'B' (Silent)
  uint8_t fanChar = 'A';
  if (fan >= 1 && fan <= 5) {
    fanChar = fan + 0x32;
  } else if (fan == 10) {
    fanChar = 'A';
  } else if (fan == 11) {
    fanChar = 'B';
  }
  payload[5] = fanChar;

  LOG("Sending Set Packet: Power=%s, Mode=%c, TempF=%d, Fan=%c",
      power ? "ON" : "OFF", modeChar, tempF, fanChar);

  S21.sendFrame(payload, 6);
}
