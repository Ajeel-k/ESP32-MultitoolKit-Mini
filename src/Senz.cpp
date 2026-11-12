/*
 * ============================================================
 *  Senz.ino v3  —  Sensor applications
 *  Temperature / Light / Distance / All-Together
 *  (MPU6050 fully removed — VL53L0X TOF only for I2C)
 * ============================================================
 */

 #include "main.hpp"
 #include "Menu.hpp"

// ── Persistent lap storage ────────────────────────────────────
float savedLaps[4] = {0, 0, 0, 0};
int   lapCount     = 0;

// ── Graph ring buffer ─────────────────────────────────────────
// Each sample slot stores a mapped Y pixel.
// Buffer holds up to 512 samples; window controls how many are shown.
#define GRAPH_BUF   512
int16_t graphBuf[GRAPH_BUF];
float   graphRawBuf[GRAPH_BUF];   // raw float values for cursor readout
int     graphBufIdx = 0;          // write head

// graphWindowSec: how many seconds the full X axis represents.
// Sample interval is fixed at ~200 ms, so samples = windowSec * 5.
int graphWindowSec = 30;          // default 30 s; range 5..300

// ── Sensor metadata ───────────────────────────────────────────
struct SensorMeta {
  const char* nameEN;
  // const char* nameCZ;
  const char* unit;
  float       rangeMin;
  float       rangeMax;
};

// idx: 1=Temp  2=Light  3=Distance
const SensorMeta SENSOR_INFO[] = {
  { "",            "",     0,    0 },
  { "Temperature", "C",  -10,   60 },
  { "Light",       "%",    0,  100 },
  { "Distance",    "cm",   0,  200 },
};

// ── Thermistor (pull-down) ────────────────────────────────────
float readTemperatureCelsius() {
  int raw = analogRead(PIN_NTC);
  if (raw <= 10 || raw >= 4085) return -99.0f;
  float vadc = raw * (3.3f / 4095.0f);
  float rNtc = NTC_SERIES * (3.3f - vadc) / vadc;
  float tK   = 1.0f / (1.0f/NTC_T0 + (1.0f/NTC_BETA) * log(rNtc/NTC_R0));
  float offset = prefs.getFloat("tempoff", 0.0f);
  return (tK - 273.15f) + offset;
}

// ── Master sensor reader ──────────────────────────────────────
float fetchSensorValue(int type) {
  switch (type) {
    case 1: return readTemperatureCelsius();
    case 2: return (analogRead(PIN_LDR) / 4095.0f) * 100.0f;
    case 3: {
      if (!loxAvailable) return -1.0f;
      VL53L0X_RangingMeasurementData_t m;
      lox.getRangingMeasurement(&m, false);
      return (m.RangeStatus != 4) ? m.RangeMilliMeter / 10.0f : -1.0f;
    }
  }
  return 0.0f;
}

String sensorName(int t) { return String(SENSOR_INFO[t].nameEN); }
String sensorUnit(int t) { return String(SENSOR_INFO[t].unit); }

// ── Map float to Y pixel (graph area: y=23..139) ─────────────
int16_t valToY(float val, int type) {
  return (int16_t)constrain(
    map((long)(val * 100),
        (long)(SENSOR_INFO[type].rangeMin * 100),
        (long)(SENSOR_INFO[type].rangeMax * 100),
        139, 23),
    23, 139);
}

// ── Draw static graph frame ───────────────────────────────────
void drawGraphFrame(int type) {
  // Y axis with labels
  tft.drawLine(18, 22, 18, 140, COL_DIM);
  tft.drawLine(18, 140, 127, 140, COL_DIM);
  tft.setTextColor(COL_DIM); tft.setTextSize(1);
  // Max label
  tft.setCursor(0, 23);
  tft.print((int)SENSOR_INFO[type].rangeMax);
  // Min label
  tft.setCursor(0, 132);
  tft.print((int)SENSOR_INFO[type].rangeMin);
}

// ── Redraw entire waveform from buffer ────────────────────────
void redrawGraph(int type, int totalSamples, int windowSamples, int writeHead) {
  tft.fillRect(19, 23, 109, 117, COL_BG);
  if (totalSamples < 2) return;

  int pts   = min(totalSamples, windowSamples);
  int plotW = 109;

  for (int i = 1; i < pts; i++) {
    int idx0 = ((writeHead - pts + i - 1) + GRAPH_BUF) % GRAPH_BUF;
    int idx1 = ((writeHead - pts + i    ) + GRAPH_BUF) % GRAPH_BUF;
    int x0   = 19 + (i-1) * plotW / max(windowSamples-1, 1);
    int x1   = 19 +  i    * plotW / max(windowSamples-1, 1);
    tft.drawLine(x0, graphBuf[idx0], x1, graphBuf[idx1], COL_ACCENT);
  }

  // Redraw axes (fill wiped them)
  drawGraphFrame(type);
}

// ── runSensorApp (Readout or Graph) ───────────────────────────
void runSensorApp(int type, int mode) {
  tft.fillScreen(COL_BG);
  tft.setTextSize(1);

  memset(graphBuf, 0, sizeof(graphBuf));
  memset(graphRawBuf, 0, sizeof(graphRawBuf));
  graphBufIdx = 0;

  String hdr = sensorName(type) + " [" + sensorUnit(type) + "]";
  drawHeader(hdr.c_str(), COL_HEAD);

  if (mode == 0) {
    // ── Readout mode ──────────────────────────────────────────
    tft.setTextColor(COL_DIM);
    tft.setCursor(4, 22); tft.print("LIVE:");
    tft.drawLine(0, 64, 128, 64, COL_DIM);
    tft.setTextColor(COL_DIM);
    tft.setCursor(4, 68); tft.print("LAPS  (SEL=save)");
    for (int i = 0; i < 4; i++) {
      tft.setTextColor(COL_DIM);
      tft.setCursor(4, 82 + i*16);
      tft.print("L" + String(i+1) + ":");
    }

    static unsigned long lastSel = 0;
    while (true) {
      if (btnBack()) { returnToMenu(1); return; }

      float val   = fetchSensorValue(type);
      String vStr = (val <= -90.0f) ? "ERR" : (val == -1.0f) ? "N/A" : String(val, 2);

      tft.setTextSize(3);
      patchText(4, 32, 124, 28, vStr, COL_OK);
      tft.setTextSize(1);
      tft.setTextColor(COL_ACCENT);
      tft.setCursor(4, 56); tft.print(sensorUnit(type));

      if (btnSel() && (millis()-lastSel > 300)) {
        lastSel = millis();
        playClick();
        savedLaps[lapCount % 4] = (val < -90.0f || val == -1.0f) ? 0 : val;
        lapCount++;
        for (int i = 0; i < 4; i++) {
          patchText(22, 82+i*16, 106, 12,
            "L"+String(i+1)+": "+String(savedLaps[i],2)+" "+sensorUnit(type), COL_GOLD);
        }
      }
      delay(120);
    }

  } else {
    // ── Graph mode ────────────────────────────────────────────
    drawGraphFrame(type);

    // Window label at bottom-left
    // X-axis time label
    tft.setTextColor(COL_DIM);
    tft.setCursor(19, 148); tft.print("0s");

    bool          paused        = false;
    int           cursorPos     = 0;   // cursor position index within buffer (for paused scrub)
    unsigned long lastSample    = 0;
    unsigned long sampleInterval= 200; // ms per sample
    int           totalSamples  = 0;
    int           displayTick   = 0;   // throttle the live value display
    unsigned long pauseTime     = 0;   // elapsed ms when pause was hit

    // Helper: draw window duration at bottom
    auto drawWindowLabel = [&]() {
      tft.fillRect(0, 148, 128, 12, COL_BG);
      tft.setTextColor(COL_DIM); tft.setCursor(19, 148);
      tft.print("0s");
      tft.setCursor(80, 148);
      tft.print(String(graphWindowSec) + "s");
      tft.setTextColor(paused ? COL_WARN : COL_OK);
      tft.setCursor(46, 148);
      tft.print(paused ? "PAUSED" : "LIVE  ");
    };
    drawWindowLabel();

    static unsigned long lastSelG = 0;
    static unsigned long lastUpG  = 0;
    static unsigned long lastDnG  = 0;

    while (true) {
      if (btnBack()) { returnToMenu(1); return; }

      unsigned long now = millis();
      int windowSamples = graphWindowSec * (1000 / sampleInterval);
      if (windowSamples < 2) windowSamples = 2;

      // SEL = toggle pause
      if (btnSel() && now - lastSelG > 300) {
        lastSelG = now;
        paused   = !paused;
        if (paused) {
          cursorPos = 0; // cursor at right edge (newest data)
        }
        playClick();
        drawWindowLabel();
      }

      if (!paused) {
        // UP / DN change window width (seconds) when live
        if (btnUp() && now - lastUpG > 150) {
          lastUpG        = now;
          graphWindowSec = min(graphWindowSec + (graphWindowSec < 60 ? 5 : 30), 300);
          drawWindowLabel();
          redrawGraph(type, totalSamples, windowSamples, graphBufIdx);
          delay(10);
        }
        if (btnDown() && now - lastDnG > 150) {
          lastDnG        = now;
          graphWindowSec = max(graphWindowSec - (graphWindowSec <= 60 ? 5 : 30), 5);
          drawWindowLabel();
          redrawGraph(type, totalSamples, windowSamples, graphBufIdx);
          delay(10);
        }

        // Sample
        if (now - lastSample >= sampleInterval) {
          lastSample = now;
          float val  = fetchSensorValue(type);

          graphBuf[graphBufIdx]    = valToY(val < -90.0f ? 0 : val, type);
          graphRawBuf[graphBufIdx] = val;
          graphBufIdx              = (graphBufIdx + 1) % GRAPH_BUF;
          totalSamples             = min(totalSamples + 1, GRAPH_BUF);

          redrawGraph(type, totalSamples, windowSamples, graphBufIdx);

          // Slow display update: only every 10th sample
          displayTick++;
          if (displayTick >= 10) {
            displayTick = 0;
            patchText(22, 4, 86, 10,
              (val <= -90.0f || val == -1.0f) ? "N/A" : (String(val,1)+" "+sensorUnit(type)),
              COL_FG);
          }
        }

      } else {
        // ── Paused: scrub cursor with UP/DN ──────────────────
        int maxCursor = min(totalSamples, windowSamples) - 1;
        if (maxCursor < 0) maxCursor = 0;

        if (btnUp() && now - lastUpG > 150) {
          lastUpG   = now;
          cursorPos = min(cursorPos + 1, maxCursor);
        }
        if (btnDown() && now - lastDnG > 150) {
          lastDnG   = now;
          cursorPos = max(cursorPos - 1, 0);
        }

        // Compute dashed cursor X position
        int pts   = min(totalSamples, windowSamples);
        int plotW = 109;
        // cursorPos=0 = rightmost (newest), maxCursor = leftmost
        int cursorScreenIdx = pts - 1 - cursorPos; // 0..pts-1
        int cx = 19 + cursorScreenIdx * plotW / max(pts-1, 1);

        // Redraw graph with cursor line
        redrawGraph(type, totalSamples, windowSamples, graphBufIdx);

        // Draw dashed vertical cursor line
        for (int y = 23; y < 140; y += 4) {
          tft.drawPixel(cx, y, COL_GOLD);
        }

        // Read value and time at cursor
        int bufAt = ((graphBufIdx - 1 - cursorPos) + GRAPH_BUF) % GRAPH_BUF;
        float rawVal  = graphRawBuf[bufAt];
        // Time: each sample = sampleInterval ms, cursorPos samples back from now
        float timeSec = cursorPos * sampleInterval / 1000.0f;

        // Display cursor info at bottom
        tft.fillRect(0, 148, 128, 12, COL_BG);
        tft.setTextColor(COL_GOLD);
        tft.setCursor(0, 148);
        char cbuf[28];
        if (rawVal > -90.0f && rawVal != -1.0f) {
          snprintf(cbuf, sizeof(cbuf), "t=%.1fs  v=%.1f%s",
                   timeSec, rawVal, SENSOR_INFO[type].unit);
        } else {
          snprintf(cbuf, sizeof(cbuf), "t=%.1fs  v=N/A", timeSec);
        }
        tft.print(cbuf);

        delay(30);
      }
    }
  }
}

// ── All-Together multi-sensor dashboard + graph ───────────────
void runAllTogetherSensor() {
  // Offer choice: dashboard or multi-line graph
  tft.fillScreen(COL_BG);
  drawHeader("ALL SENSORS", COL_HEAD);

  const char* opts_en[] = { "Dashboard", "Multi-Graph" };
  // const char* opts_cz[] = { "Prehled",   "Graf"        };
  int sel = 0;
  int moved = 1;

  // Simple 2-option picker
  while (true) {
    if(moved == 1)
      for (int i = 0; i < 2; i++) {
        int yy = 40 + i * 40;
        bool s = (i == sel);
        tft.fillRect(2, yy, 124, 32, s ? COL_SEL_BG : COL_BG);
        if (s) tft.drawRect(2, yy, 124, 32, COL_ACCENT);
        tft.setTextColor(s ? COL_FG : COL_DIM);
        tft.setCursor(10, yy+10);
        tft.print(opts_en[i]);
        moved = 0;
      }
    // tft.setTextColor(COL_DIM);
    // tft.setCursor(4, 140); tft.print("UP/DN SEL=open BACK");
  
    if (btnUp()  ) { sel = (sel > 0) ? sel-1 : 1; playNav(); delay(200); moved = 1;}
    if (btnDown()) { sel = (sel < 1) ? sel+1 : 0; playNav(); delay(200); moved = 1;}
    if (btnBack()) { returnToMenu(1); return; }
    if (btnSel())  { playClick(); delay(180); break; }
    delay(20);
  }

  if (sel == 0) {
    // ── Dashboard ─────────────────────────────────────────────
    tft.fillScreen(COL_BG);
    drawHeader("ALL SENSORS", COL_HEAD);

    // 3 sensors, each gets a row
    const char* lbl[]  = { "TEMP", "LIGHT", "DIST" };
    const char* unit[] = { "C",    "%",     "CM"   };
    uint16_t    col[]  = { COL_WARN, COL_GOLD, COL_ACCENT };

    for (int i = 0; i < 3; i++) {
      tft.setTextColor(col[i]);
      tft.setCursor(4, 26 + i*40);
      tft.print(lbl[i]);
      tft.setTextColor(COL_DIM);
      tft.drawLine(0, 37 + i*40, 128, 37 + i*40, 0x1082);
    }

    while (!btnBack()) {
      float temp  = readTemperatureCelsius();
      float light = (analogRead(PIN_LDR) / 4095.0f) * 100.0f;
      float dist  = -1.0f;
      if (loxAvailable) {
        VL53L0X_RangingMeasurementData_t m;
        lox.getRangingMeasurement(&m, false);
        if (m.RangeStatus != 4) dist = m.RangeMilliMeter / 10.0f;
      }
      float vals[] = { temp, light, dist };
      for (int i = 0; i < 3; i++) {
        String s;
        if      (vals[i] <= -90.0f) s = "ERR";
        else if (vals[i] == -1.0f)  s = "N/A";
        else                        s = String(vals[i], 1);
        patchText(60, 26 + i*40, 68, 9, s, col[i]);
        if(s != "ERR" && s != "N/A")
          tft.print(unit[i]);
      }
      // TOF status badge
      tft.setTextColor(loxAvailable ? COL_OK : COL_DIM);
      tft.setCursor(100, 148); tft.print("TOF");
      delay(150);
    }

  } else {
    // ── Multi-line graph ──────────────────────────────────────
    tft.fillScreen(COL_BG);
    drawHeader("MULTI-GRAPH", COL_HEAD);

    // Draw axes
    tft.drawLine(0, 22, 0, 139, COL_DIM);
    tft.drawLine(0, 139, 127, 139, COL_DIM);

    // 3 ring buffers, one per sensor, mapped 23..139
    const int MB = 128;
    int16_t mbuf[3][MB];
    memset(mbuf, 0, sizeof(mbuf));
    int mIdx = 0, mTotal = 0;

    uint16_t mcol[3] = { COL_WARN, COL_GOLD, COL_ACCENT };
    const char* ml[] = { "T","L","D" };

    // Legend
    for (int i = 0; i < 3; i++) {
      tft.setTextColor(mcol[i]);
      tft.setCursor(4 + i*40, 148);
      tft.print(ml[i]);
    }

    unsigned long lastMs = 0;
    while (!btnBack()) {
      if (millis() - lastMs < 200) { delay(10); continue; }
      lastMs = millis();

      float vals[3];
      vals[0] = readTemperatureCelsius();
      vals[1] = (analogRead(PIN_LDR) / 4095.0f) * 100.0f;
      vals[2] = -1.0f;
      if (loxAvailable) {
        VL53L0X_RangingMeasurementData_t m;
        lox.getRangingMeasurement(&m, false);
        if (m.RangeStatus != 4) vals[2] = m.RangeMilliMeter / 10.0f;
      }

      // Each sensor mapped independently to 23..139
      float rMin[3] = {-10, 0,   0};
      float rMax[3] = { 60, 100, 200};
      for (int s = 0; s < 3; s++) {
        float v = vals[s];
        if (v < -90.0f || v < 0) v = 0;
        mbuf[s][mIdx] = (int16_t)constrain(
          map((long)(v*10),(long)(rMin[s]*10),(long)(rMax[s]*10),139,23),23,139);
      }
      mIdx   = (mIdx + 1) % MB;
      mTotal = min(mTotal + 1, MB);

      // Clear plot area
      tft.fillRect(1, 23, 127, 116, COL_BG);

      // Draw 3 lines
      int pts = min(mTotal, 128);
      for (int s = 0; s < 3; s++) {
        for (int i = 1; i < pts; i++) {
          int i0 = ((mIdx - pts + i - 1) + MB) % MB;
          int i1 = ((mIdx - pts + i    ) + MB) % MB;
          int x0 = 1 + (i-1) * 126 / max(pts-1,1);
          int x1 = 1 +  i    * 126 / max(pts-1,1);
          tft.drawLine(x0, mbuf[s][i0], x1, mbuf[s][i1], mcol[s]);
        }
      }

      // Redraw axes
      tft.drawLine(0, 22, 0, 139, COL_DIM);
      tft.drawLine(0, 139, 127, 139, COL_DIM);

      // Live values top-right
      // patchText(75, 4, 58, 10,
      //   String(vals[0],1)+"C "+String(vals[1],0)+"%", COL_FG);
    }
  }

  returnToMenu(1);
}
