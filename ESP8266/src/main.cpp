#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <SinricPro.h>
#include <SinricProSwitch.h>
#include <SinricProLight.h>   // gunakan Light, bukan Dimmer

#include <ESP8266WebServer.h>
#include <RBDdimmer.h>
#include <ESP8266HTTPClient.h>
// Web server lokal pada port 80
ESP8266WebServer server(80);

// -------------------- Konfigurasi WiFi & SinricPro --------------------

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <SinricPro.h>
#include <SinricProSwitch.h>
#include <SinricProLight.h>
#include <RBDdimmer.h>

#define WIFI_SSID     "Trainer Instalasi"
#define WIFI_PASS     "Nagamerah10"
#define APP_KEY       "2d75ac98-4b07-446a-86ce-c863117804e3"
#define APP_SECRET    "9f8ed505-7789-467a-91a9-c21f0e28bf96-b3dcf12f-a3b6-445a-a463-2f8933a872af"
#define SWITCH_ID1    "69651077dc90b2c9e0750c6e"
#define SWITCH_ID2    "696511bc21c7ee0f7e59d594"
#define SWITCH_ID3    "6965120bdc90b2c9e0750d84"
#define SWITCH_ID4    "SINRICPRO_SWITCH_ID4"
#define SWITCH_ID5    "SINRICPRO_SWITCH_ID5" // Ganti dengan ID switch kelima dari SinricPro
#define DIMMER_ID     "SINRICPRO_DIMMER_ID"

#define RELAY1_PIN    D1
#define RELAY2_PIN    D2
#define RELAY3_PIN    D0
#define RELAY4_PIN    D4
#define BUTTON1_PIN   D3
#define BUTTON2_PIN   D7
#define PIR_PIN       D8
#define RELAY5_PIN    -1

// Set to 1 if relay module is active-low (LOW = ON), 0 if active-high (HIGH = ON)
#define RELAY_ACTIVE_LOW 0

#define DIMMER_OUTPUT_PIN  A0
#define ZEROCROSS_PIN      D5
#define NOTIF_HOST "192.168.0.100" // ganti dengan IP/hostname server notifikasi Anda

// Deklarasi objek switch global sebagai referensi
SinricProSwitch& sw1 = SinricPro[SWITCH_ID1];
SinricProSwitch& sw2 = SinricPro[SWITCH_ID2];
SinricProSwitch& sw3 = SinricPro[SWITCH_ID3];
SinricProSwitch& sw4 = SinricPro[SWITCH_ID4];
SinricProSwitch& sw5 = SinricPro[SWITCH_ID5];

// Function prototype for handleSwitchPower
bool handleSwitchPower(const String &deviceId, bool &state, int idx);

bool handleSwitchPower5(const String &deviceId, bool &state) { return handleSwitchPower(deviceId, state, 4); }

// Deklarasi objek switch global sebagai referensi

bool lampState[5] = {false, false, false, false, false};
bool pirDetected = false;
unsigned long pirLastDetectedMillis = 0;
int dimmerValue = 50;
dimmerLamp dimmer(DIMMER_OUTPUT_PIN, ZEROCROSS_PIN);

void setRelay(int idx, bool state) {
  int relayPins[5] = {RELAY1_PIN, RELAY2_PIN, RELAY3_PIN, RELAY4_PIN, RELAY5_PIN};
  if (idx >= 0 && idx < 5) {
    // Active-high relay: ON -> HIGH, OFF -> LOW
    int pin = relayPins[idx];
    if (pin >= 0) {
      int out = (state ? (RELAY_ACTIVE_LOW ? LOW : HIGH) : (RELAY_ACTIVE_LOW ? HIGH : LOW));
      digitalWrite(pin, out);
    }
  }
}

bool handleSwitchPower(const String &deviceId, bool &state, int idx) {
  lampState[idx] = state;
  setRelay(idx, state);
  return true;
}
bool handleSwitchPower1(const String &deviceId, bool &state) { return handleSwitchPower(deviceId, state, 0); }
bool handleSwitchPower2(const String &deviceId, bool &state) { return handleSwitchPower(deviceId, state, 1); }
bool handleSwitchPower3(const String &deviceId, bool &state) { return handleSwitchPower(deviceId, state, 2); }
bool handleSwitchPower4(const String &deviceId, bool &state) { return handleSwitchPower(deviceId, state, 3); }

bool onPowerState(const String &deviceId, bool &state) {
  Serial.printf("Dimmer %s\n", state ? "ON" : "OFF");
  dimmer.setState(state ? ON : OFF);
  return true;
}

// Fungsi untuk mengatur kecerahan (brightness)
bool onBrightness(const String &deviceId, int &brightness) {
  Serial.printf("Brightness: %d\n", brightness);
  dimmerValue = brightness;
  dimmer.setPower(brightness);
  return true;
}

void handleRelay1On() {
  lampState[0] = true;
  setRelay(0, true);
  server.send(200, "text/plain", "Relay 1 ON");
}

void handleRelay1Off() {
  lampState[0] = false;
  setRelay(0, false);
  server.send(200, "text/plain", "Relay 1 OFF");
}

void setup() {
  // Endpoint status semua relay dan dimmer
  server.on("/status", []() {
    String json = "{";
    json += "\"relay1\":" + String(lampState[0] ? 1 : 0) + ",";
    json += "\"relay2\":" + String(lampState[1] ? 1 : 0) + ",";
    json += "\"relay3\":" + String(lampState[2] ? 1 : 0) + ",";
    json += "\"relay4\":" + String(lampState[3] ? 1 : 0) + ",";
    json += "\"dimmer\":" + String(dimmerValue);
    json += "}";
    server.send(200, "application/json", json);
  });
  sw5.onPowerState(handleSwitchPower5);
  Serial.begin(115200);
  // Set output level before configuring as OUTPUT to avoid transient/high pulses
  int relayOffLevel = (RELAY_ACTIVE_LOW ? HIGH : LOW); // OFF = HIGH when active-low
  digitalWrite(RELAY1_PIN, relayOffLevel);
  digitalWrite(RELAY2_PIN, relayOffLevel);
  digitalWrite(RELAY3_PIN, relayOffLevel);
  digitalWrite(RELAY4_PIN, relayOffLevel);
  pinMode(RELAY1_PIN, OUTPUT);
  pinMode(RELAY2_PIN, OUTPUT);
  pinMode(RELAY3_PIN, OUTPUT);
  pinMode(RELAY4_PIN, OUTPUT);
  pinMode(BUTTON1_PIN, INPUT_PULLUP);
  pinMode(BUTTON2_PIN, INPUT_PULLUP);
  pinMode(PIR_PIN, INPUT);
  for (int i = 0; i < 5; i++) setRelay(i, false);
  dimmer.begin(NORMAL_MODE, ON);
  dimmer.setPower(dimmerValue);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  // Print pin mapping for debugging
  Serial.printf("Pins: RELAY1=%d RELAY2=%d BUTTON1=%d BUTTON2=%d PIR=%d\n", RELAY1_PIN, RELAY2_PIN, BUTTON1_PIN, BUTTON2_PIN, PIR_PIN);

  // Endpoint kontrol relay via HTTP
  server.on("/relay1/on", []() {
    lampState[0] = true;
    setRelay(0, true);
    sw1.sendPowerStateEvent(true);
    server.send(200, "text/plain", "Relay 1 ON");
  });
  server.on("/relay1/off", []() {
    lampState[0] = false;
    setRelay(0, false);
    sw1.sendPowerStateEvent(false);
    server.send(200, "text/plain", "Relay 1 OFF");
  });
  server.on("/relay2/on", []() {
    lampState[1] = true;
    setRelay(1, true);
    sw2.sendPowerStateEvent(true);
    server.send(200, "text/plain", "Relay 2 ON");
  });
  server.on("/relay2/off", []() {
    lampState[1] = false;
    setRelay(1, false);
    sw2.sendPowerStateEvent(false);
    server.send(200, "text/plain", "Relay 2 OFF");
  });
  // Relay 3
  server.on("/relay3/on", []() {
    lampState[2] = true;
    setRelay(2, true);
    sw3.sendPowerStateEvent(true);
    server.send(200, "text/plain", "Relay 3 ON");
  });
  server.on("/relay3/off", []() {
    lampState[2] = false;
    setRelay(2, false);
    sw3.sendPowerStateEvent(false);
    server.send(200, "text/plain", "Relay 3 OFF");
  });
  // Relay 4
  server.on("/relay4/on", []() {
    lampState[3] = true;
    setRelay(3, true);
    sw4.sendPowerStateEvent(true);
    server.send(200, "text/plain", "Relay 4 ON");
  });
  server.on("/relay4/off", []() {
    lampState[3] = false;
    setRelay(3, false);
    sw4.sendPowerStateEvent(false);
    server.send(200, "text/plain", "Relay 4 OFF");
  });
  // Relay 5
  server.on("/relay5/on", []() {
    lampState[4] = true;
    setRelay(4, true);
    server.send(200, "text/plain", "Relay 5 ON");
  });
  server.on("/relay5/off", []() {
    lampState[4] = false;
    setRelay(4, false);
    server.send(200, "text/plain", "Relay 5 OFF");
  });

  // Dimmer ON/OFF
  server.on("/dimmer/on", []() {
    dimmer.setState(ON);
    server.send(200, "text/plain", "Dimmer ON");
  });
  server.on("/dimmer/off", []() {
    dimmer.setState(OFF);
    server.send(200, "text/plain", "Dimmer OFF");
  });
  // Dimmer set value
  server.on("/dimmer/set", []() {
    if (server.hasArg("value")) {
      int val = server.arg("value").toInt();
      dimmerValue = val;
      dimmer.setPower(val);
      Serial.print("[WEB] Dimmer set to: ");
      Serial.println(val);
      server.send(200, "text/plain", String("Dimmer set to ") + val);
    } else {
      server.send(400, "text/plain", "Missing value param");
    }
  });

  // Endpoint status PIR
  // Endpoint status PIR (with CORS)
  server.on("/pir", HTTP_OPTIONS, []() {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
    server.send(204);
  });
  server.on("/pir", []() {
    // Return both raw pin and latched PIR status
    // Latched = 1 if motion detected within last 15 seconds (helps UI visibility)
    unsigned long now = millis();
    int raw = digitalRead(PIR_PIN);
    int latched = (now - pirLastDetectedMillis) < 15000 ? 1 : 0;
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
    String json = "{";
    json += "\"raw\":" + String(raw) + ",";
    json += "\"status\":" + String(latched) + ",";
    json += "\"since_ms\":" + String(now - pirLastDetectedMillis);
    json += "}";
    server.send(200, "application/json", json);
  });
  // Tambahkan endpoint lain sesuai kebutuhan

  server.begin();
  sw1.onPowerState(handleSwitchPower1);
  sw2.onPowerState(handleSwitchPower2);
  sw3.onPowerState(handleSwitchPower3);
  sw4.onPowerState(handleSwitchPower4);

  SinricProLight& myDimmer = SinricPro[DIMMER_ID];
  myDimmer.onPowerState(onPowerState);
  myDimmer.onBrightness(onBrightness);

  SinricPro.begin(APP_KEY, APP_SECRET);
}

void loop() {
  SinricPro.handle();
  server.handleClient();
  // Debounced button handling: require stable LOW for `debounceDelay` then toggle on press,
  // wait for release before accepting next press.
  const int numButtons = 2;
  int btnPins[numButtons] = {BUTTON1_PIN, BUTTON2_PIN};
  static int lastReading[numButtons] = {HIGH, HIGH};
  static int stableState[numButtons] = {HIGH, HIGH};
  static unsigned long lastDebounceTime[numButtons] = {0,0};
  static unsigned long lastEventMillis[numButtons] = {0,0};
  const unsigned long debounceDelay = 50; // ms
  unsigned long now = millis();
  for (int i = 0; i < numButtons; i++) {
    int reading = digitalRead(btnPins[i]);
    if (reading != lastReading[i]) {
      // reset debounce timer
      lastDebounceTime[i] = now;
      lastReading[i] = reading;
    }
    if ((now - lastDebounceTime[i]) > debounceDelay) {
      // if the button state has changed
      if (reading != stableState[i]) {
        stableState[i] = reading;
        // only act on transition to LOW (pressed)
        if (stableState[i] == LOW) {
          int relayIdx = i; // tombol ke-0 -> relay0, tombol ke-1 -> relay1
          lampState[relayIdx] = !lampState[relayIdx];
          setRelay(relayIdx, lampState[relayIdx]);
          Serial.printf("BUTTON: idx=%d pin=%d new=%s\n", relayIdx, btnPins[i], lampState[relayIdx] ? "ON" : "OFF");
          if (now - lastEventMillis[i] > 500) {
            if (relayIdx == 0) sw1.sendPowerStateEvent(lampState[relayIdx]);
            if (relayIdx == 1) sw2.sendPowerStateEvent(lampState[relayIdx]);
            lastEventMillis[i] = now;
          }
        }
      }
    }
  }
  // Active-high PIR: sensor output is HIGH when motion is detected
  bool pirNow = (digitalRead(PIR_PIN) == HIGH);

  // Periodic debug print: raw pin value and interpreted active-high state
  static unsigned long lastPirLogMillis = 0;
  unsigned long nowMillis = millis();
  if (nowMillis - lastPirLogMillis >= 1000) { // every 1s
    int raw = digitalRead(PIR_PIN);
    Serial.printf("[DEBUG] PIR raw=%d active=%d\n", raw, pirNow ? 1 : 0);
    lastPirLogMillis = nowMillis;
  }

  // Periodic button raw debug every 500ms
  static unsigned long lastBtnLogMillis = 0;
  if (nowMillis - lastBtnLogMillis >= 500) {
    int b0 = digitalRead(BUTTON1_PIN);
    int b1 = digitalRead(BUTTON2_PIN);
    Serial.printf("[DEBUG] BTN raw: btn1=%d btn2=%d\n", b0, b1);
    lastBtnLogMillis = nowMillis;
  }
  if (pirNow != pirDetected) {
    pirDetected = pirNow;
    Serial.printf("PIR: %s\n", pirDetected ? "Terdeteksi" : "Tidak Terdeteksi");
    // 1. Nyalakan lampu 1 saat PIR aktif
    if (pirDetected) {
      pirLastDetectedMillis = millis();
      lampState[0] = true;
      setRelay(0, true);
      sw1.sendPowerStateEvent(true);
      // 2. Kirim notifikasi ke web (misal via HTTP GET)
      if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        WiFiClient wifiClient;
        String url = String("http://") + NOTIF_HOST + "/notifikasi?pir=1";
        http.begin(wifiClient, url);
        int httpCode = http.GET();
        Serial.printf("Notif sent, code=%d\n", httpCode);
        http.end();
      } else {
        Serial.println("Notif failed: WiFi not connected");
      }
    } else {
      lampState[0] = false;
      setRelay(0, false);
      sw1.sendPowerStateEvent(false);
      if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        WiFiClient wifiClient;
        String url = String("http://") + NOTIF_HOST + "/notifikasi?pir=0";
        http.begin(wifiClient, url);
        int httpCode = http.GET();
        Serial.printf("Notif sent, code=%d\n", httpCode);
        http.end();
      } else {
        Serial.println("Notif failed: WiFi not connected");
      }
    }
  }
}