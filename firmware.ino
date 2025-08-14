#include "ELECHOUSE_CC1101_SRC_DRV.h"
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

// ===== CONFIG Wi-Fi AP =====
IPAddress apIP(192, 168, 4, 1);
const char* ssid = "ESP32-Repeater";
const char* password = "123456789";

// ===== CONFIG CC1101 =====
#define RXPin0 4
#define TXPin0 5
float frequency = 433.92; // Ajuste conforme seu keyfob

// ===== BUFFER DE CAPTURA =====
#define MAX_PULSES 512
volatile uint16_t pulseBuffer[MAX_PULSES];
volatile uint16_t pulseCount = 0;
volatile unsigned long lastChange = 0;
volatile bool signalCaptured = false;

// ===== CONTROLE =====
bool repeaterEnabled = false;

// ===== Servidor Web =====
AsyncWebServer server(80);

// ===== HTML embutido =====
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="pt-BR">
<head>
<meta charset="UTF-8">
<title>Repetidor RF - CC1101</title>
<style>
body { font-family: Arial; background-color:#121212; color:white; text-align:center; padding:20px;}
h1 { color:#00ffcc;}
button { background:#00ffcc; border:none; padding:15px 25px; font-size:16px; margin:10px; border-radius:5px; cursor:pointer;}
button:hover { background:#00ccaa;}
#status { margin-top:20px; font-size:18px; font-weight:bold; color:#ffcc00;}
</style>
<script>
function sendCommand(cmd) {
    fetch('/cmd?act=' + cmd)
    .then(response => response.text())
    .then(data => {
        document.getElementById("status").innerText = "Status: " + data;
    })
    .catch(error => {
        document.getElementById("status").innerText = "Erro ao enviar comando!";
    });
}
</script>
</head>
<body>
<h1>Repetidor RF - CC1101</h1>
<button onclick="sendCommand('start')">Ativar Repetidor</button>
<button onclick="sendCommand('stop')">Desativar Repetidor</button>
<div id="status">Status: aguardando comando...</div>
</body>
</html>
)rawliteral";

// ===== ISR de captura =====
void IRAM_ATTR receiverISR() {
  if (!repeaterEnabled) return;
  unsigned long now = micros();
  uint16_t pulseLen = now - lastChange;
  lastChange = now;

  if (pulseCount < MAX_PULSES) {
    pulseBuffer[pulseCount++] = pulseLen;
  } else {
    signalCaptured = true;
  }
}

// ===== Envia sinal capturado =====
void sendCapturedSignal() {
  Serial.println("[DEBUG] Enviando sinal capturado...");
  
  ELECHOUSE_cc1101.setModul(0);
  ELECHOUSE_cc1101.Init();
  ELECHOUSE_cc1101.setModulation(2);
  ELECHOUSE_cc1101.setMHZ(frequency);
  ELECHOUSE_cc1101.setDeviation(0);
  ELECHOUSE_cc1101.SetTx();

  for (uint16_t i = 0; i < pulseCount; i++) {
    digitalWrite(TXPin0, (i % 2 == 0) ? HIGH : LOW);
    delayMicroseconds(pulseBuffer[i]);
  }
  digitalWrite(TXPin0, LOW);

  ELECHOUSE_cc1101.SetRx();
  Serial.println("[DEBUG] Envio finalizado.");
}

// ===== Setup =====
void setup() {
  Serial.begin(115200);
  delay(500);

  // Wi-Fi AP
  WiFi.softAPConfig(apIP, apIP, IPAddress(255,255,255,0));
  WiFi.softAP(ssid, password);
  Serial.println(WiFi.softAPIP());

  // CC1101
  ELECHOUSE_cc1101.setModul(0);
  ELECHOUSE_cc1101.Init();
  ELECHOUSE_cc1101.setModulation(2);
  ELECHOUSE_cc1101.setMHZ(frequency);
  ELECHOUSE_cc1101.setDeviation(0);
  ELECHOUSE_cc1101.SetRx();

  pinMode(RXPin0, INPUT);
  pinMode(TXPin0, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(RXPin0), receiverISR, CHANGE);

  // Rota principal (HTML embutido)
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(200, "text/html", index_html);
  });

  // Rota de comandos
  server.on("/cmd", HTTP_GET, [](AsyncWebServerRequest *request){
    if (request->hasParam("act")) {
      String action = request->getParam("act")->value();
      if (action == "start") {
        repeaterEnabled = true;
        pulseCount = 0;
        signalCaptured = false;
        request->send(200, "text/plain", "Repetidor Ativado");
      } else if (action == "stop") {
        repeaterEnabled = false;
        request->send(200, "text/plain", "Repetidor Desativado");
      } else {
        request->send(200, "text/plain", "Comando inválido");
      }
    } else {
      request->send(200, "text/plain", "Sem comando");
    }
  });

  server.begin();
}

// ===== Loop =====
void loop() {
  if (repeaterEnabled && pulseCount > 20 && micros() - lastChange > 5000 && !signalCaptured) {
    signalCaptured = true;
  }

  if (signalCaptured && repeaterEnabled) {
    detachInterrupt(digitalPinToInterrupt(RXPin0));
    Serial.print("[DEBUG] Pulsos capturados: ");
    Serial.println(pulseCount);

    sendCapturedSignal();

    pulseCount = 0;
    signalCaptured = false;
    attachInterrupt(digitalPinToInterrupt(RXPin0), receiverISR, CHANGE);
  }
}
