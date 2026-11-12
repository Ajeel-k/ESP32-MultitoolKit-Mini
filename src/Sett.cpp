/*
 * ============================================================
 * Sett.ino  —  Settings application
 * Language / Simon Speed / Sound / Brightness / Temp Offset / Colours
 * ============================================================
 */

 #include "main.hpp"
 #include "Menu.hpp"
 
void runSettings() {
  int  cursor  = (subCursor < 6) ? subCursor : 0;
  bool needRd  = true;

  const int SETT_COUNT = 5;

  auto drawSettings = [&]() {
    tft.fillScreen(COL_BG);
    drawHeader("SETTINGS", COL_HEAD);

    // Read live values
    float tempOff = prefs.getFloat("tempoff", 0.0f);
    int currentTheme = prefs.getInt("theme", 0);

    // Theme names for display
    const char* themeNamesEN[] = {"Default", "Synthwave", "Hacker", "Fire"};
    // const char* themeNamesCZ[] = {"Vychozi", "Synthwave", "Hacker", "Ohen"};

    struct {
      const char* nameEN;
      // const char* nameCZ;
      String      value;
      const char* hintEN;
      // const char* hintCZ;
    } rows[SETT_COUNT] = {
      // {
      //   "Language",   "Jazyk",
      //   isCzech ? "Cestina" : "English",
      //   "SEL=toggle", "SEL=prepnout"
      // },
      {
        "Simon Speed",
        String(simonSpeed) + " ms",
        "SEL=cycle 250>50"
      },
      {
        "Sound",
        soundOn ? "Enabled" : "Disabled",
        "SEL=toggle"
      },
      {
        "Brightness",
        String(brightness),
        "SEL=cycle 255>50"
      },
      {
        "Temp Offset",
        (tempOff >= 0 ? "+" : "") + String(tempOff, 1) + " C",
        "UP/DN=fine SEL=reset"
      },
      {
        "Colours",
        themeNamesEN[currentTheme],
        "SEL=cycle themes"
      }
    };

    for (int i = 0; i < SETT_COUNT; i++) {
      // Adjusted Y spacing from 26 to 22 to fit 6 items neatly on screen
      int yy = 21 + i * 22; 
      bool sel = (i == cursor);

      if (sel) {
        tft.fillRect(2, yy - 2, 124, 20, COL_SEL_BG);
        tft.drawLine(2, yy-2, 2, yy+17, COL_ACCENT);
      }

      // Name
      tft.setTextColor(sel ? COL_FG : COL_DIM);
      tft.setCursor(8, yy + 1);
      tft.print(rows[i].nameEN);

      // Value (right-aligned feel)
      tft.setTextColor(sel ? COL_GOLD : 0x2945);
      tft.setCursor(8, yy + 10);
      tft.print("> ");
      tft.print(rows[i].value);

      // Hint for selected row
      if (sel) {
        tft.setTextColor(COL_ACCENT);
        int hLen = strlen(rows[i].hintEN) * 6;
        tft.setCursor(128 - hLen - 2, yy + 1);
        tft.print(rows[i].hintEN);
      }
    }

    // Footer
    tft.drawLine(0, 156, 128, 156, COL_DIM);
    tft.setTextColor(COL_DIM);
    tft.setCursor(4, 158);
    tft.print("UP/DN=navigate  BACK=menu");
  };

  while (true) {
    if (needRd) { drawSettings(); needRd = false; }

    float tempOff = prefs.getFloat("tempoff", 0.0f);

    if (btnUp()) {
      playNav();
      if (cursor == 4) {
        // Fine-tune temp offset [cite: 19]
        tempOff = constrain(tempOff + 0.5f, -15.0f, 15.0f);
        prefs.putFloat("tempoff", tempOff);
      } else {
        cursor = (cursor > 0) ? cursor - 1 : SETT_COUNT - 1;
      }
      delay(140); needRd = true;
    }

    if (btnDown()) {
      playNav();
      if (cursor == 4) {
        // Fine-tune temp offset [cite: 23]
        tempOff = constrain(tempOff - 0.5f, -15.0f, 15.0f);
        prefs.putFloat("tempoff", tempOff);
      } else {
        cursor = (cursor < SETT_COUNT - 1) ? cursor + 1 : 0;
      }
      delay(140); needRd = true;
    }

    if (btnSel()) {
      playClick();
      switch (cursor) {
        case 0:
          simonSpeed = (simonSpeed <= 50) ? 250 : simonSpeed - 50;
          prefs.putInt("simon", simonSpeed);
          break;
        case 1:
          soundOn = !soundOn;
          prefs.putBool("sound", soundOn);
          // Play a test beep if turning on [cite: 30]
          if (soundOn) playTone(880, 80);
          break;
        case 2:
          brightness = (brightness <= 50) ? 255 : brightness - 50;
          prefs.putInt("bright", brightness);
          break;
        case 3:
          // Reset offset to zero [cite: 33]
          prefs.putFloat("tempoff", 0.0f);
          break;
        case 4:
          // Cycle through 4 color themes
          int currentTheme = prefs.getInt("theme", 0);
          currentTheme = (currentTheme + 1) % 4;
          prefs.putInt("theme", currentTheme);
          
          // Apply theme colors globally and save to NVS 
          if (currentTheme == 0) { // Default (Cyan/Orange)
            COL_ACCENT = 0x07FF; COL_GOLD = 0xFEA0; COL_OK = 0x07E0; COL_HEAD = 0xFD20;
          } else if (currentTheme == 1) { // Synthwave (Magenta/Cyan)
            COL_ACCENT = 0xF81F; COL_GOLD = 0x07FF; COL_OK = 0x07FF; COL_HEAD = 0xF81F;
          } else if (currentTheme == 2) { // Hacker (All Green)
            COL_ACCENT = 0x07E0; COL_GOLD = 0x07E0; COL_OK = 0x07E0; COL_HEAD = 0x07E0;
          } else if (currentTheme == 3) { // Fire (Red/Orange)
            COL_ACCENT = 0xF800; COL_GOLD = 0xFEA0; COL_OK = 0xFD20; COL_HEAD = 0xF800;
          }
          
          prefs.putUInt("colAccent", COL_ACCENT);
          prefs.putUInt("colGold",   COL_GOLD);
          prefs.putUInt("colOk",     COL_OK);
          prefs.putUInt("colHead",   COL_HEAD);
          break;
      }
      delay(200); needRd = true;
    }

    if (btnBack()) {
      subCursor = cursor; // save cursor position [cite: 36]
      playBack();
      returnToMenu(0);
      return;
    }

    delay(20);
  }
}