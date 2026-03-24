/*
  Project Happy Plant — ESP32 Sketch
  -----------------------------------------------
  Pin 5  → Relay (pump)
  Pin 4  → Physical button (triggers pump)

  Endpoints the website calls:
    GET /water                  → trigger pump once for pumpDuration seconds
    GET /setDuration?seconds=N  → set how long the pump runs
    GET /setSchedule?days=N     → set the repeating watering interval in days
*/

#include <WiFi.h>
#include <WebServer.h>

// ── WiFi credentials ──────────────────────────
const char* ssid     = "TheMintWIFI";      // <-- replace
const char* password = "bestwifi64";  // <-- replace

// ── Pin definitions ───────────────────────────
const int PUMP_PIN   = 5;
const int BUTTON_PIN = 4;

// ── State ─────────────────────────────────────
int  pumpDuration   = 5;
int  scheduleDays   = 1;
bool pumpRunning    = false;
unsigned long pumpStartTime = 0;

unsigned long lastWaterTime = 0;
bool scheduleEnabled        = false;

int  lastButtonState        = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;

WebServer server(80);

void triggerPump() {
  if (pumpRunning) return;
  digitalWrite(PUMP_PIN, HIGH);
  pumpRunning   = true;
  pumpStartTime = millis();
  lastWaterTime = millis();
  Serial.println("Pump ON");
}

void addCORSHeaders() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
}

void handleWater() {
  addCORSHeaders();
  triggerPump();
  server.send(200, "text/plain", "OK");
}

void handleSetDuration() {
  addCORSHeaders();
  if (server.hasArg("seconds")) {
    int val = server.arg("seconds").toInt();
    if (val >= 1 && val <= 60) {
      pumpDuration = val;
      Serial.print("Pump duration set to ");
      Serial.print(pumpDuration);
      Serial.println("s");
    }
  }
  server.send(200, "text/plain", "OK");
}

void handleSetSchedule() {
  addCORSHeaders();
  if (server.hasArg("days")) {
    int val = server.arg("days").toInt();
    if (val >= 1 && val <= 30) {
      scheduleDays    = val;
      scheduleEnabled = true;
      lastWaterTime   = millis();
      Serial.print("Schedule set to every ");
      Serial.print(scheduleDays);
      Serial.println(" day(s)");
    }
  }
  server.send(200, "text/plain", "OK");
}

void handleOptions() {
  addCORSHeaders();
  server.send(204);
}

void setup() {
  Serial.begin(115200);

  pinMode(PUMP_PIN,   OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  digitalWrite(PUMP_PIN, LOW);

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/water",       HTTP_GET,     handleWater);
  server.on("/setDuration", HTTP_GET,     handleSetDuration);
  server.on("/setSchedule", HTTP_GET,     handleSetSchedule);
  server.on("/water",       HTTP_OPTIONS, handleOptions);
  server.on("/setDuration", HTTP_OPTIONS, handleOptions);
  server.on("/setSchedule", HTTP_OPTIONS, handleOptions);

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();

  if (pumpRunning && (millis() - pumpStartTime >= (unsigned long)pumpDuration * 1000)) {
    digitalWrite(PUMP_PIN, LOW);
    pumpRunning = false;
    Serial.println("Pump OFF");
  }

  int reading = digitalRead(BUTTON_PIN);
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading == LOW) {
      triggerPump();
    }
  }
  lastButtonState = reading;

  if (scheduleEnabled && !pumpRunning) {
    unsigned long intervalMs = (unsigned long)scheduleDays * 24UL * 60UL * 60UL * 1000UL;
    if (millis() - lastWaterTime >= intervalMs) {
      Serial.println("Scheduled watering triggered");
      triggerPump();
    }
  }
}