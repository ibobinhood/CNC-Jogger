#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoOTA.h>

const char* ssid = "Kulup Evi Master 2.4"; 
const char* password = "Hiz_No95";

// Sabit IP
IPAddress local_IP(192, 168, 0, 66);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);

WebServer server(80);
const int LED_PIN = 15; 

// Pinler
const int JOY_X = 1;
const int JOY_Y = 2;
const int BTNS[] = {3, 4, 5, 6, 7, 8}; 
const int HOME_BTN = 9; // --- BENİM HATAM DÜZELTİLDİ: 10 değil 9! ---
const int BAT_PIN = 12; 

// --- YENİ NESİL KESİNTİSİZ HTML ARAYÜZÜ ---
const char* htmlArayuz = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Bobinhood Dashboard</title>
  <style>
    body { background: #0a0a0a; color: #0f0; font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; text-align: center; margin: 0; padding: 20px; }
    h2 { letter-spacing: 2px; text-shadow: 0 0 10px #0f0; }
    .panel { display: flex; flex-wrap: wrap; justify-content: center; gap: 40px; margin-top: 30px; }
    .joy-bg { width: 160px; height: 160px; border: 3px solid #333; border-radius: 50%; position: relative; background: radial-gradient(circle, #222 0%, #111 100%); margin: auto; box-shadow: inset 0 0 20px #000; }
    .joy-crosshair { position: absolute; top: 50%; left: 0; width: 100%; height: 1px; background: rgba(0,255,0,0.2); }
    .joy-crosshair.v { top: 0; left: 50%; width: 1px; height: 100%; }
    .joy-stick { width: 40px; height: 40px; background: #0f0; border-radius: 50%; position: absolute; top: 60px; left: 60px; box-shadow: 0 0 15px #0f0, inset 0 0 10px #fff; transition: transform 0.05s linear; }
    .grid { display: grid; grid-template-columns: 1fr 1fr; gap: 15px; }
    .btn { padding: 20px 30px; background: #1a1a1a; border: 2px solid #333; color: #aaa; border-radius: 10px; font-weight: bold; font-size: 1.2rem; transition: 0.1s; text-shadow: 0 0 5px #000; }
    .btn.active { background: #0f0; color: #000; border-color: #0f0; box-shadow: 0 0 20px #0f0, inset 0 0 10px #fff; transform: scale(0.95); text-shadow: none; }
    #b-home { grid-column: span 2; border-color: #0aa; color: #0aa; }
    #b-home.active { background: #0aa; color: #000; border-color: #0aa; box-shadow: 0 0 20px #0aa, inset 0 0 10px #fff; text-shadow: none; }
    #term { margin-top: 40px; background: #000; padding: 15px; height: 100px; overflow-y: auto; border: 1px solid #333; text-align: left; font-family: monospace; color: #aaa; border-radius: 5px;}
    .battery-hud { position: absolute; top: 20px; right: 20px; font-size: 1.2rem; font-weight: bold; color: #0ff; text-shadow: 0 0 8px #0ff; border: 1px solid #0ff; padding: 5px 15px; border-radius: 5px; background: #001a1a; }
  </style>
</head>
<body>
  <div class="battery-hud">BAT: <span id="bat-val">--%</span></div>
  <h2>>> BOBINHOOD CNC DASHBOARD</h2>
  <div class="panel">
    <div class="module">
      <h3 style="color:#555;">JOYSTICK</h3>
      <div class="joy-bg">
        <div class="joy-crosshair"></div><div class="joy-crosshair v"></div>
        <div class="joy-stick" id="stick"></div>
      </div>
    </div>
    <div class="module">
      <h3 style="color:#555;">EKSEN KONTROL</h3>
      <div class="grid">
        <div class="btn" id="b0">X+</div><div class="btn" id="b1">X-</div>
        <div class="btn" id="b2">Y+</div><div class="btn" id="b3">Y-</div>
        <div class="btn" id="b4">Z+</div><div class="btn" id="b5">Z-</div>
        <div class="btn" id="b-home">HOME</div>
      </div>
    </div>
  </div>
  <div id="term">Bobinhood Ağına Bağlanıldı. Veri bekleniyor...<br></div>

<script>
  function veriCek() {
    fetch('/state')
      .then(r => r.json())
      .then(d => {
        const maxRadius = 60; 
        let rawX = (d.x / 8191) * 120 - 60;
        let rawY = (d.y / 8191) * 120 - 60;
        
        if (Math.abs(rawX) < 5) rawX = 0;
        if (Math.abs(rawY) < 5) rawY = 0;
        
        let distance = Math.sqrt(rawX * rawX + rawY * rawY);
        let finalX = rawX;
        let finalY = rawY;
        
        if (distance > maxRadius) {
          finalX = (rawX / distance) * maxRadius;
          finalY = (rawY / distance) * maxRadius;
        }
        
        document.getElementById('stick').style.transform = `translate(${finalX}px, ${finalY}px)`;
        
        let anyBtn = false;
        let btnNames = ["X+", "X-", "Y+", "Y-", "Z+", "Z-"];
        let logStr = "";
        
        for(let i=0; i<6; i++){
          let btn = document.getElementById('b'+i);
          if(d.b[i] == 1) {
            btn.classList.add('active');
            logStr += btnNames[i] + " ";
            anyBtn = true;
          } else {
            btn.classList.remove('active');
          }
        }

        let btnHome = document.getElementById('b-home');
        if(d.h == 1) {
          btnHome.classList.add('active');
          logStr += "[HOME] ";
          anyBtn = true;
        } else {
          btnHome.classList.remove('active');
        }

        document.getElementById('bat-val').innerText = d.v + "%";
        
        if(anyBtn || distance > 20) { 
           let term = document.getElementById('term');
           term.innerHTML += `> Joy:[${Math.round(rawX)}, ${Math.round(rawY)}] | Buton:[${anyBtn ? logStr : 'Yok'}]<br>`;
           term.scrollTop = term.scrollHeight; 
        }

        setTimeout(veriCek, 80); 
      })
      .catch(e => {
        console.log("Lag girdi, toparlanıyor...");
        setTimeout(veriCek, 500); 
      });
  }
  veriCek(); 
</script>
</body>
</html>
)rawliteral";


void setup() {
  pinMode(LED_PIN, OUTPUT);
  for (int i = 0; i < 6; i++) { pinMode(BTNS[i], INPUT_PULLUP); }
  
  pinMode(HOME_BTN, INPUT_PULLUP);

  WiFi.setSleep(false); 
  
  WiFi.setTxPower(WIFI_POWER_8_5dBm);
  WiFi.config(local_IP, gateway, subnet);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(LED_PIN, !digitalRead(LED_PIN)); delay(200); 
  }
  digitalWrite(LED_PIN, HIGH);

  server.on("/", []() { 
    server.send(200, "text/html", htmlArayuz); 
  });

  server.on("/state", []() {
    // --- YENİ: Batarya Okuma Filtresi (Ortalama Alma) ---
    long batToplam = 0;
    for(int j = 0; j < 20; j++) {
      batToplam += analogRead(BAT_PIN);
    }
    float ortalamaADC = batToplam / 20.0;
    
    // Filtrelenmiş değeri voltaja çevir
    float batVoltaj = (ortalamaADC / 8191.0) * 3.3 * 2.0; 
    
    int batYuzde = (batVoltaj - 3.2) / (4.2 - 3.2) * 100;
    if(batYuzde > 100) batYuzde = 100;
    if(batYuzde < 0) batYuzde = 0;

    String json = "{";
    json += "\"x\":" + String(analogRead(JOY_X)) + ",";
    json += "\"y\":" + String(analogRead(JOY_Y)) + ",";
    json += "\"h\":" + String(digitalRead(HOME_BTN) == LOW ? 1 : 0) + ",";
    json += "\"v\":" + String(batYuzde) + ","; 
    json += "\"b\":[";
    for(int i=0; i<6; i++) {
      json += String(digitalRead(BTNS[i]) == LOW ? 1 : 0);
      if(i < 5) json += ",";
    }
    json += "]}";
    
    server.send(200, "application/json", json);
  });

  server.begin();
  
  ArduinoOTA.setHostname("Bobinhood-Remote");
  ArduinoOTA.setPassword("bobinhood26"); 
  ArduinoOTA.begin();
}

void loop() {
  ArduinoOTA.handle();
  server.handleClient();
}