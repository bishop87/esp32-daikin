# E — Execution

## Tooling
- Arduino IDE
- ESP32 Arduino Core
- ArduinoJson
- ESPAsyncWebServer (se compatibile)
- WiFiManager o equivalente
- SPIFFS o LittleFS

---

## Coding Rules

- Vietato l’uso di delay()
- Tutte le operazioni devono essere non bloccanti
- Codice modulare e separato per responsabilità
- Header e implementation separati (.h / .cpp)

---

## Module Responsibilities

### s21_driver
- Gestione fisica linea S21
- Timing e framing
- Half-duplex
- Nessuna logica di business

### daikin_state
- Stato completo del climatizzatore
- Sincronizzazione con frame S21
- Accesso thread-safe se necessario

### api_server
- REST API
- Validazione input
- Mapping API → stato

### wifi_manager
- Provisioning primo avvio
- Riconnessione automatica
- Captive portal

### webui
- Hosting file statici
- Mapping URL → SPIFFS
- Nessuna logica

---

## REST API Requirements

- JSON come unico formato
- Endpoint separati per:
  - stato
  - controllo
  - configurazione
- API idempotenti quando possibile

---

## Web UI Requirements

- Mobile-first
- Responsive
- HTML / CSS / JS puri
- Compatibile smartphone verticale e desktop
- Tutte le azioni passano dalle API REST

---

## Hardware Interface Rules

- Linea DATA S21 trattata come open-drain
- Uso di transistor / MOSFET / optoisolatore
- Un solo GPIO ESP32 per TX/RX
- Nessun pull-up interno ESP32 attivo

---

## Output Artifacts

- Firmware ESP32 compilabile
- File Web UI in /data
- Documentazione API
- Note hardware S21
