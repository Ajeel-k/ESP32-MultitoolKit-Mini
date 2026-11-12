/*
 * ============================================================
 *  Menu.ino v3  —  Navigation layer
 * ============================================================
 */
#include "main.hpp"
#include "Sett.hpp"
#include "Senz.hpp"
#include "Game.hpp"
#include "Util.hpp"
#include "Menu.hpp"

int mainCursor = 0;
int subCursor  = 0;
int optCursor  = 0;

const int MAIN_ITEMS = 4;
// Sensors: AllTogether, Temp, Light, Dist  (no accel/gyro/bubble — MPU removed)
// Games: Simon, Dino, Snake, Flappy
const int SUB_ITEMS_COUNT[MAIN_ITEMS] = { 4, 9, 4, 5 };

// String tr(const char* en, const char* cz) {
//   return isCzech ? String(cz) : String(en);
// }

String getSubmenuItemText(int cat, int idx) {
  if (cat == 0) {
    const char* en[] = { "All Sensors", "Temperature", "Light Level", "Distance" };
    // const char* cz[] = { "Vsechny",     "Teplota",     "Jas svetla",  "Vzdalenost" };
    return String(en[idx]);
  }
  if (cat == 1) {
    const char* en[] = { "Tone Gen.", "Periodic Table", "Constants",
                         "Unit Conv.", "Calculator", "Wi-Fi Scan",
                         "BLE Scan",  "Stopwatch",  "Timer" };
    // const char* cz[] = { "Generator", "Prvky",          "Konstanty",
    //                      "Prevodnik", "Kalkulacka",      "Wi-Fi",
    //                      "BT Sken",   "Stopky",          "Minutka" };
    return String(en[idx]);
  }
  if (cat == 2) {
    const char* en[] = { "Simon Says", "Dino Run", "Snake", "Flappy Bird" };
    // const char* cz[] = { "Simon Says", "Dinosaur", "Had",   "Flappy Bird" };
    return String(en[idx]);
  }
  if (cat == 3) {
    const char* en[] = {"Simon Speed", "Sound", "Brightness", "Temp Offset", "Colours" };
    // const char* cz[] = { "Jazyk",    "Rychlost",    "Zvuk",  "Jas",        "Offset tepl.","Barvy" };
    return String(en[idx]);
  }
  return "";
}

const char* MAIN_LABELS_EN[MAIN_ITEMS] = { "SENSORS","UTILITIES","GAMES","SETTINGS" };
// const char* MAIN_LABELS_CZ[MAIN_ITEMS] = { "SENZORY","NASTROJE", "HRY", "NASTAVENI"};

// ── runMenu ───────────────────────────────────────────────────
void runMenu() {
  if (updateScreen) { drawMenu(); updateScreen = false; }

  static unsigned long lastBtn = 0;
  if (millis() - lastBtn < 250) return;

  bool moved = false;

  if (btnUp()) {
    playNav();
    if      (currentMenuLevel == 0) mainCursor = (mainCursor > 0) ? mainCursor-1 : MAIN_ITEMS-1;
    else if (currentMenuLevel == 1) subCursor  = (subCursor  > 0) ? subCursor-1  : SUB_ITEMS_COUNT[mainCursor]-1;
    else                            optCursor  = (optCursor  > 0) ? optCursor-1  : 1;
    moved = true;
  }
  if (btnDown()) {
    playNav();
    if      (currentMenuLevel == 0) mainCursor = (mainCursor < MAIN_ITEMS-1) ? mainCursor+1 : 0;
    else if (currentMenuLevel == 1) subCursor  = (subCursor  < SUB_ITEMS_COUNT[mainCursor]-1) ? subCursor+1 : 0;
    else                            optCursor  = (optCursor  < 1) ? optCursor+1 : 0;
    moved = true;
  }
  if (btnSel()) {
    playClick();
    while (btnSel()) delay(10);
    if (currentMenuLevel == 0) {
      currentMenuLevel = 1; subCursor = 0;
    } else if (currentMenuLevel == 1) {
      // Sensors 1-3 (not AllTogether=0) offer mode select
      if (mainCursor == 0 && subCursor >= 1 && subCursor <= 3) {
        currentMenuLevel = 2; optCursor = 0;
      } else {
        currentState = STATE_APP;
        tft.fillScreen(COL_BG);
      }
    } else {
      currentState = STATE_APP;
      tft.fillScreen(COL_BG);
    }
    moved = true;
  }
  if (btnBack()) {
    playBack();
    if (currentMenuLevel > 0) currentMenuLevel--;
    moved = true;
  }
  if (moved) { lastBtn = millis(); updateScreen = true; }
}

// ── drawMenu ──────────────────────────────────────────────────
void drawMenu() {
  tft.fillScreen(COL_BG);
  tft.setTextSize(1);

  // ── Level 0: main categories ──────────────────────────────
  if (currentMenuLevel == 0) {
    tft.fillRect(0, 0, 128, 18, 0x0010);
    tft.setTextColor(COL_GOLD);
    tft.setCursor(4, 5);
    tft.print("MULTI-TOOL ");
    tft.drawLine(0, 18, 128, 18, COL_DIM);

    for (int i = 0; i < MAIN_ITEMS; i++) {
      int yRow = 26 + i * 32;
      bool sel = (i == mainCursor);

      if (sel) {
        tft.fillRect(0, yRow-3, 128, 30, COL_SEL_BG);
        tft.drawRect(0, yRow-3, 128, 30, COL_ACCENT);
        tft.setTextColor(COL_FG);
      } else {
        tft.setTextColor(COL_DIM);
      }

      tft.setTextSize(2);
      tft.setCursor(6, yRow+3);
      tft.print(i + 1);

      tft.setTextSize(1);
      tft.setCursor(26, yRow+5);
      tft.print(MAIN_LABELS_EN[i]);

      // Item count on same line after the category name — right side
      if (sel) {
        tft.setTextColor(COL_ACCENT);
        // Print count right-aligned in the row
        String cnt = String(SUB_ITEMS_COUNT[i]);
        tft.setCursor(116 - cnt.length()*6, yRow+5);
        tft.print(cnt);
        tft.setCursor(116, yRow+5);
        tft.print(">");
      }
    }
    // No footer text — it was cut off at screen edge anyway
  }

  // ── Level 1: sub-items ────────────────────────────────────
  else if (currentMenuLevel == 1) {
    tft.fillRect(0, 0, 128, 18, 0x0010);
    tft.setTextColor(COL_HEAD);
    tft.setCursor(4, 5);
    tft.print(MAIN_LABELS_EN[mainCursor]);
    // No "< BACK" text — removed per request
    tft.drawLine(0, 18, 128, 18, COL_DIM);

    int total   = SUB_ITEMS_COUNT[mainCursor];
    int visRows = min(total, 7);
    int scrollOff = 0;
    if (subCursor >= visRows) scrollOff = subCursor - visRows + 1;

    for (int i = 0; i < visRows; i++) {
      int ri   = i + scrollOff;
      if (ri >= total) break;
      int yRow = 22 + i * 18;
      bool sel = (ri == subCursor);

      if (sel) {
        tft.fillRect(2, yRow, 124, 16, COL_SEL_BG);
        tft.drawLine(2, yRow, 2, yRow+15, COL_ACCENT);
        tft.setTextColor(COL_FG);
      } else {
        tft.setTextColor(COL_DIM);
      }
      tft.setCursor(8, yRow+4);
      tft.print(String(ri+1) + ". " + getSubmenuItemText(mainCursor, ri));
    }

    // Scroll indicator bar
    if (total > visRows) {
      for (int i = 0; i < total; i++) {
        uint16_t dc = (i == subCursor) ? COL_ACCENT : COL_DIM;
        tft.fillRect(124, 22 + i*(116/total), 3, max(2, 114/total - 1), dc);
      }
    }
    // No footer hint text
  }

  // ── Level 2: mode select ──────────────────────────────────
  else {
    tft.fillRect(0, 0, 128, 18, 0x0010);
    tft.setTextColor(COL_PURPLE);
    tft.setCursor(4, 5);
    tft.print("SELECT MODE");
    tft.drawLine(0, 18, 128, 18, COL_DIM);

    tft.setTextColor(COL_GOLD);
    tft.setCursor(8, 26);
    tft.print(getSubmenuItemText(mainCursor, subCursor));
    tft.drawLine(8, 36, 120, 36, COL_DIM);

    // Mode labels — keep description on one line, indent second line 4px
    const char* modeLabel_en[] = { "Readout & Log", "Live Graph" };
    // const char* modeLabel_cz[] = { "Hodnoty & Log", "Zivy Graf"  };
    const char* modeDesc_en[]  = { "Value + 4 laps", "Scrolling plot" };
    // const char* modeDesc_cz[]  = { "Hodnota+zaznamy","Posuvny graf"  };

    for (int i = 0; i < 2; i++) {
      int yRow = 44 + i * 46;
      bool sel = (i == optCursor);
      if (sel) {
        tft.fillRect(2, yRow-2, 124, 42, COL_SEL_BG);
        tft.drawRect(2, yRow-2, 124, 42, COL_ACCENT);
        tft.setTextColor(COL_FG);
      } else {
        tft.setTextColor(COL_DIM);
      }
      tft.setCursor(8, yRow+4);
      tft.print(String(i+1) + ". ");
      tft.print(modeLabel_en[i]);
      tft.setTextColor(sel ? COL_ACCENT : 0x2945);
      tft.setCursor(12, yRow+18);   // 4px indent so it reads as sub-text
      tft.print(modeDesc_en[i]);
    }
    // No footer
  }
}

// ── runApp dispatcher ─────────────────────────────────────────
void runApp() {
  if (mainCursor == 0) {
    if      (subCursor == 0) runAllTogetherSensor();
    else                     runSensorApp(subCursor, optCursor);

  } else if (mainCursor == 1) {
    if      (subCursor == 0) runToneGenerator();
    else if (subCursor == 1) runPeriodicTable();
    else if (subCursor == 2) runConstants();
    else if (subCursor == 3) runUnitConverter();
    else if (subCursor == 4) runCalculator();
    else if (subCursor == 5) runWiFiScanner();
    else if (subCursor == 6) runBluetoothScanner();
    else if (subCursor == 7) runStopwatch();
    else if (subCursor == 8) runTimerApp();

  } else if (mainCursor == 2) {
    if      (subCursor == 0) runSimonSays();
    else if (subCursor == 1) runChromeDino();
    else if (subCursor == 2) runSnakeGame();
    else if (subCursor == 3) runFlappyBird();

  } else if (mainCursor == 3) {
    runSettings();
  }
}
