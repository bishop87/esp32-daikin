#ifndef WEB_UI_H
#define WEB_UI_H

#include <Arduino.h>

const char WEB_UI_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Daikin Control</title>
  <style>
    * { margin: 0; padding: 0; box-sizing: border-box; }
    body {
      font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', sans-serif;
      background: linear-gradient(135deg, #1a1a2e 0%, #16213e 100%);
      min-height: 100vh;
      color: #fff;
      padding: 20px;
    }
    .container {
      max-width: 400px;
      margin: 0 auto;
    }
    .card {
      background: rgba(255,255,255,0.1);
      backdrop-filter: blur(10px);
      border-radius: 20px;
      padding: 15px 20px;
      margin-bottom: 15px;
      border: 1px solid rgba(255,255,255,0.1);
    }
    h1 {
      text-align: center;
      font-size: 24px;
      margin-bottom: 20px;
      background: linear-gradient(90deg, #00d4ff, #7b2cbf);
      -webkit-background-clip: text;
      -webkit-text-fill-color: transparent;
    }
    .temp-display {
      text-align: center;
      font-size: 60px;
      font-weight: 200;
      margin: 10px 0;
    }
    .temp-display span { font-size: 32px; }
    .info-row {
      display: flex;
      justify-content: space-between;
      padding: 10px 0;
      border-bottom: 1px solid rgba(255,255,255,0.1);
    }
    .info-label { opacity: 0.7; }
    .power-btn {
      width: 100%;
      padding: 15px;
      font-size: 18px;
      border: none;
      border-radius: 15px;
      cursor: pointer;
      transition: all 0.3s;
      font-weight: 600;
    }
    .power-on {
      background: linear-gradient(135deg, #00d4ff, #0099cc);
      color: #fff;
    }
    .power-off {
      background: rgba(255,255,255,0.1);
      color: #fff;
    }
    .mode-grid {
      display: grid;
      grid-template-columns: repeat(5, 1fr);
      gap: 8px;
      margin: 15px 0;
    }
    .mode-btn {
      padding: 12px 8px;
      border: none;
      border-radius: 10px;
      background: rgba(255,255,255,0.1);
      color: #fff;
      cursor: pointer;
      font-size: 12px;
      transition: all 0.3s;
    }
    .mode-btn.active {
      background: linear-gradient(135deg, #00d4ff, #0099cc);
    }
    .slider-container { margin: 20px 0; }
    .slider-label {
      display: flex;
      justify-content: space-between;
      margin-bottom: 10px;
      opacity: 0.7;
    }
    input[type="range"] {
      width: 100%;
      height: 8px;
      border-radius: 4px;
      background: rgba(255,255,255,0.2);
      -webkit-appearance: none;
    }
    input[type="range"]::-webkit-slider-thumb {
      -webkit-appearance: none;
      width: 24px;
      height: 24px;
      border-radius: 50%;
      background: #00d4ff;
      cursor: pointer;
    }
    .fan-grid {
      display: grid;
      grid-template-columns: repeat(6, 1fr);
      gap: 6px;
    }
    .fan-btn {
      padding: 10px 5px;
      border: none;
      border-radius: 8px;
      background: rgba(255,255,255,0.1);
      color: #fff;
      cursor: pointer;
      font-size: 11px;
      transition: all 0.3s;
    }
    .fan-btn.active {
      background: linear-gradient(135deg, #7b2cbf, #5a189a);
    }
    .send-btn {
      width: 100%;
      padding: 18px;
      font-size: 20px;
      border: none;
      border-radius: 15px;
      cursor: pointer;
      transition: all 0.3s;
      font-weight: 700;
      background: linear-gradient(135deg, #10b981, #059669);
      color: #fff;
      margin-top: 20px;
    }
    .send-btn:active {
      transform: scale(0.98);
    }
    .status { text-align: center; opacity: 0.5; font-size: 12px; margin-top: 20px; }
  </style>
</head>
<body>
  <div class="container">
    <h1>🌡️ Daikin Control</h1>
    
    <div class="card">
      <div class="temp-display"><span id="roomTemp">--</span><span>°C</span></div>
      <div class="info-row">
        <span class="info-label">Outside</span>
        <span id="outsideTemp">--°C</span>
      </div>
      <div class="info-row">
        <span class="info-label">Target</span>
        <span id="targetTemp">--°C</span>
      </div>
      <div class="info-row" style="border-bottom: none; padding-top: 15px;">
        <span class="info-label">Auto Refresh</span>
        <button id="refreshBtn" onclick="toggleAutoRefresh()" style="padding: 8px 16px; border: none; border-radius: 8px; cursor: pointer; font-size: 12px; transition: all 0.3s; background: linear-gradient(135deg, #10b981, #059669); color: #fff;">ON</button>
      </div>
    </div>

    <div class="card">
      <button id="powerBtn" class="power-btn power-off" onclick="togglePower()">POWER OFF</button>
      
      <div class="mode-grid">
        <button class="mode-btn" data-mode="1" onclick="selectMode(1)">Auto</button>
        <button class="mode-btn" data-mode="3" onclick="selectMode(3)">Cool</button>
        <button class="mode-btn" data-mode="4" onclick="selectMode(4)">Heat</button>
        <button class="mode-btn" data-mode="2" onclick="selectMode(2)">Dry</button>
        <button class="mode-btn" data-mode="6" onclick="selectMode(6)">Fan</button>
      </div>

      <div class="slider-container">
        <div class="slider-label">
          <span>Temperature</span>
          <span id="tempValue">22°C</span>
        </div>
        <input type="range" id="tempSlider" min="16" max="30" value="22" oninput="selectTemp(this.value)">
      </div>

      <div class="slider-container">
        <div class="slider-label"><span>Fan Speed</span></div>
        <div class="fan-grid">
          <button class="fan-btn" data-fan="1" onclick="selectFan(1)">1</button>
          <button class="fan-btn" data-fan="2" onclick="selectFan(2)">2</button>
          <button class="fan-btn" data-fan="3" onclick="selectFan(3)">3</button>
          <button class="fan-btn" data-fan="4" onclick="selectFan(4)">4</button>
          <button class="fan-btn" data-fan="5" onclick="selectFan(5)">5</button>
          <button class="fan-btn" data-fan="10" onclick="selectFan(10)">A</button>
        </div>
      </div>

      <button class="send-btn" onclick="sendConfig()">📤 INVIA</button>
    </div>

    <div class="card">
      <div class="info-row" style="display:block; border:none; padding-bottom:5px">
        <div style="margin-bottom:10px; opacity:0.7">Aggiornamento Firmware</div>
      </div>
      
      <div class="info-row" style="display:block; border:none; padding:10px 0">
        <div style="font-size:12px; margin-bottom:5px; opacity:0.5">File Locale (.bin)</div>
        <input type="file" id="fwFile" accept=".bin" style="color:#fff; width:100%; font-size:12px">
        <button onclick="uploadFirmware()" style="margin-top:8px; padding:10px; width:100%; border-radius:10px; border:none; background:rgba(255,255,255,0.1); color:#fff; cursor:pointer">Carica da File</button>
      </div>

      <div class="info-row" style="display:block; border:none; padding-top:10px">
        <div style="font-size:12px; margin-bottom:5px; opacity:0.5">URL Remoto</div>
        <input type="text" id="fwUrl" placeholder="http://..." style="width:100%; padding:8px; border-radius:5px; border:none; background:rgba(255,255,255,0.2); color:#fff;">
        <button onclick="updateFromUrl()" style="margin-top:8px; padding:10px; width:100%; border-radius:10px; border:none; background:linear-gradient(135deg, #3b82f6, #2563eb); color:#fff; cursor:pointer">Aggiorna da URL</button>
      </div>
      
      <div id="otaStatus" style="text-align:center; font-size:12px; margin-top:10px; min-height:15px; color:#fbbf24"></div>
    </div>

    <div class="status" id="status">Connecting...</div>
  </div>

  <script>
    // Server state (from AC)
    let serverState = { power: false, mode: 3, target_temp: 22, fan: 5, room_temp: 0, outside_temp: 0, connected: false };
    // Local pending state (what user has selected)
    let localState = { power: false, mode: 3, target_temp: 22, fan: 5 };

    async function fetchStatus(syncLocal = true) {
      try {
        const res = await fetch('/status');
        serverState = await res.json();
        // Only sync local state to server state if requested (not after send)
        if (syncLocal) {
          localState = {
            power: serverState.power,
            mode: serverState.mode,
            target_temp: Math.round(serverState.target_temp),
            fan: serverState.fan
          };
        }
        updateUI();
        
        const statusEl = document.getElementById('status');
        if (serverState.connected) {
             statusEl.textContent = 'Connected';
             statusEl.style.color = '#10b981'; // Green
             statusEl.style.opacity = '1';
        } else {
             statusEl.textContent = 'Disconnected (Timeout)';
             statusEl.style.color = '#ef4444'; // Red
             statusEl.style.opacity = '1';
        }

      } catch (e) {
        const statusEl = document.getElementById('status');
        statusEl.textContent = 'Connection error';
        statusEl.style.color = '#ef4444'; 
      }
    }

    function updateUI() {
      // Display server temps
      document.getElementById('roomTemp').textContent = serverState.room_temp?.toFixed(1) || '--';
      document.getElementById('outsideTemp').textContent = (serverState.outside_temp?.toFixed(1) || '--') + '°C';
      document.getElementById('targetTemp').textContent = (serverState.target_temp?.toFixed(1) || '--') + '°C';
      
      // Power button reflects LOCAL state
      const btn = document.getElementById('powerBtn');
      btn.textContent = localState.power ? 'POWER ON' : 'POWER OFF';
      btn.className = 'power-btn ' + (localState.power ? 'power-on' : 'power-off');

      // Mode buttons reflect LOCAL state
      document.querySelectorAll('.mode-btn').forEach(b => {
        b.classList.toggle('active', parseInt(b.dataset.mode) === localState.mode);
      });

      // Fan buttons reflect LOCAL state
      document.querySelectorAll('.fan-btn').forEach(b => {
        b.classList.toggle('active', parseInt(b.dataset.fan) === localState.fan);
      });

      // Temp slider reflects LOCAL state
      document.getElementById('tempSlider').value = localState.target_temp;
      document.getElementById('tempValue').textContent = localState.target_temp + '°C';
    }

    function togglePower() {
      localState.power = !localState.power;
      updateUI();
    }

    function selectMode(m) {
      localState.mode = m;
      updateUI();
    }

    function selectTemp(t) {
      localState.target_temp = parseInt(t);
      document.getElementById('tempValue').textContent = t + '°C';
    }

    function selectFan(f) {
      localState.fan = f;
      updateUI();
    }

    async function sendConfig() {
      document.getElementById('status').textContent = 'Sending...';
      try {
        const params = {
          power: localState.power ? '1' : '0',
          temp: localState.target_temp,
          mode: localState.mode,
          fan: localState.fan
        };
        await fetch('/set?' + new URLSearchParams(params));
        // Don't sync local state after send - trust what user set (optimistic UI)
        document.getElementById('status').textContent = 'Sent!';
        setTimeout(() => { document.getElementById('status').textContent = 'Connected'; }, 2000);
      } catch (e) {
        document.getElementById('status').textContent = 'Error sending command';
      }
    }

    fetchStatus();
    let autoRefreshEnabled = false;
    let refreshInterval = null;

    // Update button to show OFF state initially
    document.getElementById('refreshBtn').textContent = 'OFF';
    document.getElementById('refreshBtn').style.background = 'rgba(255,255,255,0.1)';

    function toggleAutoRefresh() {
      autoRefreshEnabled = !autoRefreshEnabled;
      const btn = document.getElementById('refreshBtn');
      if (autoRefreshEnabled) {
        btn.textContent = 'ON';
        btn.style.background = 'linear-gradient(135deg, #10b981, #059669)';
        refreshInterval = setInterval(fetchStatus, 30000);
      } else {
        btn.textContent = 'OFF';
        btn.style.background = 'rgba(255,255,255,0.1)';
        clearInterval(refreshInterval);
      }
    }

    async function uploadFirmware() {
      const fileInput = document.getElementById('fwFile');
      const file = fileInput.files[0];
      if (!file) { alert('Seleziona un file .bin!'); return; }
      
      const status = document.getElementById('otaStatus');
      status.textContent = 'Caricamento in corso... NON SPEGNERE!';
      status.style.color = '#fbbf24';
      
      const formData = new FormData();
      formData.append('update', file);
      
      try {
        const res = await fetch('/update', { method: 'POST', body: formData });
        if (res.ok) {
            status.textContent = 'Completato! Riavvio in corso...';
            status.style.color = '#10b981';
            setTimeout(() => location.reload(), 15000);
        } else {
            status.textContent = 'Errore Caricamento';
            status.style.color = '#ef4444';
        }
      } catch (e) {
        status.textContent = 'Errore: ' + e.message;
        status.style.color = '#ef4444';
      }
    }

    async function updateFromUrl() {
      const url = document.getElementById('fwUrl').value;
      if (!url) { alert('Inserisci un URL valido!'); return; }
      
      const status = document.getElementById('otaStatus');
      status.textContent = 'Download in corso... NON SPEGNERE!';
      status.style.color = '#fbbf24';
      
      try {
        const res = await fetch('/update-url', { 
            method: 'POST', 
            headers: {'Content-Type': 'application/x-www-form-urlencoded'},
            body: 'url=' + encodeURIComponent(url)
        });
        
        if (res.ok) {
            status.textContent = 'Aggiornamento avviato! Riavvio se OK...';
            status.style.color = '#10b981';
            setTimeout(() => location.reload(), 20000);
        } else {
            status.textContent = 'Errore: ' + await res.text();
            status.style.color = '#ef4444';
        }
      } catch (e) {
        status.textContent = 'Errore: ' + e.message;
        status.style.color = '#ef4444';
      }
    }
  </script>
</body>
</html>
)rawliteral";

#endif
