#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <SinricPro.h>
#include <SinricProSwitch.h>
#include <SinricProLight.h>   // gunakan Light, bukan Dimmer
#include <RBDdimmer.h>

// -------------------- Konfigurasi WiFi & SinricPro --------------------
#define WIFI_SSID     "Puput"
#define WIFI_PASS     "gunawan123"

#define APP_KEY       "23d7a9a0-35b4-4e8f-b869-4fb0367e35f3"
#define APP_SECRET    "1a544df4-3cd3-4945-94c0-9932511fd82d-19eff719-0084-4282-b970-48560119a74d"

#define SWITCH_ID1    "68ca6343033cb129b008f852"
#define SWITCH_ID2    "68ca63b1033cb129b008f8d9"
#define SWITCH_ID3    "68ca6403b73c366187ee264b"
#define SWITCH_ID4    "SINRICPRO_SWITCH_ID4"
#define DIMMER_ID     "SINRICPRO_DIMMER_ID"

// -------------------- Pin ESP8266 --------------------
#define RELAY1_PIN    D1
#define RELAY2_PIN    D2
#define RELAY3_PIN    D3
#define RELAY4_PIN    D4

#define BUTTON1_PIN   D5
#define BUTTON2_PIN   D6
#define BUTTON3_PIN   D7
#define BUTTON4_PIN   RX

#define PIR_PIN       D0

// RobotDyn AC Dimmer
#define DIMMER_OUTPUT_PIN  D8   // ke pin modul dimmer (Gate)
#define ZEROCROSS_PIN      D3   // ke pin Zero-Cross modul dimmer

// -------------------- Variabel --------------------
bool lampState[4] = {false, false, false, false};
bool pirDetected = false;
int dimmerValue = 50;

// -------------------- RobotDyn Dimmer --------------------
dimmerLamp dimmer(DIMMER_OUTPUT_PIN, ZEROCROSS_PIN);

// -------------------- Fungsi Relay --------------------
void setRelay(int idx, bool state) {
  int relayPins[4] = {RELAY1_PIN, RELAY2_PIN, RELAY3_PIN, RELAY4_PIN};
  digitalWrite(relayPins[idx], state ? HIGH : LOW);
}

// -------------------- Callback Switch --------------------
void handleSwitchPower1(const String &deviceId, bool state) {
  lampState[0] = state;
  setRelay(0, state);
}
void handleSwitchPower2(const String &deviceId, bool state) {
  lampState[1] = state;
  setRelay(1, state);
}
void handleSwitchPower3(const String &deviceId, bool state) {
  lampState[2] = state;
  setRelay(2, state);
}
void handleSwitchPower4(const String &deviceId, bool state) {
  lampState[3] = state;
  setRelay(3, state);
}

// -------------------- Callback Dimmer --------------------
bool onPowerState(const String &deviceId, bool &state) {
  Serial.printf("Dimmer %s\n", state ? "ON" : "OFF");
  dimmer.setState(state);
  return true;
}

bool onBrightness(const String &deviceId, int &brightness) {
  Serial.printf("Brightness: %d\n", brightness);
  dimmerValue = brightness;
  dimmer.setPower(brightness);   // atur 0–100%
  return true;
}

// -------------------- Setup --------------------
void setup() {
  Serial.begin(115200);

  pinMode(RELAY1_PIN, OUTPUT);
  pinMode(RELAY2_PIN, OUTPUT);
  pinMode(RELAY3_PIN, OUTPUT);
  pinMode(RELAY4_PIN, OUTPUT);

  pinMode(BUTTON1_PIN, INPUT_PULLUP);
  pinMode(BUTTON2_PIN, INPUT_PULLUP);
  pinMode(BUTTON3_PIN, INPUT_PULLUP);
  pinMode(BUTTON4_PIN, INPUT_PULLUP);

  pinMode(PIR_PIN, INPUT);

  for (int i = 0; i < 4; i++) setRelay(i, false);

  // Setup RobotDyn dimmer
  dimmer.begin(NORMAL_MODE, ON);
  dimmer.setPower(dimmerValue);

  // WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected!");

  // SinricPro Switch
  SinricProSwitch &sw1 = SinricPro[SWITCH_ID1];
  sw1.onPowerState(handleSwitchPower1);
  SinricProSwitch &sw2 = SinricPro[SWITCH_ID2];
  sw2.onPowerState(handleSwitchPower2);
  SinricProSwitch &sw3 = SinricPro[SWITCH_ID3];
  sw3.onPowerState(handleSwitchPower3);
  SinricProSwitch &sw4 = SinricPro[SWITCH_ID4];
  sw4.onPowerState(handleSwitchPower4);

  // SinricPro Dimmer (pakai Light)
  SinricProLight &myDimmer = SinricPro[DIMMER_ID];
  myDimmer.onPowerState(onPowerState);
  myDimmer.onBrightness(onBrightness);

  SinricPro.begin(APP_KEY, APP_SECRET);
}

// -------------------- Loop --------------------
void loop() {
  SinricPro.handle();

  // Tombol fisik
  static bool lastBtn[4] = {HIGH, HIGH, HIGH, HIGH};
  int btnPins[4] = {BUTTON1_PIN, BUTTON2_PIN, BUTTON3_PIN, BUTTON4_PIN};

  for (int i = 0; i < 4; i++) {
    bool btn = digitalRead(btnPins[i]);
    if (btn == LOW && lastBtn[i] == HIGH) {
      lampState[i] = !lampState[i];
      setRelay(i, lampState[i]);
      switch(i) {
        case 0: SinricPro[SWITCH_ID1].sendPowerStateEvent(lampState[i]); break;
        case 1: SinricPro[SWITCH_ID2].sendPowerStateEvent(lampState[i]); break;
        case 2: SinricPro[SWITCH_ID3].sendPowerStateEvent(lampState[i]); break;
        case 3: SinricPro[SWITCH_ID4].sendPowerStateEvent(lampState[i]); break;
      }
      delay(200);
    }
    lastBtn[i] = btn;
  }

  // Sensor PIR
  bool pirNow = digitalRead(PIR_PIN);
  if (pirNow != pirDetected) {
    pirDetected = pirNow;
    Serial.printf("PIR: %s\n", pirDetected ? "Terdeteksi" : "Tidak Terdeteksi");
    // opsional: bisa kirim event ke SinricPro
  }
}
