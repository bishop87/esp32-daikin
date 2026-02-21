# Project: ESP32 Daikin S21 Controller

## D — Directive

### Goal
Sviluppare un firmware per ESP32 WROOM DEV KIT, programmato tramite Arduino IDE, che permetta il controllo completo di un condizionatore Daikin modello FTXS35J2V1B tramite il connettore S21 presente sulla scheda elettronica dell’unità interna.

Il sistema deve replicare e superare le funzionalità del modulo WiFi ufficiale Daikin BRP069B43.

---

### Functional Requirements
- Comunicazione bidirezionale stabile tramite protocollo Daikin S21
- Esposizione di TUTTE le funzioni del climatizzatore tramite API REST
- Gestione WiFi con provisioning al primo avvio e riconnessione automatica
- Web UI integrata che utilizza ESCLUSIVAMENTE le API REST
- Web UI completamente responsive:
  - smartphone (schermi verticali)
  - desktop / laptop

---

### Reference Project
Il seguente repository è riferimento tecnico per protocollo e logica S21:

https://github.com/revk/ESP32-Faikout

Il codice deve essere:
- studiato
- reinterpretato
- adattato
NON copiato senza comprensione.

---

### Constraints
- MCU: ESP32 C3 super mini
- IDE: Arduino IDE
- Linguaggio: C++ (Arduino framework)
- Comunicazione: S21 half-duplex, open-drain
- Nessun uso del modulo WiFi Daikin originale
- Nessun servizio cloud esterno
- Non provare a compilare il codice, lo farò io con Arduino IDE

---

### Success Criteria
- Comunicazione S21 stabile e verificabile
- Stato del climatizzatore sempre sincronizzato
- Tutte le funzioni accessibili via REST
- Web UI funzionante su mobile e desktop
- Riconnessione WiFi automatica dopo reboot
- Firmware stabile in funzionamento continuo

---

### Non-Goals
- Integrazione Alexa / Google Home
- App mobile nativa
- Modifiche hardware alla scheda Daikin
- Alimentazione ESP32 dal connettore S21

---

### Project Structure (Mandatory)
L’agente DEVE generare il progetto rispettando ESATTAMENTE la struttura definita come

esp32-daikin/
│
├─ doe/
│   ├─ directive.md
│   ├─ orchestration.md
│   └─ execution.md
│
├─ esp32-daikin.ino
│
├─ src/
│   ├─ daikin/
│   │   ├─ s21_driver.h
│   │   ├─ s21_driver.cpp
│   │   ├─ daikin_state.h
│   │   └─ daikin_state.cpp
│   │
│   ├─ api/
│   │   ├─ api_server.h
│   │   └─ api_server.cpp
│   │
│   ├─ wifi/
│   │   ├─ wifi_manager.h
│   │   └─ wifi_manager.cpp
│   │
│   ├─ webui/
│   │   ├─ webui.h
│   │   └─ webui.cpp
│   │
│   └─ system/
│       ├─ config.h
│       └─ logger.h
│
├─ data/
│   ├─ index.html
│   ├─ style.css
│   └─ app.js
│
├─ hardware/
│   └─ s21_notes.md
│
└─ README.md

Ogni deviazione è considerata errore.
