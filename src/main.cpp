/*
 * ============================================================
 *  MultiTool v3.0  —  ESP32-C3 Handheld Instrument
 *  Main tab: hardware init, globals, input layer, audio
 * ============================================================
 *
 *  I2C NOTE: ESP32-C3 internal pull-ups (~45 kΩ) are too weak
 *  for reliable I2C on a breadboard. Add 4.7 kΩ resistors from
 *  SDA (GPIO8) and SCL (GPIO9) to 3.3 V for a permanent fix.
 *
 *  THERMISTOR WIRING (pull-down):
 *    3.3V ─── NTC ─── GPIO1 ─── 10 kΩ ─── GND
 *  Beta calibrated from measured ~8.2 kΩ @ 21-22 °C.
 */

#include <Arduino.h>
#include "main.hpp"
#include "Menu.hpp"

// ── Default colour palette (user-customisable via Settings) ───
// Accent colours are stored in NVS as "colAccent" etc.
// All COL_* are variables so Settings can change them live.
uint16_t COL_ACCENT  = 0x07FF;   // Cyan
uint16_t COL_GOLD    = 0xFEA0;   // Amber
uint16_t COL_OK      = ST7735_GREEN;
uint16_t COL_HEAD    = 0xFD20;   // Orange
uint16_t COL_PURPLE  = 0xF81F;   // Magenta

// ── Globals ───────────────────────────────────────────────────
Preferences      prefs;
Adafruit_ST7735  tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
Adafruit_VL53L0X lox = Adafruit_VL53L0X();

// bool  isCzech    = true;
int   simonSpeed = 150;
bool  soundOn    = true;
int   brightness = 255;

AppState currentState     = STATE_MENU;
int      currentMenuLevel = 0;
bool     updateScreen     = true;

bool loxAvailable = false;

// ── Boot ──────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);

  SPI.begin(TFT_SCLK, -1, TFT_MOSI, TFT_CS);
  tft.initR(INITR_BLACKTAB);
  tft.setRotation(0);
  tft.fillScreen(COL_BG);
  tft.setTextColor(COL_ACCENT);
  tft.setTextSize(1);
  tft.setCursor(14, 30);
  tft.print("MULTI-TOOL");
  tft.setCursor(14, 50);
  tft.setTextColor(COL_DIM);
  tft.print("Initialising...");
  drawBootBar(0);
  //Making the loading more fun
  delay(200);

  prefs.begin("settings", false);
  // isCzech    = prefs.getBool ("lang",      true);
  simonSpeed = prefs.getInt  ("simon",     150);
  soundOn    = prefs.getBool ("sound",     true);
  brightness = prefs.getInt  ("bright",   255);
  COL_ACCENT = prefs.getUInt ("colAccent", 0x07FF);
  COL_GOLD   = prefs.getUInt ("colGold",   0xFEA0);
  COL_OK     = prefs.getUInt ("colOk",     ST7735_GREEN);
  COL_HEAD   = prefs.getUInt ("colHead",   0xFD20);

  pinMode(PIN_BUZZ,   OUTPUT);
  pinMode(PIN_K1_UP,  INPUT_PULLUP);
  pinMode(PIN_K4_SEL, INPUT_PULLUP);
  analogReadResolution(12);

  drawBootBar(20);
    //Making the loading more fun
  delay(200);

  // I2C soft pull-up assist
  pinMode(I2C_SDA, OUTPUT); digitalWrite(I2C_SDA, HIGH);
  pinMode(I2C_SCL, OUTPUT); digitalWrite(I2C_SCL, HIGH);
  delay(10);
  pinMode(I2C_SDA, INPUT_PULLUP);
  pinMode(I2C_SCL, INPUT_PULLUP);
  delay(5);
  Wire.begin(I2C_SDA, I2C_SCL);
  Wire.setClock(100000);

  drawBootBar(70);
  //Making the loading more fun
  delay(200);

  // Probe VL53L0X @ 0x29
  Wire.beginTransmission(0x29);
  if (Wire.endTransmission() == 0) {
    tft.setCursor(14, 75); tft.setTextColor(COL_DIM);
    tft.print("VL53L0X...");
    if (lox.begin(0x29, false, &Wire)) {
      lox.startRangeContinuous();
      loxAvailable = true;
      tft.setTextColor(COL_OK); tft.print(" OK");
    } else {
      tft.setTextColor(COL_WARN); tft.print(" FAIL");
    }
  } else {
    tft.setCursor(14, 75); tft.setTextColor(COL_WARN);
    tft.print("VL53L0X not found");
  }

  drawBootBar(100);
  delay(800);
  tft.fillScreen(COL_BG);
  randomSeed(analogRead(PIN_LDR) + analogRead(PIN_NTC) + millis());
}

void drawBootBar(int pct) {
  int x = 14, y = 100, w = 100, h = 8;
  tft.drawRect(x, y, w, h, COL_DIM);
  tft.fillRect(x + 1, y + 1, (w - 2) * pct / 100, h - 2, COL_ACCENT);
  tft.fillRect(x, y + 12, w, 10, COL_BG);
  tft.setCursor(x, y + 12); tft.setTextColor(COL_DIM);
  tft.print(String(pct) + (pct < 100 ? "% loading" : "% ready!"));
}

// ── Main loop ─────────────────────────────────────────────────
void loop() {
  if (currentState == STATE_MENU) runMenu();
  else                            runApp();
}

// ── Input layer ───────────────────────────────────────────────
bool btnUp()   { return digitalRead(PIN_K1_UP)  == LOW; }
bool btnSel()  { return digitalRead(PIN_K4_SEL) == LOW; }
bool btnDown() { return analogRead(PIN_A_BTN) < 200; }
bool btnBack() { int v = analogRead(PIN_A_BTN); return (v >= 1000 && v <= 2600); }

int waitButton(unsigned long timeoutMs = 0) {
  unsigned long t0 = millis();
  while (true) {
    if (btnUp())   return 0;
    if (btnDown()) return 1;
    if (btnBack()) return 2;
    if (btnSel())  return 3;
    if (timeoutMs && (millis() - t0 > timeoutMs)) return -1;
    delay(10);
  }
}

// ── Audio ─────────────────────────────────────────────────────
void playTone(int freq, int dur) {
  if (!soundOn || freq <= 0) return;
  tone(PIN_BUZZ, freq, dur);
  delay(dur);
  noTone(PIN_BUZZ);
}
void playClick() { playTone(1200, 18); }
void playNav()   { playTone(900,  14); }
void playBack()  { playTone(550,  20); }
void playError() { playTone(200, 150); delay(80); playTone(150, 300); }
void playWin()   { playTone(523,80); delay(40); playTone(659,80); delay(40); playTone(784,160); }

// ── Drawing helpers ───────────────────────────────────────────
void patchText(int x, int y, int w, int h, String val, uint16_t col) {
  tft.fillRect(x, y, w, h, COL_BG);
  tft.setCursor(x, y);
  tft.setTextColor(col);
  tft.setTextSize(1);
  tft.print(val);
}

void drawHeader(const char* title, uint16_t col) {
  tft.fillRect(0, 0, 128, 18, 0x0010);
  tft.setTextColor(col);
  tft.setTextSize(1);
  tft.setCursor(4, 5);
  tft.print(title);
  tft.drawLine(0, 18, 128, 18, COL_DIM);
}

void drawCentered(int y, const char* txt, uint16_t col, int sz = 1) {
  tft.setTextSize(sz);
  int len = strlen(txt) * 6 * sz;
  tft.setCursor((128 - len) / 2, y);
  tft.setTextColor(col);
  tft.print(txt);
}

void returnToMenu(int level = 1) {
  currentState     = STATE_MENU;
  currentMenuLevel = level;
  updateScreen     = true;
  delay(220);
}
