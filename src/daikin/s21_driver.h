#ifndef S21_DRIVER_H
#define S21_DRIVER_H

#include <Arduino.h>

class S21Driver {
public:
  // Initialize the driver (pins, serial port)
  void begin();

  // Main loop function to process incoming data
  // Should be called frequently
  void loop();

  // Write raw data to S21 bus
  void write(const uint8_t *data, size_t len);

  // Send a basic status poll command (Command 'F')
  void pollState();

  // Poll status on-demand (blocking, refresh State)
  void pollNow();

  // Helper to construct and send valid S21 Frames (Public for control)
  void sendFrame(const uint8_t *payload, size_t len);

  // Check if we have received valid data recently (timeout 10s)
  bool isConnected();

private:
  // Internal method to handle received byte
  void processByte(uint8_t byte);

  // Calculate Checksum (Mod 256 of sum of bytes)
  uint8_t calculateChecksum(const uint8_t *data, size_t len);

private:
  // Protocol State Machine
  int protocolState = 0;
  unsigned long lastActionTime = 0;
  unsigned long lastSuccessTime = 0; // Timestamp of last valid packet

  // RX Buffer
  uint8_t rxBuffer[64];
  int rxIndex = 0;
};

// Global instance declaration if needed, or just use singleton pattern
extern S21Driver S21;

#endif // S21_DRIVER_H
