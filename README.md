# ESP32 Daikin S21 Controller (ESP32-C3 Super Mini)

This project allows you to control Daikin air conditioners (split units) via the S21 interface using an ESP32-C3 Super Mini board. It provides a modern Web UI, a JSON API for integration (e.g., Home Assistant), and supports Over-The-Air (OTA) firmware updates.

## Features

- **Direct Hardware Control**: Communicates directly with the AC unit via the S21 protocol (UART).
- **Web Interface**: responsive UI to control power, mode, temperature, and fan speed.
- **Real-time Status**: Displays internal/external temperature and connection status.
- **REST API**: Simple JSON API for easy integration with third-party systems.
- **OTA Updates**: Update firmware via Web UI (local file upload or remote URL).
- **Status LED**: Visual feedback for WiFi connection status.

## Hardware Requirements

- **ESP32-C3 Super Mini** board (or compatible).
- **S21 Adapter Cable**: Breaks out the 4-pin S21 connector (VCC, GND, TX, RX) from the indoor unit PCB.
    - **Note**: The S21 port provides 5V, but logic levels are 5V-tolerant. The ESP32-C3 signals are 3.3V. A level shifter is recommended but often works directly with 5V tolerant inputs. *Proceed at your own risk.*

### Wiring

| ESP32 Pin | Daikin S21 Pin | Description |
|-----------|----------------|-------------|
| 5V        | 5V             | Power Input |
| GPIO 4(TX)| RX             | ESP TX -> AC RX |
| GPIO 3(RX)| TX             | ESP RX <- AC TX |
| n.c.      | +14V           | not connected |
| GND       | GND            | Ground      |



> **Note**: Daikin `TX` goes to ESP `RX` (GPIO 3), and Daikin `RX` goes to ESP `TX` (GPIO 4).

## Configuration

1.  Copy `src/system/config.h.example` to `src/system/config.h`.
2.  Edit `src/system/config.h` with your WiFi credentials:
    ```c
    #define WIFI_SSID "YOUR_WIFI_SSID"
    #define WIFI_PASS "YOUR_WIFI_PASSWORD"
    ```
3.  (Optional) Calibrate the outside temperature offset if needed.

## Installation

1.  Open the project in Arduino IDE or PlatformIO.
2.  Select board: **ESP32C3 Dev Module**.
3.  Enable **USB CDC On Boot** if debugging via USB serial is required.
4.  Compile and Upload.

## Usage

### Web Interface
Navigate to `http://<ESP_IP_ADDRESS>/` in your browser.
- **Power**: Toggle AC On/Off.
- **Mode**: Auto, Cool, Heat, Dry, Fan.
- **Temp**: Set target temperature.
- **Fan**: Set fan speed (1-5, Auto).
- **Firmware Update**: Upload `.bin` files or update from a URL.

### Status LED (GPIO 8)
- **Blinking**: Connecting to WiFi.
- **OFF**: Connected successfully (Normal operation).
- **ON (Solid)**: WiFi connection failed or error.

### API Reference

#### Get Status
**Endpoint**: `GET /status`

Returns the current state of the AC unit.

**Response**:
```json
{
  "power": true,
  "mode": 3,
  "target_temp": 24.0,
  "room_temp": 25.5,
  "outside_temp": 30.0,
  "fan": 5,
  "connected": true
}
```
- `mode`: 1 (Auto), 2 (Dry), 3 (Cool), 4 (Heat), 6 (Fan)
- `connected`: `true` if S21 packets are being received (last 10s), `false` if disconnected/timeout.

#### Set State
**Endpoint**: `GET /set`

Controls the AC unit. Parameters can be combined.

**Parameters**:
- `power`: `on`, `true`, `1` (or `off`, `false`, `0`)
- `mode`: `1` (Auto), `2` (Dry), `3` (Cool), `4` (Heat), `6` (Fan)
- `temp`: Target temperature (e.g., `24`, `24.5`)
- `fan`: Fan speed `1`-`5`, or `10` (Auto)

**Examples**:
- Turn ON Cool Mode at 24°C:
  `http://<IP>/set?power=on&mode=3&temp=24`
- Set Fan to Auto:
  `http://<IP>/set?fan=10`
- Turn OFF:
  `http://<IP>/set?power=off`

#### OTA Firmware Update (API)
- **POST /update**: Multipart form upload with field name `update` containing the `.bin` file.
- **POST /update-url**: JSON or Form data with `url` field pointing to the `.bin` file location.

---
**Disclaimer**: This software is not affiliated with Daikin. Use at your own risk. Connecting unverified hardware to your AC unit may void your warranty or cause damage.
