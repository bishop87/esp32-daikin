#if defined(CONFIG_IDF_TARGET_ESP32C3)
#include "esp_private/esp_clk.h"
#include "soc/rtc_cntl_reg.h"
#else
#include "soc/rtc_cntl_reg.h"
#include "soc/soc.h"
#endif
#include "src/daikin/daikin_state.h"
#include "src/daikin/s21_driver.h"
#include "src/system/config.h"
#include "src/system/logger.h"
#include "src/web/web_ui.h"
#include <HTTPUpdate.h>
#include <Preferences.h>
#include <Update.h>
#include <WebServer.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

Preferences preferences;
String splitName = "NomeSplit";

WebServer server(API_PORT);

void handleRoot() { server.send(200, "text/html", WEB_UI_HTML); }

void handleStatus() {
  // Poll fresh data from AC
  S21.pollNow();

  String json = "{";
  json += "\"power\":" + String(State.power ? "true" : "false") + ",";
  json += "\"mode\":" + String(State.mode) + ",";
  json += "\"target_temp\":" + String(State.targetTemp) + ",";
  json += "\"room_temp\":" + String(State.roomTemp) + ",";
  json += "\"outside_temp\":" + String(State.outsideTemp) + ",";
  json += "\"fan\":" + String(State.fan) + ",";
  json += "\"connected\":" + String(S21.isConnected() ? "true" : "false") + ",";
  json += "\"split_name\":\"" + splitName + "\"";
  json += "}";
  server.send(200, "application/json", json);
}

void handleSetConfig() {
  if (server.hasArg("name")) {
    String newName = server.arg("name");
    newName.trim();
    if (newName.length() > 0) {
      splitName = newName;
      preferences.begin("daikin", false);
      preferences.putString("split_name", splitName);
      preferences.end();
      server.send(200, "application/json",
                  "{\"status\":\"ok\", \"name\":\"" + splitName + "\"}");
      LOG("Config: Split Name set to %s", splitName.c_str());
    } else {
      server.send(400, "text/plain", "Invalid name");
    }
  } else {
    server.send(400, "text/plain", "Missing 'name' parameter");
  }
}

void handleSet() {
  if (server.hasArg("temp")) {
    float temp = server.arg("temp").toFloat();
    // Default values if not provided
    uint8_t mode = State.mode > 0 ? State.mode : 3; // Default Cool
    uint8_t fan = State.fan > 0 ? State.fan : 5;    // Default 5
    bool power = true;

    if (server.hasArg("mode"))
      mode = server.arg("mode").toInt();
    if (server.hasArg("fan"))
      fan = server.arg("fan").toInt();
    if (server.hasArg("power")) {
      String p = server.arg("power");
      power = (p == "1" || p == "true" || p == "on");
    }

    State.setDaikinState(power, mode, temp, fan);
    server.send(200, "application/json", "{\"status\":\"ok\"}");
    LOG("API: Set Temp %.1f, Mode %d, Fan %d, Power %d", temp, mode, fan,
        power);
  } else {
    server.send(400, "text/plain", "Missing 'temp' parameter");
  }
}

// OTA Handlers
void handleUpdateUrl() {
  if (!server.hasArg("url")) {
    server.send(400, "text/plain", "Missing url");
    return;
  }
  String url = server.arg("url");
  LOG("OTA: Updating from URL: %s", url.c_str());

  WiFiClientSecure client;
  client.setInsecure(); // Allow any certificate

  // Disable auto-reboot to send response first
  httpUpdate.rebootOnUpdate(false);
  t_httpUpdate_return ret = httpUpdate.update(client, url);

  switch (ret) {
  case HTTP_UPDATE_FAILED:
    server.send(500, "text/plain", "Fail: " + httpUpdate.getLastErrorString());
    break;
  case HTTP_UPDATE_NO_UPDATES:
    server.send(304, "text/plain", "No updates");
    break;
  case HTTP_UPDATE_OK:
    server.send(200, "text/plain", "OK");
    delay(1000);
    ESP.restart();
    break;
  }
}

void handleUpdateUpload() {
  HTTPUpload &upload = server.upload();
  if (upload.status == UPLOAD_FILE_START) {
    LOG("OTA: Upload Start: %s", upload.filename.c_str());
    if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
      Update.printError(Serial);
    }
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
      Update.printError(Serial);
    }
  } else if (upload.status == UPLOAD_FILE_END) {
    if (Update.end(true)) {
      LOG("OTA: Success: %u bytes", upload.totalSize);
    } else {
      Update.printError(Serial);
    }
  }
}

void setup() {
  // Disable brownout detector (temporary fix for USB power issues)
#if defined(CONFIG_IDF_TARGET_ESP32C3)
  // For C3, the register is different or handled by HAL.
  // Disabling for compatibility with original intent.
#else
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
#endif

  // Initialize Preferences
  preferences.begin("daikin", false); // Namespace "daikin", read/write
  splitName = preferences.getString("split_name", "NomeSplit");
  preferences.end();
  LOG("Config: Split Name loaded: %s", splitName.c_str());

  // Initialize LED
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LED_OFF); // Start OFF

  // Initialize debug serial
  Serial.begin(DEBUG_BAUD_RATE);
  delay(1000); // Give some time for serial monitor to connect

  LOG("\n=== ESP32 Daikin S21 Active Polling ===");
  LOG("Mode: Master/Polling");
  LOG("RX: %d, TX: %d", S21_RX_PIN, S21_TX_PIN);

  // Initialize S21 driver
  S21.begin();

  // Short delay to stabilize power before WiFi
  delay(500);

  // WiFi Setup
  WiFi.mode(WIFI_STA); // Explicitly set mode to Station
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  LOG("Connecting to WiFi [%s]...", WIFI_SSID);
  // Non-blocking wait (sort of, we just log)
  int retries = 0;
  while (WiFi.status() != WL_CONNECTED &&
         retries < 40) { // Increased timeout to 20s

    // Blink LED
    digitalWrite(LED_PIN, (retries % 2 == 0) ? LED_ON : LED_OFF);

    delay(500);
    Serial.print(".");
    retries++;
  }
  if (WiFi.status() == WL_CONNECTED) {
    digitalWrite(LED_PIN, LED_OFF); // WiFi OK -> LED OFF
    LOG("\nWiFi Connected! IP: %s", WiFi.localIP().toString().c_str());

    // API Routes
    server.on("/", handleRoot);
    server.on("/status", handleStatus);
    server.on("/status", handleStatus);
    server.on("/status", handleStatus);
    server.on("/set", handleSet);
    server.on("/set-config", handleSetConfig);

    // OTA Routes
    server.on("/update-url", HTTP_POST, handleUpdateUrl);
    server.on(
        "/update", HTTP_POST,
        []() {
          server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
          if (!Update.hasError()) {
            delay(1000);
            ESP.restart();
          }
        },
        handleUpdateUpload);
    server.begin();
    LOG("HTTP Server Started on port %d", API_PORT);
  } else {
    digitalWrite(LED_PIN, LED_ON); // WiFi Fail -> LED ON (Solid)
    LOG("\nWiFi Connection Failed! Running in Serial-Only mode.");
  }
}

// Main driver loop
void loop() {
  S21.loop();
  server.handleClient();

  // Simple CLI (Keep it for debugging)
  if (Serial.available()) {
    String line = Serial.readStringUntil('\n');
    line.trim();

    if (line.length() > 0) {
      char cmd = line.charAt(0);
      float temp = 22.0;
      if (line.length() > 1)
        temp = line.substring(1).toFloat();

      if (cmd == 'C') {                         // Cool
        State.setDaikinState(true, 3, temp, 5); // Cool, Temp, Fan 5
        LOG("CMD: Set Cool %.1f", temp);
      } else if (cmd == 'H') {                  // Heat
        State.setDaikinState(true, 4, temp, 5); // Heat, Temp, Fan 5
        LOG("CMD: Set Heat %.1f", temp);
      } else if (cmd == 'D') { // Dry
        State.setDaikinState(true, 2, temp, 5);
        LOG("CMD: Set Dry %.1f", temp);
      } else if (cmd == 'A') { // Auto
        State.setDaikinState(true, 1, temp, 5);
        LOG("CMD: Set Auto %.1f", temp);
      } else if (cmd == 'F') { // Fan Mode
        State.setDaikinState(true, 6, 25.0, 5);
        LOG("CMD: Set Fan Only");
      } else if (cmd == 'O') { // OFF
        State.setDaikinState(false, 3, 25.0, 5);
        LOG("CMD: Set Power OFF");
      } else {
        LOG("Unknown Command. Use C24, H22, D24, A24, F, O");
      }
    }
  }

  // Minimal yield to watchdog
  delay(1);
}
