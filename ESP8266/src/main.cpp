#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <SinricPro.h>
#include <SinricProSwitch.h>
#include <SinricProLight.h>   // gunakan Light, bukan Dimmer

#include <ESP8266WebServer.h>
#include <RBDdimmer.h>
// Web server lokal pada port 80
ESP8266WebServer server(80);

// -------------------- Konfigurasi WiFi & SinricPro --------------------

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <SinricPro.h>
#include <SinricProSwitch.h>
#include <SinricProLight.h>
#include <RBDdimmer.h>

#define WIFI_SSID     "Puput"
#define WIFI_PASS     "gunawan123"
#define APP_KEY       "23d7a9a0-35b4-4e8f-b869-4fb0367e35f3"
#define APP_SECRET    "1a544df4-3cd3-4945-94c0-9932511fd82d-19eff719-0084-4282-b970-48560119a74d"
#define SWITCH_ID1    "68ca6343033cb129b008f852"
#define SWITCH_ID2    "68ca63b1033cb129b008f8d9"
#define SWITCH_ID3    "68ca6403b73c366187ee264b"
#define SWITCH_ID4    "SINRICPRO_SWITCH_ID4"
#define SWITCH_ID5    "SINRICPRO_SWITCH_ID5" // Ganti dengan ID switch kelima dari SinricPro
#define DIMMER_ID     "SINRICPRO_DIMMER_ID"

#define RELAY1_PIN    D1
#define RELAY2_PIN    D2
#define RELAY3_PIN    D3
#define RELAY4_PIN    D4
#define RELAY5_PIN    D9 // Ganti D9 dengan pin yang Anda gunakan untuk relay 5
#define BUTTON1_PIN   D5
#define BUTTON2_PIN   D6
#define BUTTON3_PIN   D7
#define BUTTON4_PIN   D0
#define PIR_PIN       D8

#define DIMMER_OUTPUT_PIN  D6
#define ZEROCROSS_PIN      D5

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
int dimmerValue = 50;
dimmerLamp dimmer(DIMMER_OUTPUT_PIN, ZEROCROSS_PIN);

void setRelay(int idx, bool state) {
  int relayPins[5] = {RELAY1_PIN, RELAY2_PIN, RELAY3_PIN, RELAY4_PIN, RELAY5_PIN};
  if (idx >= 0 && idx < 5) {
    digitalWrite(relayPins[idx], state ? HIGH : LOW);
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
  digitalWrite(RELAY1_PIN, HIGH);
  server.send(200, "text/plain", "Relay 1 ON");
}

void handleRelay1Off() {
  digitalWrite(RELAY1_PIN, LOW);
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
    json += "\"relay5\":" + String(lampState[4] ? 1 : 0) + ",";
    json += "\"dimmer\":" + String(dimmerValue);
    json += "}";
    server.send(200, "application/json", json);
  });
  sw5.onPowerState(handleSwitchPower5);
  Serial.begin(115200);
  pinMode(RELAY1_PIN, OUTPUT);
  pinMode(RELAY2_PIN, OUTPUT);
  pinMode(RELAY3_PIN, OUTPUT);
  pinMode(RELAY4_PIN, OUTPUT);
  pinMode(RELAY5_PIN, OUTPUT);
  pinMode(BUTTON1_PIN, INPUT_PULLUP);
  pinMode(BUTTON2_PIN, INPUT_PULLUP);
  pinMode(BUTTON3_PIN, INPUT_PULLUP);
  pinMode(BUTTON4_PIN, INPUT_PULLUP);
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
  server.on("/pir", []() {
    int pir = digitalRead(PIR_PIN);
    server.send(200, "application/json", String("{\"status\":") + pir + "}");
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
  static bool lastBtn[4] = {HIGH, HIGH, HIGH, HIGH};
  int btnPins[4] = {BUTTON1_PIN, BUTTON2_PIN, BUTTON3_PIN, BUTTON4_PIN};
  static unsigned long lastEventMillis[4] = {0,0,0,0};
  for (int i = 0; i < 4; i++) {
    bool btn = digitalRead(btnPins[i]);
    if (btn == LOW && lastBtn[i] == HIGH) {
      lampState[i] = !lampState[i];
      setRelay(i, lampState[i]);
      unsigned long now = millis();
      if (now - lastEventMillis[i] > 500) { // minimal 500ms antar event
        if (i == 0) sw1.sendPowerStateEvent(lampState[i]);
        if (i == 1) sw2.sendPowerStateEvent(lampState[i]);
        if (i == 2) sw3.sendPowerStateEvent(lampState[i]);
        if (i == 3) sw4.sendPowerStateEvent(lampState[i]);
        lastEventMillis[i] = now;
      }
      delay(200); // debounce tombol
    }
    lastBtn[i] = btn;
  }
  bool pirNow = digitalRead(PIR_PIN);
  if (pirNow != pirDetected) {
    pirDetected = pirNow;
    Serial.printf("PIR: %s\n", pirDetected ? "Terdeteksi" : "Tidak Terdeteksi");
    // 1. Nyalakan lampu 1 saat PIR aktif
    if (pirDetected) {
      lampState[0] = true;
      setRelay(0, true);
      sw1.sendPowerStateEvent(true);
      // 2. Kirim notifikasi ke web (misal via HTTP GET)
      WiFiClient client;
      if (client.connect("alamat_server_web", 80)) {
        client.print(String("GET /notifikasi?pir=1 HTTP/1.1\r\nHost: alamat_server_web\r\nConnection: close\r\n\r\n"));
      }
    } else {
      lampState[0] = false;
      setRelay(0, false);
      sw1.sendPowerStateEvent(false);
      WiFiClient client;
      if (client.connect("alamat_server_web", 80)) {
        client.print(String("GET /notifikasi?pir=0 HTTP/1.1\r\nHost: alamat_server_web\r\nConnection: close\r\n\r\n"));
      }
    }
  }
}
