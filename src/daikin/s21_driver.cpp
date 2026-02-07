#include "s21_driver.h"
#include "../system/config.h"
#include "../system/logger.h"
#include "daikin_state.h"

S21Driver S21;

// Constants for State Machine
#define STATE_INIT_D20 0
#define STATE_WAIT_D20 1
#define STATE_INIT_F8 2
#define STATE_WAIT_F8 3
#define STATE_INIT_F2 4
#define STATE_WAIT_F2 5
#define STATE_INIT_F4 6
#define STATE_WAIT_F4 7
#define STATE_INIT_F3 8
#define STATE_WAIT_F3 9
#define STATE_INIT_F1 10
#define STATE_WAIT_F1 11
#define STATE_INIT_F5 12
#define STATE_WAIT_F5 13
#define STATE_INIT_D8 14
#define STATE_WAIT_D8 15
#define STATE_INIT_RH 16
#define STATE_WAIT_RH 17
#define STATE_INIT_RA 18
#define STATE_WAIT_RA 19
#define STATE_IDLE 100

// Internal flags (file scope, since we didn't add them to header)
static bool g_ackReceived = false;
static bool g_nakReceived = false;

void S21Driver::begin() {
  LOG("[S21] Initializing S21 Driver (Faikout Logic)...");

  // Hardware Setup
  pinMode(S21_RX_PIN, INPUT_PULLUP);
  Serial1.begin(S21_BAUD_RATE, S21_CONFIG, S21_RX_PIN, S21_TX_PIN);

  // Init State
  protocolState = STATE_INIT_D20;
  lastActionTime = millis();
  lastSuccessTime =
      millis(); // Assume connected at start to avoid immediate red error
  g_ackReceived = false;

  delay(100);
  LOG("[S21] Ready. RX:%d TX:%d", S21_RX_PIN, S21_TX_PIN);
}

void S21Driver::loop() {
  // 1. Process Incoming Data
  while (Serial1.available()) {
    processByte(Serial1.read());
  }

  // 2. Manage Protocol State
  pollState();
}

void S21Driver::pollState() {
  unsigned long now = millis();

  // Timeout handling for waits
  if (protocolState % 2 != 0) {        // Odd states are WAIT states
    if (now - lastActionTime > 1000) { // 1 second timeout
      Serial.println("Timeout waiting for ACK. Retrying...");
      protocolState--; // Go back to SEND state
      return;
    }
  }

  // State Machine
  switch (protocolState) {
  case STATE_INIT_D20:
    sendFrame((const uint8_t *)"D20", 3);
    protocolState = STATE_WAIT_D20;
    lastActionTime = now;
    g_ackReceived = false;
    break;

  case STATE_WAIT_D20:
    if (g_ackReceived) {
      Serial.println("Got ACK for D20. Moving to F8.");
      delay(50); // Small delay between commands
      protocolState = STATE_INIT_F8;
    }
    break;

  case STATE_INIT_F8:
    sendFrame((const uint8_t *)"F8", 2);
    protocolState = STATE_WAIT_F8;
    lastActionTime = now;
    g_ackReceived = false;
    break;

  case STATE_WAIT_F8:
    if (g_ackReceived) {
      Serial.println("Got ACK for F8. Moving to F2.");
      delay(50);
      protocolState = STATE_INIT_F2;
    }
    break;

  case STATE_INIT_F2:
    sendFrame((const uint8_t *)"F2", 2);
    protocolState = STATE_WAIT_F2;
    lastActionTime = now;
    g_ackReceived = false;
    break;
  case STATE_WAIT_F2:
    if (g_ackReceived) {
      delay(50);
      protocolState = STATE_INIT_F4;
    }
    break;

  case STATE_INIT_F4:
    sendFrame((const uint8_t *)"F4", 2);
    protocolState = STATE_WAIT_F4;
    lastActionTime = now;
    g_ackReceived = false;
    break;
  case STATE_WAIT_F4:
    if (g_ackReceived) {
      delay(50);
      protocolState = STATE_INIT_F3;
    }
    break;

  case STATE_INIT_F3:
    sendFrame((const uint8_t *)"F3", 2);
    protocolState = STATE_WAIT_F3;
    lastActionTime = now;
    g_ackReceived = false;
    break;
  case STATE_WAIT_F3:
    if (g_ackReceived) {
      delay(50);
      protocolState = STATE_INIT_F1;
    }
    break;

  case STATE_INIT_F1:
    sendFrame((const uint8_t *)"F1", 2);
    protocolState = STATE_WAIT_F1;
    lastActionTime = now;
    g_ackReceived = false;
    break;
  case STATE_WAIT_F1:
    if (g_ackReceived) {
      delay(50);
      protocolState = STATE_INIT_F5;
    }
    break;

  case STATE_INIT_F5:
    sendFrame((const uint8_t *)"F5", 2);
    protocolState = STATE_WAIT_F5;
    lastActionTime = now;
    g_ackReceived = false;
    break;
  case STATE_WAIT_F5:
    if (g_ackReceived) {
      delay(50);
      protocolState = STATE_INIT_D8;
    }
    break;

  case STATE_INIT_D8:
    sendFrame((const uint8_t *)"D80000", 6);
    protocolState = STATE_WAIT_D8;
    lastActionTime = now;
    g_ackReceived = false;
    break;
  case STATE_WAIT_D8:
    if (g_ackReceived || g_nakReceived) {
      // Faikout says D80000 might get NAK but we proceed?
      Serial.println("D80000 done (ACK/NAK). Moving to RH.");
      delay(50);
      protocolState = STATE_INIT_RH;
    }
    break;

  case STATE_INIT_RH:
    sendFrame((const uint8_t *)"RH", 2);
    protocolState = STATE_WAIT_RH;
    lastActionTime = now;
    g_ackReceived = false;
    break;
  case STATE_WAIT_RH:
    if (g_ackReceived) {
      delay(50);
      protocolState = STATE_INIT_RA;
    }
    break;

  case STATE_INIT_RA:
    sendFrame((const uint8_t *)"Ra", 2);
    protocolState = STATE_WAIT_RA;
    lastActionTime = now;
    g_ackReceived = false;
    break;
  case STATE_WAIT_RA:
    if (g_ackReceived) {
      Serial.println("Init Sequence Complete! Entering Idle Loop.");
      protocolState = STATE_IDLE;
    }
    break;

  case STATE_IDLE:
    // In IDLE, we no longer auto-poll. Just wait for explicit pollNow() calls.
    break;
  }
}

// On-demand polling: sends key queries and waits for responses
void S21Driver::pollNow() {
  const char *cmds[] = {"Ra", "RH", "F1"};
  const int numCmds = 3;

  for (int i = 0; i < numCmds; i++) {
    g_ackReceived = false;
    sendFrame((const uint8_t *)cmds[i], strlen(cmds[i]));

    // Wait for response (up to 500ms per command)
    unsigned long start = millis();
    while (!g_ackReceived && (millis() - start) < 500) {
      while (Serial1.available()) {
        processByte(Serial1.read());
      }
      delay(10);
    }
    delay(50); // Small gap between commands
  }
}

void S21Driver::write(const uint8_t *data, size_t len) {
  if (len == 0)
    return;
  Serial.print("TX: 0x");
  for (size_t i = 0; i < len; i++) {
    if (data[i] < 0x10)
      Serial.print("0");
    Serial.print(data[i], HEX);
  }
  Serial.println();
  Serial1.write(data, len);
}

void S21Driver::sendFrame(const uint8_t *payload, size_t len) {
  uint8_t frame[64];
  size_t idx = 0;

  frame[idx++] = 0x02; // STX

  uint8_t checksum = 0;
  for (size_t i = 0; i < len; i++) {
    frame[idx++] = payload[i];
    checksum += payload[i];
  }

  // Faikout Checksum Rule: If 0x03, replace with 0x05
  if (checksum == 0x03) {
    // Serial.println("Checksum corrected (0x03 -> 0x05)");
    checksum = 0x05;
  }

  frame[idx++] = checksum;
  frame[idx++] = 0x03; // ETX

  write(frame, idx);
}

uint8_t S21Driver::calculateChecksum(const uint8_t *data, size_t len) {
  // Unused internally now, but needed for future verification
  return 0;
}

// Internal method to handle received byte
void S21Driver::processByte(uint8_t byte) {
  // 1. Store byte in buffer
  if (rxIndex < sizeof(rxBuffer)) {
    rxBuffer[rxIndex++] = byte;
  } else {
    // Overflow protection: Reset if buffer full without packet end
    rxIndex = 0;
  }

  // 2. Check for End of Packet
  if (byte == 0x03) { // ETX
    // Print Raw Packet for debugging
    Serial.print("RX Frame: ");
    for (int i = 0; i < rxIndex; i++) {
      if (rxBuffer[i] < 0x10)
        Serial.print("0");
      Serial.print(rxBuffer[i], HEX);
    }

    // Identification
    if (rxIndex > 1) {
      if (rxBuffer[rxIndex - 2] == 0x06)
        Serial.print(" (ACK included)");
      if (rxBuffer[0] == 0x06) {
        Serial.print(" (ACK Start)");
        g_ackReceived = true;
        lastSuccessTime = millis(); // Valid, connected
      } // Some ACKs are separate?
    }
    Serial.println();

    // 3. Decode
    // If packet starts with ACK (06), actual STX might be at index 1
    int frameStart = 0;
    if (rxBuffer[0] == 0x06 && rxIndex > 1 && rxBuffer[1] == 0x02) {
      frameStart = 1;
    } else if (rxBuffer[0] == 0x02) {
      frameStart = 0;
    }

    State.decodeFrame(&rxBuffer[frameStart], rxIndex - frameStart);
    lastSuccessTime = millis(); // Valid frame received

    // 4. Reset Buffer
    rxIndex = 0;
    return;
  }

  // Also handle standalone ACK/NAK bytes as single-byte messages if they happen
  // outside full frames? User log showed "RX: 0x06 (ACK)" which meant single
  // byte? Yes, Faikout driver usually sees ACK as single byte. But my current
  // code prints every byte. If I BUFFER everything relative to STX, standalone
  // ACKs might be lost if I don't handle them. Logic update:
  if (byte == 0x06 && rxIndex == 1) { // Single byte ACK
    Serial.println("RX: ACK (Single Byte)");
    g_ackReceived = true;
    lastSuccessTime = millis();
    rxIndex = 0; // Reset
  } else if (byte == 0x15 && rxIndex == 1) {
    Serial.println("RX: NAK");
    g_nakReceived = true;
    rxIndex = 0;
  }
}

bool S21Driver::isConnected() {
  // If no valid packet/ACK in last 10 seconds, consider disconnected
  return (millis() - lastSuccessTime) < 10000;
}
