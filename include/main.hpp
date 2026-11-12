#ifndef MAIN_HPP
#define MAIN_HPP

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <Adafruit_VL53L0X.h>
#include <Preferences.h>
#include <WiFi.h>
#include <BLEDevice.h>
#include <BLEScan.h>

// ── Pin definitions ──────────────────────────────────────────
#define PIN_LDR     0
#define PIN_NTC     1
#define PIN_BUZZ    2
#define PIN_A_BTN   3
#define TFT_SCLK    4
#define PIN_K1_UP   5
#define TFT_MOSI    6
#define TFT_CS      7
#define I2C_SDA     8
#define I2C_SCL     9
#define TFT_RST     10 
#define TFT_DC      20
#define PIN_K4_SEL  21

// Fixed colours (never change)
#define COL_BG      ST7735_BLACK
#define COL_FG      ST7735_WHITE
#define COL_DIM     0x4208
#define COL_SEL_BG  0x0319
#define COL_WARN    ST7735_RED

// ── Thermistor constants ──────────────────────────────────────
#define NTC_SERIES   10000.0f
#define NTC_R0        8200.0f
#define NTC_T0        294.15f
#define NTC_BETA      3380.0f

enum AppState { STATE_MENU, STATE_APP };

/* externs */
extern uint16_t COL_ACCENT;// Cyan
extern uint16_t COL_GOLD;   // Amber
extern uint16_t COL_OK;
extern uint16_t COL_HEAD;   // Orange
extern uint16_t COL_PURPLE;   // Magenta

extern Preferences      prefs;
extern Adafruit_ST7735  tft;
extern Adafruit_VL53L0X lox;

extern bool  isCzech;
extern int   simonSpeed;
extern bool  soundOn;
extern int   brightness;

extern AppState currentState;
extern int      currentMenuLevel;
extern bool     updateScreen;

extern bool loxAvailable;

/* Function protos */
void drawBootBar(int pct);

bool btnUp();
bool btnSel();
bool btnDown();
bool btnBack();

int waitButton(unsigned long timeoutMs);

void playTone(int freq, int dur);

void playClick();
void playNav();
void playBack();
void playError();
void playWin();

void patchText(int x, int y, int w, int h, String val, uint16_t col);
void drawHeader(const char* title, uint16_t col);
void drawCentered(int y, const char* txt, uint16_t col, int sz);
void returnToMenu(int level);

#endif