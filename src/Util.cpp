/*
 * ============================================================
 *  Util.ino v3
 * ============================================================
 */

 #include "main.hpp"
 #include "Menu.hpp"

// ════════════════════════════════════════════════════════════
//  TONE GENERATOR  (unchanged from v2)
// ════════════════════════════════════════════════════════════
void runToneGenerator() {
  tft.fillScreen(COL_BG);
  drawHeader("TONE GENERATOR", COL_HEAD);

  int  freq = 440, noteIdx = 0;
  bool modeNotes = false, toneOn = false;

  const int    nFreqs[] = {262,277,294,311,330,349,370,392,415,440,466,494,
                            523,554,587,622,659,698,740,784,831,880,932,988};
  const char*  nNames[] = {"C4","C#4","D4","D#4","E4","F4","F#4","G4","G#4",
                            "A4","A#4","B4","C5","C#5","D5","D#5","E5","F5",
                            "F#5","G5","G#5","A5","A#5","B5"};
  const int NOTE_COUNT = 24;

  tft.setTextColor(COL_DIM);
  tft.setCursor(4, 22); tft.print("SEL=mode  BACK=exit");
  tft.drawLine(0, 31, 128, 31, COL_DIM);
  tft.setCursor(4, 118); tft.print("UP/DN = "); tft.print("change");

  auto redraw = [&]() {
    tft.fillRect(0, 32, 128, 84, COL_BG);
    if (modeNotes) {
      tft.setTextColor(COL_DIM); tft.setCursor(4,40); tft.print("NOTE MODE");
      tft.setTextSize(2);
      patchText(10,54,108,22,String(nNames[noteIdx]),COL_ACCENT);
      tft.setTextSize(1);
      patchText(10,82,108,12,String(nFreqs[noteIdx])+" Hz",COL_GOLD);
    } else {
      tft.setTextColor(COL_DIM); tft.setCursor(4,40); tft.print("FREQ MODE");
      tft.setTextSize(2);
      patchText(10,54,108,22,String(freq)+" Hz",COL_OK);
      tft.setTextSize(1);
      float T = (freq>0)?(1000.0f/freq):0;
      patchText(10,82,108,12,"T="+String(T,2)+"ms",COL_GOLD);
    }
    patchText(4,100,120,12,
      toneOn?(">> PLAYING <<"):("-- ticho --"),
      toneOn?COL_WARN:COL_DIM);
  };
  redraw();

  while (true) {
    if (btnBack()) { noTone(PIN_BUZZ); returnToMenu(1); return; }
    bool ch=false;
    if (btnSel()) { modeNotes=!modeNotes; toneOn=false; noTone(PIN_BUZZ); delay(200); ch=true; }
    if (btnUp())  { if(modeNotes){noteIdx=(noteIdx<NOTE_COUNT-1)?noteIdx+1:0;}
                    else{freq=min(freq+(freq<1000?10:50),20000);}  delay(80); ch=true; }
    if (btnDown()){ if(modeNotes){noteIdx=(noteIdx>0)?noteIdx-1:NOTE_COUNT-1;}
                    else{freq=max(freq-(freq<=1000?10:50),20);}    delay(80); ch=true; }
    if (ch) { int f=modeNotes?nFreqs[noteIdx]:freq; toneOn=true; tone(PIN_BUZZ,f); redraw(); }
    delay(30);
  }
}

// ════════════════════════════════════════════════════════════
//  PERIODIC TABLE  (full 118 elements — from uploaded v2)
// ════════════════════════════════════════════════════════════
struct ChemElement {
  const char* sym;
  const char* name;
  float       dens;
  int         year;
  int         atomicNum;
  const char* chg;
  const char* group;
  const char* desc;
};

const ChemElement ELEMENTS[] PROGMEM = {
  {"H",  "Hydrogen",    0.09,  1766,  1,"+1","Nonmetal",  "Fuel, water"},
  {"He", "Helium",      0.18,  1868,  2,"0", "Noble Gas", "Balloons"},
  {"Li", "Lithium",     0.53,  1817,  3,"+1","Alkali",    "Batteries"},
  {"Be", "Beryllium",   1.85,  1798,  4,"+2","Alkaline",  "Aerospace"},
  {"B",  "Boron",       2.34,  1808,  5,"+3","Metalloid", "Glass"},
  {"C",  "Carbon",      2.26,     0,  6,"+4","Nonmetal",  "Life,fuel"},
  {"N",  "Nitrogen",    1.25,  1772,  7,"-3","Nonmetal",  "Fertiliser"},
  {"O",  "Oxygen",      1.43,  1774,  8,"-2","Nonmetal",  "Breathing"},
  {"F",  "Fluorine",    1.70,  1886,  9,"-1","Halogen",   "PTFE,teeth"},
  {"Ne", "Neon",        0.90,  1898, 10,"0", "Noble Gas", "Lighting"},
  {"Na", "Sodium",      0.97,  1807, 11,"+1","Alkali",    "Salt,nerves"},
  {"Mg", "Magnesium",   1.74,  1755, 12,"+2","Alkaline",  "Alloys"},
  {"Al", "Aluminium",   2.70,  1825, 13,"+3","Metal",     "Cans,planes"},
  {"Si", "Silicon",     2.33,  1824, 14,"+4","Metalloid", "Chips,glass"},
  {"P",  "Phosphorus",  1.82,  1669, 15,"-3","Nonmetal",  "DNA,matches"},
  {"S",  "Sulfur",      2.07,     0, 16,"-2","Nonmetal",  "Acid,rubber"},
  {"Cl", "Chlorine",    3.21,  1774, 17,"-1","Halogen",   "Pools,bleach"},
  {"Ar", "Argon",       1.78,  1894, 18,"0", "Noble Gas", "Welding"},
  {"K",  "Potassium",   0.86,  1807, 19,"+1","Alkali",    "Fertiliser"},
  {"Ca", "Calcium",     1.54,  1808, 20,"+2","Alkaline",  "Bones,cement"},
  {"Sc", "Scandium",    2.99,  1879, 21,"+3","Transition","Bikes,alloys"},
  {"Ti", "Titanium",    4.51,  1791, 22,"+4","Transition","Implants"},
  {"V",  "Vanadium",    6.11,  1801, 23,"+5","Transition","Steel tools"},
  {"Cr", "Chromium",    7.19,  1797, 24,"+3","Transition","Stainless"},
  {"Mn", "Manganese",   7.43,  1774, 25,"+2","Transition","Batteries"},
  {"Fe", "Iron",        7.87,-3000,  26,"+3","Transition","Steel"},
  {"Co", "Cobalt",      8.90,  1735, 27,"+2","Transition","Magnets"},
  {"Ni", "Nickel",      8.91,  1751, 28,"+2","Transition","Coins"},
  {"Cu", "Copper",      8.96,-8000,  29,"+2","Transition","Wire"},
  {"Zn", "Zinc",        7.14,-1000,  30,"+2","Transition","Galvanising"},
  {"Ga", "Gallium",     5.91,  1875, 31,"+3","Metal",     "LEDs,semicon"},
  {"Ge", "Germanium",   5.32,  1886, 32,"+4","Metalloid", "Fiber optics"},
  {"As", "Arsenic",     5.73,  1250, 33,"-3","Metalloid", "LEDs"},
  {"Se", "Selenium",    4.82,  1817, 34,"-2","Nonmetal",  "Photocells"},
  {"Br", "Bromine",     3.10,  1826, 35,"-1","Halogen",   "Flame ret."},
  {"Kr", "Krypton",     3.75,  1898, 36,"0", "Noble Gas", "Lasers"},
  {"Rb", "Rubidium",    1.53,  1861, 37,"+1","Alkali",    "Atomic clock"},
  {"Sr", "Strontium",   2.63,  1790, 38,"+2","Alkaline",  "Fireworks"},
  {"Y",  "Yttrium",     4.47,  1794, 39,"+3","Transition","Lasers"},
  {"Zr", "Zirconium",   6.52,  1789, 40,"+4","Transition","Nuclear"},
  {"Nb", "Niobium",     8.57,  1801, 41,"+5","Transition","Supercond."},
  {"Mo", "Molybdenum", 10.20,  1778, 42,"+6","Transition","Hard steel"},
  {"Tc", "Technetium", 11.50,  1937, 43,"+7","Transition","Med.imaging"},
  {"Ru", "Ruthenium",  12.40,  1844, 44,"+3","Transition","Contacts"},
  {"Rh", "Rhodium",    12.40,  1803, 45,"+3","Transition","Catalysts"},
  {"Pd", "Palladium",  12.00,  1803, 46,"+2","Transition","Cat.conv."},
  {"Ag", "Silver",     10.50,-5000,  47,"+1","Transition","Mirrors"},
  {"Cd", "Cadmium",     8.65,  1817, 48,"+2","Transition","Batteries"},
  {"In", "Indium",      7.31,  1863, 49,"+3","Metal",     "Touchscreens"},
  {"Sn", "Tin",         7.31,-3500,  50,"+4","Metal",     "Solder"},
  {"Sb", "Antimony",    6.69,-1000,  51,"-3","Metalloid", "Batteries"},
  {"Te", "Tellurium",   6.24,  1782, 52,"-2","Metalloid", "Solar"},
  {"I",  "Iodine",      4.93,  1811, 53,"-1","Halogen",   "Disinfect."},
  {"Xe", "Xenon",       5.90,  1898, 54,"0", "Noble Gas", "Flash lamps"},
  {"Cs", "Cesium",      1.93,  1860, 55,"+1","Alkali",    "Atomic clock"},
  {"Ba", "Barium",      3.51,  1808, 56,"+2","Alkaline",  "X-ray"},
  {"La", "Lanthanum",   6.15,  1839, 57,"+3","Lanthanide","Camera lens"},
  {"Ce", "Cerium",      6.77,  1803, 58,"+3","Lanthanide","Lighters"},
  {"Pr", "Praseodym.",  6.77,  1885, 59,"+3","Lanthanide","Goggles"},
  {"Nd", "Neodymium",   7.01,  1885, 60,"+3","Lanthanide","Magnets"},
  {"Pm", "Promethium",  7.26,  1945, 61,"+3","Lanthanide","Pacemakers"},
  {"Sm", "Samarium",    7.52,  1879, 62,"+3","Lanthanide","Lasers"},
  {"Eu", "Europium",    5.24,  1896, 63,"+3","Lanthanide","Phosphors"},
  {"Gd", "Gadolinium",  7.90,  1880, 64,"+3","Lanthanide","MRI"},
  {"Tb", "Terbium",     8.23,  1843, 65,"+3","Lanthanide","Phosphors"},
  {"Dy", "Dysprosium",  8.55,  1886, 66,"+3","Lanthanide","Lasers"},
  {"Ho", "Holmium",     8.80,  1878, 67,"+3","Lanthanide","Med.lasers"},
  {"Er", "Erbium",      9.07,  1842, 68,"+3","Lanthanide","Fiber opt."},
  {"Tm", "Thulium",     9.32,  1879, 69,"+3","Lanthanide","X-rays"},
  {"Yb", "Ytterbium",   6.90,  1878, 70,"+3","Lanthanide","Stress gauge"},
  {"Lu", "Lutetium",    9.84,  1907, 71,"+3","Lanthanide","PET scan"},
  {"Hf", "Hafnium",    13.30,  1923, 72,"+4","Transition","Nuclear"},
  {"Ta", "Tantalum",   16.60,  1802, 73,"+5","Transition","Phones"},
  {"W",  "Tungsten",   19.30,  1783, 74,"+6","Transition","Filaments"},
  {"Re", "Rhenium",    21.00,  1925, 75,"+4","Transition","Jet engines"},
  {"Os", "Osmium",     22.60,  1803, 76,"+4","Transition","Pen nibs"},
  {"Ir", "Iridium",    22.60,  1803, 77,"+4","Transition","Spark plugs"},
  {"Pt", "Platinum",   21.50,  1735, 78,"+4","Transition","Jewelry"},
  {"Au", "Gold",       19.30,-3000,  79,"+3","Transition","Electronics"},
  {"Hg", "Mercury",    13.50,-2000,  80,"+2","Transition","Thermometers"},
  {"Tl", "Thallium",   11.80,  1861, 81,"+3","Metal",     "Switches"},
  {"Pb", "Lead",       11.30,-5000,  82,"+2","Metal",     "Batteries"},
  {"Bi", "Bismuth",     9.78,-3000,  83,"+3","Metal",     "Medicines"},
  {"Po", "Polonium",    9.20,  1898, 84,"+4","Metalloid", "Antistatic"},
  {"At", "Astatine",    7.00,  1940, 85,"-1","Halogen",   "Cancer therap"},
  {"Rn", "Radon",       9.73,  1900, 86,"0", "Noble Gas", "Cancer detect"},
  {"Fr", "Francium",    1.87,  1939, 87,"+1","Alkali",    "Research"},
  {"Ra", "Radium",      5.50,  1898, 88,"+2","Alkaline",  "Luminous paint"},
  {"Ac", "Actinium",   10.10,  1899, 89,"+3","Actinide",  "Neutron source"},
  {"Th", "Thorium",    11.70,  1828, 90,"+4","Actinide",  "Nuclear fuel"},
  {"Pa", "Protactinium",15.4,  1913, 91,"+5","Actinide",  "Research"},
  {"U",  "Uranium",    18.95,  1789, 92,"+4","Actinide",  "Nuclear fuel"},
  {"Np", "Neptunium",  20.20,  1940, 93,"+5","Actinide",  "Research"},
  {"Pu", "Plutonium",  19.84,  1940, 94,"+4","Actinide",  "Weapons,power"},
  {"Am", "Americium",  13.70,  1944, 95,"+3","Actinide",  "Smoke alarms"},
  {"Cm", "Curium",     13.50,  1944, 96,"+3","Actinide",  "Research"},
  {"Bk", "Berkelium",  14.00,  1949, 97,"+3","Actinide",  "Research"},
  {"Cf", "Californium",15.10,  1950, 98,"+3","Actinide",  "Neutron source"},
  {"Es", "Einsteinium",8.84,   1952, 99,"+3","Actinide",  "Research"},
  {"Fm", "Fermium",    0.00,   1952,100,"+3","Actinide",  "Research"},
  {"Md", "Mendelevium",0.00,   1955,101,"+3","Actinide",  "Research"},
  {"No", "Nobelium",   0.00,   1958,102,"+2","Actinide",  "Research"},
  {"Lr", "Lawrencium", 0.00,   1961,103,"+3","Actinide",  "Research"},
  {"Rf", "Rutherfordm",0.00,   1964,104,"+4","Transition","Research"},
  {"Db", "Dubnium",    0.00,   1970,105,"+5","Transition","Research"},
  {"Sg", "Seaborgium", 0.00,   1974,106,"+6","Transition","Research"},
  {"Bh", "Bohrium",    0.00,   1981,107,"+7","Transition","Research"},
  {"Hs", "Hassium",    0.00,   1984,108,"+8","Transition","Research"},
  {"Mt", "Meitnerium", 0.00,   1982,109,"?", "Transition","Research"},
  {"Ds", "Darmstadtm", 0.00,   1994,110,"?", "Transition","Research"},
  {"Rg", "Roentgenium",0.00,   1994,111,"?", "Transition","Research"},
  {"Cn", "Copernicium",0.00,   1996,112,"?", "Transition","Research"},
  {"Nh", "Nihonium",   0.00,   2004,113,"?", "Post-Trans","Research"},
  {"Fl", "Flerovium",  0.00,   1999,114,"?", "Post-Trans","Research"},
  {"Mc", "Moscovium",  0.00,   2003,115,"?", "Post-Trans","Research"},
  {"Lv", "Livermorium",0.00,   2000,116,"?", "Post-Trans","Research"},
  {"Ts", "Tennessine", 0.00,   2010,117,"?", "Halogen",   "Research"},
  {"Og", "Oganesson",  0.00,   2002,118,"?", "Noble Gas", "Research"},
};
const int ELEM_COUNT = 118;

void runPeriodicTable() {
  int idx = 0, needRd = true;
  while (true) {
    if (needRd) {
      tft.fillScreen(COL_BG);
      drawHeader("PERIODIC TABLE", COL_HEAD);
      const ChemElement& e = ELEMENTS[idx];

      tft.setTextSize(3); tft.setTextColor(COL_ACCENT);
      tft.setCursor(4, 22); tft.print(e.sym);

      tft.setTextSize(1); tft.setTextColor(COL_DIM);
      tft.setCursor(54, 22); tft.print("Z="); tft.print(e.atomicNum);

      tft.setTextColor(COL_GOLD);
      tft.setCursor(54, 36); tft.print(e.name);

      tft.drawLine(4, 52, 124, 52, COL_DIM);

      String yearStr = (e.year == 0) ? "Ancient" :
                       (e.year < 0) ? "~"+String(-e.year)+" BC" :
                       String(e.year)+" AD";

      struct { const char* lbl; String val; uint16_t col; } rows[] = {
        {"Dens", String(e.dens,2)+" g/cm3", COL_FG     },
        {"Disc", yearStr,                   COL_FG     },
        {"Chg",  String(e.chg),             COL_OK     },
        {"Grp",  String(e.group),           COL_ACCENT },
        {"Use",  String(e.desc),            COL_GOLD   },
      };
      for (int r=0;r<5;r++) {
        tft.setTextColor(COL_DIM);
        tft.setCursor(4, 58+r*17); tft.print(rows[r].lbl); tft.print(":");
        tft.setTextColor(rows[r].col);
        tft.setCursor(46, 58+r*17); tft.print(rows[r].val);
      }
      tft.setTextColor(COL_DIM);
      tft.setCursor(4,148); tft.print(String(idx+1)+"/118  UP/DN  BACK");
      needRd = false;
    }
    if (btnUp()  ) { idx=(idx>0)?idx-1:ELEM_COUNT-1; playNav(); delay(140); needRd=true; }
    if (btnDown()) { idx=(idx<ELEM_COUNT-1)?idx+1:0;  playNav(); delay(140); needRd=true; }
    if (btnBack()) { returnToMenu(1); return; }
    delay(20);
  }
}

// ════════════════════════════════════════════════════════════
//  CONSTANTS  (expanded, unit on same line)
// ════════════════════════════════════════════════════════════
struct PhysConst { const char* sym; const char* val; const char* unit; };
const PhysConst CONSTS[] = {
  { "pi",   "3.14159265",   ""          },
  { "e",    "2.71828182",   ""          },
  { "phi",  "1.61803398",   "(golden)"  },
  { "c",    "2.9979E8",     "m/s"       },
  { "h",    "6.626E-34",    "J.s"       },
  { "hbar", "1.055E-34",    "J.s"       },
  { "G",    "6.674E-11",    "m3/kg/s2"  },
  { "g",    "9.80665",      "m/s2"      },
  { "Na",   "6.022E23",     "/mol"      },
  { "k",    "1.380E-23",    "J/K"       },
  { "R",    "8.31446",      "J/mol/K"   },
  { "e-",   "1.602E-19",    "C"         },
  { "me",   "9.109E-31",    "kg"        },
  { "mp",   "1.673E-27",    "kg"        },
  { "eps0", "8.854E-12",    "F/m"       },
  { "mu0",  "1.257E-6",     "H/m"       },
  { "sig",  "5.670E-8",     "W/m2/K4"  },
  { "F",    "96485",        "C/mol"     },
  { "atm",  "101325",       "Pa"        },
  { "Ly",   "9.461E15",     "m"         },
};
const int CONST_COUNT = 20;

void runConstants() {
  tft.fillScreen(COL_BG);
  drawHeader("CONSTANTS", COL_HEAD);
  int scroll=0, visRows=7, needRd=1;
  while (true) {
    if (needRd) {
      tft.fillRect(0,19,128,136,COL_BG);
      for (int i=0;i<visRows;i++) {
        int ri=i+scroll; if(ri>=CONST_COUNT) break;
        int yy=22+i*19;
        // Symbol + value + unit all on one line
        tft.setTextColor(COL_GOLD);  tft.setCursor(4,yy);  tft.print(CONSTS[ri].sym);
        tft.setTextColor(COL_FG);    tft.setCursor(40,yy); tft.print(CONSTS[ri].val);
        tft.setTextColor(COL_ACCENT);tft.setCursor(40,yy+9); tft.print(CONSTS[ri].unit);
        tft.drawLine(2,yy+17,126,yy+17,0x1082);
      }
      tft.setTextColor(COL_DIM);
      tft.setCursor(4,153); tft.print("UP/DN  BACK=exit");
      needRd=0;
    }
    if (btnUp()  &&scroll>0)              {scroll--;playNav();delay(130);needRd=1;}
    if (btnDown()&&scroll<CONST_COUNT-visRows){scroll++;playNav();delay(130);needRd=1;}
    if (btnBack()) { returnToMenu(1); return; }
    delay(20);
  }
}

// ════════════════════════════════════════════════════════════
//  UNIT CONVERTER  — category > from-unit > to-unit > value
// ════════════════════════════════════════════════════════════

struct ConvUnit { const char* name; double factor; double offset; }; // T(target)=src*factor+offset
struct ConvCat  { const char* name; int unitStart; int unitCount; };

// All units relative to a base unit per category:
// Length: base=m  Volume: base=L  Temp: base=°C  Mass: base=kg
// Pressure: base=Pa  Energy: base=J  Power: base=W  Area: base=m2  Speed: base=m/s
const ConvUnit ALL_UNITS[] = {
  // Length (idx 0..7)  base=m
  {"m",     1,        0}, {"km",    0.001,    0}, {"cm",   100,      0},
  {"mm",    1000,     0}, {"in",    39.3701,  0}, {"ft",   3.28084,  0},
  {"yd",    1.09361,  0}, {"mi",    0.000621, 0},
  // Volume (idx 8..14)  base=L
  {"L",     1,        0}, {"mL",    1000,     0}, {"gal",  0.264172, 0},
  {"fl oz", 33.814,   0}, {"cup",   4.22675,  0}, {"pt",   2.11338,  0}, {"qt", 1.05669,0},
  // Temperature (idx 15..17)  special handling
  {"C",     1,        0}, {"F",     1,        0}, {"K",    1,        0},
  // Mass (idx 18..23)  base=kg
  {"kg",    1,        0}, {"g",     1000,     0}, {"mg",   1e6,      0},
  {"lb",    2.20462,  0}, {"oz",    35.274,   0}, {"t",    0.001,    0},
  // Pressure (idx 24..29)  base=Pa
  {"Pa",    1,        0}, {"kPa",   0.001,    0}, {"MPa",  1e-6,     0},
  {"bar",   1e-5,     0}, {"psi",   0.000145, 0}, {"atm",  9.869e-6, 0},
  // Energy (idx 30..34)  base=J
  {"J",     1,        0}, {"kJ",    0.001,    0}, {"cal",  0.239006, 0},
  {"kcal",  2.39e-4,  0}, {"kWh",   2.778e-7, 0},
  // Power (idx 35..38)  base=W
  {"W",     1,        0}, {"kW",    0.001,    0}, {"MW",   1e-6,     0}, {"hp",  0.00134, 0},
  // Area (idx 39..43)  base=m2
  {"m2",    1,        0}, {"cm2",   1e4,      0}, {"km2",  1e-6,     0},
  {"ft2",   10.7639,  0}, {"ac",    0.000247, 0},
  // Speed (idx 44..47)  base=m/s
  {"m/s",   1,        0}, {"km/h",  3.6,      0}, {"mph",  2.23694,  0}, {"kn", 1.94384, 0},
};

const ConvCat CONV_CATS[] = {
  {"Distance",    0,  8},
  {"Volume",      8,  7},
  {"Temperature", 15, 3},
  {"Mass",        18, 6},
  {"Pressure",    24, 6},
  {"Energy",      30, 5},
  {"Power",       35, 4},
  {"Area",        39, 5},
  {"Speed",       44, 4},
};
const int CAT_COUNT = 9;

// Temperature conversion (special — not simple factor)
double convTemp(double val, int fromIdx, int toIdx) {
  // 0=C 1=F 2=K  (relative to ALL_UNITS offset: fromIdx-15, toIdx-15)
  int fi = fromIdx - 15, ti = toIdx - 15;
  double c;
  if      (fi==0) c = val;
  else if (fi==1) c = (val-32.0)*5.0/9.0;
  else            c = val - 273.15;
  if      (ti==0) return c;
  else if (ti==1) return c*9.0/5.0+32.0;
  else            return c + 273.15;
}

double convertValue(double val, int fromAbsIdx, int toAbsIdx) {
  // Convert val from fromAbsIdx units to base, then to toAbsIdx units
  if (fromAbsIdx >= 15 && fromAbsIdx <= 17)
    return convTemp(val, fromAbsIdx, toAbsIdx);
  // Convert to base
  double base = val / ALL_UNITS[fromAbsIdx].factor;
  return base * ALL_UNITS[toAbsIdx].factor;
}

// Simple number entry for the converter: UP/DN scroll digit, SEL advance, BACK delete
// Returns entered value, or NAN if cancelled
double enterNumber(const char* prompt) {
  tft.fillScreen(COL_BG);
  drawHeader(prompt, COL_HEAD);

  String numStr = "0";
  int    digit  = 0;  // 0-9 picker

  // Digit picker items: 0-9 . - =enter
  const char* items[] = {"0","1","2","3","4","5","6","7","8","9",".","(-)",
                          "OK","CLR"};
  const int NITEM = 14;
  int pickerIdx = 1;

  tft.setTextColor(COL_DIM);
  tft.setCursor(4,22); tft.print("Enter value:");

  auto redrawNum = [&]() {
    tft.fillRect(0,32,128,30,COL_BG);
    tft.setTextSize(2);
    patchText(4,36,120,22,numStr,COL_OK);
    tft.setTextSize(1);

    // Picker (3 items centred)
    tft.fillRect(0,66,128,56,COL_BG);
    for (int d=-1;d<=1;d++) {
      int ii=(pickerIdx+d+NITEM)%NITEM;
      int yy=94+d*22;
      bool sel=(d==0);
      if(sel){
        tft.fillRect(20,yy-10,88,19,COL_SEL_BG);
        tft.drawRect(20,yy-10,88,19,COL_ACCENT);
        tft.setTextSize(2); tft.setTextColor(COL_FG);
        int cx=(128-strlen(items[ii])*12)/2;
        tft.setCursor(cx,yy-7); tft.print(items[ii]);
        tft.setTextSize(1);
      } else {
        tft.setTextColor(COL_DIM);
        int cx=(128-strlen(items[ii])*6)/2;
        tft.setCursor(cx,yy-3); tft.print(items[ii]);
      }
    }
    tft.setTextColor(0x2945);
    tft.setCursor(58,68); tft.print("^");
    tft.setCursor(58,118); tft.print("v");
    tft.setTextColor(COL_DIM);
    tft.setCursor(4,138); tft.print("SEL=enter  BACK=del/cancel");
  };
  redrawNum();

  while (true) {
    if (btnUp())   { pickerIdx=(pickerIdx>0)?pickerIdx-1:NITEM-1; playNav(); delay(100); redrawNum(); }
    if (btnDown()) { pickerIdx=(pickerIdx<NITEM-1)?pickerIdx+1:0; playNav(); delay(100); redrawNum(); }
    if (btnSel()) {
      playClick();
      String chosen = String(items[pickerIdx]);
      if (chosen == "OK") {
        return numStr.toDouble();
      } else if (chosen == "CLR") {
        numStr = "0";
      } else if (chosen == "(-)") {
        if (numStr.startsWith("-")) numStr.remove(0,1);
        else numStr = "-" + numStr;
      } else {
        if (numStr == "0" && chosen != ".") numStr = chosen;
        else if (numStr.length() < 10) numStr += chosen;
      }
      delay(160); redrawNum();
    }
    if (btnBack()) {
      if (numStr.length() > 1) {
        numStr.remove(numStr.length()-1);
        if (numStr == "-") numStr = "0";
        playBack(); delay(120); redrawNum();
      } else {
        return NAN; // cancelled
      }
    }
    delay(20);
  }
}

void runUnitConverter() {
  // Level 0: category
  int catSel = 0, unitFrom = 0, unitTo = 1;
  int level  = 0;

  while (true) {
    // ── Level 0: category list ─────────────────────────────────
    if (level == 0) {
      tft.fillScreen(COL_BG);
      drawHeader("UNIT CONVERTER", COL_HEAD);
      bool needRd = true;
      while (true) {
        if (needRd) {
          tft.fillRect(0,19,128,134,COL_BG);
          int vis = min(CAT_COUNT, 7);
          int sc  = (catSel>=vis) ? catSel-vis+1 : 0;
          for (int i=0;i<vis;i++) {
            int ri=i+sc; if(ri>=CAT_COUNT) break;
            int yy=22+i*18;
            bool sel=(ri==catSel);
            if(sel){ tft.fillRect(2,yy,124,16,COL_SEL_BG); tft.drawLine(2,yy,2,yy+15,COL_ACCENT); }
            tft.setTextColor(sel?COL_FG:COL_DIM);
            tft.setCursor(8,yy+4);
            tft.print(String(ri+1)+". "+String(CONV_CATS[ri].name));
          }
          needRd=false;
        }
        if (btnUp()  ) { catSel=(catSel>0)?catSel-1:CAT_COUNT-1; playNav(); delay(140); needRd=true; }
        if (btnDown()) { catSel=(catSel<CAT_COUNT-1)?catSel+1:0;  playNav(); delay(140); needRd=true; }
        if (btnBack()) { returnToMenu(1); return; }
        if (btnSel())  { playClick(); delay(160); level=1; unitFrom=0; break; }
        delay(20);
      }
    }

    // ── Level 1: choose FROM unit ─────────────────────────────
    if (level == 1) {
      const ConvCat& cat = CONV_CATS[catSel];
      tft.fillScreen(COL_BG);
      drawHeader(("FROM: "+String(cat.name)).c_str(), COL_ACCENT);
      bool needRd=true;
      while (true) {
        if (needRd) {
          tft.fillRect(0,19,128,134,COL_BG);
          int vis=min(cat.unitCount,7);
          int sc=(unitFrom>=vis)?unitFrom-vis+1:0;
          for (int i=0;i<vis;i++) {
            int ri=i+sc; if(ri>=cat.unitCount) break;
            int absIdx=cat.unitStart+ri;
            int yy=22+i*18; bool sel=(ri==unitFrom);
            if(sel){ tft.fillRect(2,yy,124,16,COL_SEL_BG); tft.drawLine(2,yy,2,yy+15,COL_ACCENT); }
            tft.setTextColor(sel?COL_FG:COL_DIM);
            tft.setCursor(8,yy+4); tft.print(ALL_UNITS[absIdx].name);
          }
          needRd=false;
        }
        if (btnUp()  ) { unitFrom=(unitFrom>0)?unitFrom-1:cat.unitCount-1; playNav(); delay(140); needRd=true; }
        if (btnDown()) { unitFrom=(unitFrom<cat.unitCount-1)?unitFrom+1:0;  playNav(); delay(140); needRd=true; }
        if (btnBack()) { level=0; break; }
        if (btnSel())  { playClick(); delay(160); level=2; unitTo=(unitFrom==0)?1:0; break; }
        delay(20);
      }
      if (level==0) continue;
    }

    // ── Level 2: choose TO unit ───────────────────────────────
    if (level == 2) {
      const ConvCat& cat = CONV_CATS[catSel];
      tft.fillScreen(COL_BG);
      drawHeader(("TO: "+String(ALL_UNITS[cat.unitStart+unitFrom].name)).c_str(), COL_GOLD);
      bool needRd=true;
      while (true) {
        if (needRd) {
          tft.fillRect(0,19,128,134,COL_BG);
          int vis=min(cat.unitCount,7);
          int sc=(unitTo>=vis)?unitTo-vis+1:0;
          for (int i=0;i<vis;i++) {
            int ri=i+sc; if(ri>=cat.unitCount) break;
            int absIdx=cat.unitStart+ri;
            int yy=22+i*18; bool sel=(ri==unitTo);
            if(sel){ tft.fillRect(2,yy,124,16,COL_SEL_BG); tft.drawLine(2,yy,2,yy+15,COL_ACCENT); }
            uint16_t c=(ri==unitFrom)?COL_DIM:(sel?COL_FG:COL_DIM);
            tft.setTextColor(c);
            tft.setCursor(8,yy+4); tft.print(ALL_UNITS[absIdx].name);
            if(ri==unitFrom){ tft.setTextColor(COL_DIM); tft.print(" (src)"); }
          }
          needRd=false;
        }
        if (btnUp()  ) { unitTo=(unitTo>0)?unitTo-1:cat.unitCount-1; playNav(); delay(140); needRd=true; }
        if (btnDown()) { unitTo=(unitTo<cat.unitCount-1)?unitTo+1:0;  playNav(); delay(140); needRd=true; }
        if (btnBack()) { level=1; break; }
        if (btnSel() && unitTo != unitFrom) { playClick(); delay(160); level=3; break; }
        delay(20);
      }
      if (level==1) continue;
    }

    // ── Level 3: enter value and show result ──────────────────
    if (level == 3) {
      const ConvCat& cat = CONV_CATS[catSel];
      int fromAbs = cat.unitStart + unitFrom;
      int toAbs   = cat.unitStart + unitTo;

      double val = enterNumber(("Enter "+String(ALL_UNITS[fromAbs].name)).c_str());
      if (isnan(val)) { level=2; continue; }

      double res = convertValue(val, fromAbs, toAbs);

      // Result screen
      tft.fillScreen(COL_BG);
      drawHeader("RESULT", COL_OK);
      tft.setTextColor(COL_DIM); tft.setCursor(4,24);
      tft.print(String(CONV_CATS[catSel].name));

      tft.setTextSize(1); tft.setTextColor(COL_FG);
      // Input
      tft.setCursor(4,42);
      char ibuf[22]; snprintf(ibuf,sizeof(ibuf),"%.6g %s",val,ALL_UNITS[fromAbs].name);
      tft.print(ibuf);
      // Arrow
      tft.setTextColor(COL_DIM);
      tft.setCursor(4,56); tft.print("=");
      // Result large
      tft.setTextSize(2); tft.setTextColor(COL_OK);
      char rbuf[16]; snprintf(rbuf,sizeof(rbuf),"%.5g",res);
      tft.setCursor(4,66); tft.print(rbuf);
      tft.setTextSize(1); tft.setTextColor(COL_ACCENT);
      tft.setCursor(4,90); tft.print(ALL_UNITS[toAbs].name);

      tft.setTextColor(COL_DIM);
      tft.setCursor(4,118); tft.print("SEL=convert again");
      tft.setCursor(4,130); tft.print("BACK=change units");

      while (true) {
        if (btnSel())  { level=3; break; } // enter new value same conversion
        if (btnBack()) { level=2; break; } // go back to to-unit selection
        delay(20);
      }
    }
  }
}

// ════════════════════════════════════════════════════════════
//  CALCULATOR  (fixed decimal, fixed division slash)
// ════════════════════════════════════════════════════════════
void runCalculator() {
  tft.fillScreen(COL_BG);
  drawHeader("CALCULATOR", COL_HEAD);

  // Items: digits, decimal, operators, = , CLR
  const char* items[] = {
    "0","1","2","3","4","5","6","7","8","9",
    ".","+","-","*","/",
    "=","CLR"
  };
  const int ITEM_COUNT = 17;

  String  expr      = "";
  String  result    = "";
  int     pickerIdx = 1;

  tft.drawLine(0, 58,  128, 58,  COL_DIM);
  tft.drawLine(0, 118, 128, 118, COL_DIM);
  tft.setTextColor(COL_DIM);
  tft.setCursor(4,22); tft.print("Expression:");
  tft.setCursor(4,122); tft.print("Result:");

  // ── Evaluator (left-to-right, correct decimal handling) ───
  auto evaluate = [](String s) -> double {
    s.trim();
    if (s.length() == 0) return 0;

    // Tokenise into numbers and operators
    // We'll use a simple state machine
    struct Token { bool isOp; char op; double num; };
    Token tokens[48];
    int   tc = 0;

    double cur = 0;
    bool   inNum   = false;
    bool   hasDot  = false;
    double decMul  = 0.1;
    bool   negNum  = false;

    for (int i = 0; i <= (int)s.length(); i++) {
      char c = (i < (int)s.length()) ? s.charAt(i) : '\0';

      bool isDigitChar = (c >= '0' && c <= '9');
      bool isDot       = (c == '.');
      bool isOp        = (c=='+' || c=='-' || c=='*' || c=='/');
      bool isEnd       = (c == '\0');

      if (isDigitChar) {
        if (!hasDot) cur = cur*10 + (c-'0');
        else         { cur += (c-'0')*decMul; decMul*=0.1; }
        inNum = true;
      } else if (isDot && !hasDot) {
        hasDot = true; decMul = 0.1; inNum = true;
      } else if (isOp || isEnd) {
        if (inNum || isEnd) {
          if (tc < 48) { tokens[tc].isOp=false; tokens[tc].num=negNum?-cur:cur; tc++; }
          cur=0; inNum=false; hasDot=false; decMul=0.1; negNum=false;
        }
        // Unary minus: operator after nothing or after another operator
        if (isOp) {
          bool prevIsOp = (tc==0) || (tokens[tc-1].isOp);
          if (c=='-' && prevIsOp) {
            negNum = true;
          } else {
            if (tc < 48) { tokens[tc].isOp=true; tokens[tc].op=c; tc++; }
          }
        }
      }
    }

    if (tc == 0) return 0;
    double result = tokens[0].num;
    for (int i = 1; i+1 < tc; i += 2) {
      char op  = tokens[i].op;
      double b = tokens[i+1].num;
      if      (op=='+') result += b;
      else if (op=='-') result -= b;
      else if (op=='*') result *= b;
      else if (op=='/')  {
        if (b == 0) return (double)NAN;
        result /= b;
      }
    }
    return result;
  };

  auto redrawAll = [&]() {
    // Expression
    tft.fillRect(0,32,128,24,COL_BG);
    tft.setTextColor(COL_FG); tft.setCursor(4,40);
    String disp = expr.length()>19 ? expr.substring(expr.length()-19) : expr;
    tft.print(disp + "_");

    // Picker
    tft.fillRect(0,60,128,56,COL_BG);
    for (int d=-1;d<=1;d++) {
      int ii=(pickerIdx+d+ITEM_COUNT)%ITEM_COUNT;
      int yy=88+d*22;
      bool sel=(d==0);
      if(sel){
        tft.fillRect(20,yy-10,88,19,COL_SEL_BG);
        tft.drawRect(20,yy-10,88,19,COL_ACCENT);
        tft.setTextSize(2); tft.setTextColor(COL_FG);
        int cx=(128-strlen(items[ii])*12)/2;
        tft.setCursor(max(cx,0),yy-7); tft.print(items[ii]);
        tft.setTextSize(1);
      } else {
        tft.setTextColor(COL_DIM);
        int cx=(128-strlen(items[ii])*6)/2;
        tft.setCursor(max(cx,0),yy-3); tft.print(items[ii]);
      }
    }
    tft.setTextColor(0x2945);
    tft.setCursor(58,62); tft.print("^");
    tft.setCursor(58,112); tft.print("v");

    // Result
    tft.fillRect(0,130,128,20,COL_BG);
    tft.setTextColor(result.startsWith("ERR")?COL_WARN:COL_OK);
    tft.setCursor(4,134); tft.print(result);
    tft.setTextColor(COL_DIM);
    tft.setCursor(4,150); tft.print("SEL=enter  BACK=del/exit");
  };

  redrawAll();

  while (true) {
    if (btnUp()  ) { pickerIdx=(pickerIdx>0)?pickerIdx-1:ITEM_COUNT-1; playNav(); delay(100); redrawAll(); }
    if (btnDown()) { pickerIdx=(pickerIdx<ITEM_COUNT-1)?pickerIdx+1:0; playNav(); delay(100); redrawAll(); }

    if (btnSel()) {
      playClick();
      String chosen = String(items[pickerIdx]);
      if (chosen=="CLR") {
        expr=""; result="";
      } else if (chosen=="=") {
        if (expr.length()>0) {
          double res = evaluate(expr);
          if (isnan(res))      result = "ERR: div/0";
          else if (isinf(res)) result = "ERR: inf";
          else {
            String rs=String(res,8);
            // Trim trailing zeros after decimal point
            if (rs.indexOf('.')>=0) {
              while (rs.endsWith("0")) rs.remove(rs.length()-1);
              if (rs.endsWith("."))   rs.remove(rs.length()-1);
            }
            result=rs;
          }
        }
      } else {
        // Prevent consecutive operators (replace last)
        if (expr.length()>0) {
          char last=expr.charAt(expr.length()-1);
          bool lastOp=(last=='+'||last=='-'||last=='*'||last=='/');
          bool newOp =(chosen=="+"||chosen=="-"||chosen=="*"||chosen=="/");
          if (lastOp && newOp) expr.remove(expr.length()-1);
        }
        if (expr.length()<24) expr+=chosen;
        result="";
      }
      delay(160); redrawAll();
    }

    if (btnBack()) {
      if (expr.length()>0) {
        expr.remove(expr.length()-1); result="";
        playBack(); delay(120); redrawAll();
      } else {
        returnToMenu(1); return;
      }
    }
    delay(20);
  }
}

// ════════════════════════════════════════════════════════════
//  WI-FI SCANNER  (unchanged)
// ════════════════════════════════════════════════════════════
void runWiFiScanner() {
  tft.fillScreen(COL_BG);
  drawHeader("WI-FI SCAN", COL_HEAD);
  tft.setTextColor(COL_DIM); tft.setCursor(4,24); tft.print("Scanning...");
  WiFi.mode(WIFI_STA); WiFi.disconnect(); delay(100);
  int n = WiFi.scanNetworks();
  tft.fillRect(0,19,128,130,COL_BG);
  if (n==0) {
    tft.setTextColor(COL_WARN); tft.setCursor(4,30); tft.print("No networks found");
    while (!btnBack()) delay(20);
    WiFi.scanDelete(); returnToMenu(1); return;
  }
  int selNet=0; bool needRd=true;
  while (true) {
    if (needRd) {
      tft.fillRect(0,19,128,134,COL_BG);
      int vis=min(n,7), sc=(selNet>=vis)?selNet-vis+1:0;
      for (int i=0;i<vis&&(i+sc)<n;i++) {
        int ri=i+sc; int yy=22+i*18; bool sel=(ri==selNet);
        if(sel){tft.fillRect(2,yy-1,124,16,COL_SEL_BG);tft.drawLine(2,yy-1,2,yy+14,COL_ACCENT);}
        String ssid=WiFi.SSID(ri); if(ssid.length()>13) ssid=ssid.substring(0,12)+"~";
        tft.setTextColor(sel?COL_FG:COL_DIM); tft.setCursor(6,yy+1); tft.print(ssid);
        int rssi=WiFi.RSSI(ri); int bars=map(constrain(rssi,-90,-40),-90,-40,1,5);
        for (int b=0;b<5;b++) tft.fillRect(106+b*4,yy+14-(b+1)*2,3,(b+1)*2,b<bars?COL_OK:COL_DIM);
        if(sel){
          tft.setTextColor(COL_ACCENT); tft.setCursor(6,148);
          tft.print("Ch:"); tft.print(WiFi.channel(ri));
          tft.print("  "); tft.print(rssi); tft.print("dBm ");
          tft.print(WiFi.encryptionType(ri)==WIFI_AUTH_OPEN?"OPEN":"ENC");
        }
      }
      needRd=false;
    }
    if (btnUp()  ){selNet=(selNet>0)?selNet-1:n-1; playNav();delay(130);needRd=true;}
    if (btnDown()){selNet=(selNet<n-1)?selNet+1:0; playNav();delay(130);needRd=true;}
    if (btnBack()){WiFi.scanDelete();returnToMenu(1);return;}
    delay(20);
  }
}

// ════════════════════════════════════════════════════════════
//  BLE SCANNER  (unchanged)
// ════════════════════════════════════════════════════════════
void runBluetoothScanner() {
  tft.fillScreen(COL_BG);
  drawHeader("BLE SCAN", COL_HEAD);
  tft.setTextColor(COL_DIM); tft.setCursor(4,24); tft.print("Scanning 4s...");
  BLEDevice::init(""); BLEScan* scan=BLEDevice::getScan();
  scan->setActiveScan(true); BLEScanResults res=scan->start(4,false);
  tft.fillRect(0,19,128,130,COL_BG);
  int count=res.getCount();
  if(count==0){tft.setTextColor(COL_WARN);tft.setCursor(4,30);tft.print("No BLE devices");
    while(!btnBack())delay(20); returnToMenu(1); return;}
  int selDev=0; bool needRd=true;
  while(true){
    if(needRd){
      tft.fillRect(0,19,128,134,COL_BG);
      int vis=min(count,7),sc=(selDev>=vis)?selDev-vis+1:0;
      for(int i=0;i<vis&&(i+sc)<count;i++){
        int ri=i+sc; int yy=22+i*18; bool sel=(ri==selDev);
        auto dev=res.getDevice(ri);
        if(sel){tft.fillRect(2,yy-1,124,16,COL_SEL_BG);tft.drawLine(2,yy-1,2,yy+14,COL_ACCENT);}
        tft.setTextColor(sel?COL_ACCENT:COL_DIM); tft.setCursor(6,yy+1);
        tft.print(String(dev.getAddress().toString().c_str()).substring(0,12));
        tft.setTextColor(sel?COL_OK:COL_DIM); tft.setCursor(100,yy+1); tft.print(dev.getRSSI());
        if(sel){String nm=String(dev.getName().c_str());if(!nm.length())nm="(unnamed)";
          patchText(4,148,120,9,nm.substring(0,20),COL_GOLD);}
      }
      tft.setTextColor(COL_DIM); tft.setCursor(4,157);
      tft.print(String(count)+" devs  BACK=exit"); needRd=false;
    }
    if(btnUp()  ){selDev=(selDev>0)?selDev-1:count-1;playNav();delay(130);needRd=true;}
    if(btnDown()){selDev=(selDev<count-1)?selDev+1:0; playNav();delay(130);needRd=true;}
    if(btnBack()){returnToMenu(1);return;}
    delay(20);
  }
}

// ════════════════════════════════════════════════════════════
//  STOPWATCH  (fixed lap spacing: 4 laps with 14px row height)
// ════════════════════════════════════════════════════════════
void runStopwatch() {
  tft.fillScreen(COL_BG);
  drawHeader("STOPWATCH", COL_HEAD);

  float  lapTimes[4] = {0,0,0,0};
  int    lapIdx      = 0;
  bool   active      = false;
  long   elapsed     = 0, startMs = 0;

  // Static layout
  tft.setTextColor(COL_DIM);
  tft.setCursor(4, 76);  tft.print("SEL=start/stop");
  tft.setCursor(4, 86);  tft.print("UP=lap  BACK=exit");
  tft.drawLine(0, 96, 128, 96, COL_DIM);
  // Lap labels
  for (int i=0;i<4;i++) {
    tft.setTextColor(COL_DIM);
    tft.setCursor(4, 100+i*14);
    tft.print("L"+String(i+1)+":");
  }

  auto fmtTime=[](long ms)->String{
    int m=ms/60000, s=(ms%60000)/1000, c=(ms%1000)/10;
    char b[12]; sprintf(b,"%02d:%02d.%02d",m,s,c); return String(b);
  };

  unsigned long lastSel=0, lastUp=0;
  while (true) {
    if (btnBack()) { returnToMenu(1); return; }
    if (btnSel() && millis()-lastSel>250) {
      lastSel=millis(); active=!active;
      if(active) startMs=millis()-elapsed;
      playClick();
    }
    if (btnUp() && active && millis()-lastUp>400) {
      lastUp=millis();
      lapTimes[lapIdx%4]=elapsed/1000.0f; lapIdx++;
      playTone(1400,30);
      // Redraw all laps
      for(int i=0;i<4;i++){
        uint16_t lc=(lapTimes[i]>0)?COL_GOLD:COL_DIM;
        // Clear old value then redraw
        tft.fillRect(26,100+i*14,100,10,COL_BG);
        tft.setCursor(26,100+i*14); tft.setTextColor(lc);
        tft.print(lapTimes[i]>0 ? String(lapTimes[i],2)+"s" : "---");
      }
    }
    if(active) elapsed=millis()-startMs;

    tft.setTextSize(2);
    patchText(4,34,120,22,fmtTime(elapsed),active?COL_OK:COL_WARN);
    tft.setTextSize(1);
    patchText(4,60,120,10,active?(" RUNNING "):(" STOPPED"),
              active?COL_OK:COL_DIM);

    // Refresh lap values every loop
    for(int i=0;i<4;i++){
      tft.fillRect(26,100+i*14,100,10,COL_BG);
      tft.setCursor(26,100+i*14);
      tft.setTextColor(lapTimes[i]>0?COL_GOLD:COL_DIM);
      tft.print(lapTimes[i]>0 ? String(lapTimes[i],2)+"s" : "---");
    }
    delay(40);
  }
}

// ════════════════════════════════════════════════════════════
//  COUNTDOWN TIMER  (fixes: set-value preserved, hold=+10s)
// ════════════════════════════════════════════════════════════
void runTimerApp() {
  tft.fillScreen(COL_BG);
  drawHeader("TIMER", COL_HEAD);

  int  setMinutes  = 0, setSeconds = 30;   // The configured duration — never changes after start
  int  minutes     = 0, seconds    = 30;   // Current countdown values
  bool counting    = false;
  bool editing     = true;
  int  editField   = 1;                    // 0=min 1=sec
  long lastTick    = millis();

  // For hold-to-accelerate on UP/DN in edit mode
  unsigned long heldSince = 0;
  bool          held      = false;

  auto fmtSet = [&]() -> String {
    char b[10]; sprintf(b,"%02d:%02d",setMinutes,setSeconds); return String(b);
  };

  auto drawHints = [&]() {
    tft.fillRect(0,136,128,16,COL_BG);
    tft.setTextColor(COL_DIM); tft.setCursor(4,138);
    if (editing) tft.print("UP/DN=set(hold=x10) SEL=ok");
    else         tft.print("SEL=pause  BACK=reset");
  };

  auto drawAll = [&]() {
    tft.fillRect(0,22,128,112,COL_BG);
    // Current time BIG
    char buf[10]; sprintf(buf,"%02d:%02d",minutes,seconds);
    tft.setTextSize(3); tft.setTextColor(COL_FG);
    tft.setCursor(4,44); tft.print(buf);
    tft.setTextSize(1);

    if (editing) {
      // Underline active field
      int ux=(editField==0)?4:58;
      tft.drawLine(ux,72,ux+40,72,COL_ACCENT);
    }

    // "Set:" line — always shows the configured start value
    tft.setTextColor(COL_DIM); tft.setCursor(4,84);
    tft.print("Set: ");
    tft.setTextColor(COL_GOLD);
    tft.print(fmtSet());

    if (!editing) {
      // Colour hint based on remaining
      int rem = minutes*60+seconds;
      uint16_t tc = (rem>30)?COL_OK:(rem>10?COL_GOLD:COL_WARN);
      tft.setTextSize(3);
      tft.fillRect(4,44,120,26,COL_BG);
      tft.setTextColor(tc); tft.setCursor(4,44); tft.print(buf);
      tft.setTextSize(1);
    }
  };

  drawAll(); drawHints();

  while (true) {
    if (editing) {
      bool up   = btnUp();
      bool down = btnDown();
      unsigned long now = millis();

      if (up || down) {
        if (!held) { held=true; heldSince=now; }
        int step = (now-heldSince > 1200) ? 10 : 1;  // hold >1.2s → jump 10
        if (editField==0) {
          if (up)   setMinutes=min(setMinutes+step,99);
          if (down) setMinutes=max(setMinutes-step,0);
          minutes=setMinutes; seconds=setSeconds;
        } else {
          if (up)   setSeconds=min(setSeconds+step,59);
          if (down) setSeconds=max(setSeconds-step,0);
          minutes=setMinutes; seconds=setSeconds;
        }
        playNav(); delay(held&&(now-heldSince>1200)?80:130);
        drawAll();
      } else {
        held=false;
      }

      if (btnSel()) {
        if (editField==0) { editField=1; playClick(); delay(180); drawAll(); }
        else {
          if (setMinutes>0||setSeconds>0) {
            editing=false; counting=true;
            minutes=setMinutes; seconds=setSeconds;
            lastTick=millis();
            playClick(); delay(180); drawAll(); drawHints();
          }
        }
      }
      if (btnBack()) { returnToMenu(1); return; }

    } else {
      if (btnSel()) {
        counting=!counting; lastTick=millis(); playClick(); delay(200);
      }
      if (btnBack()) {
        // Reset: restore to configured start value
        counting=false; editing=true; editField=1;
        minutes=setMinutes; seconds=setSeconds;
        playBack(); delay(200); drawAll(); drawHints();
        continue;
      }
      if (counting && (millis()-lastTick>=1000)) {
        lastTick=millis();
        if (seconds>0) seconds--;
        else if (minutes>0) { minutes--; seconds=59; }
        else {
          counting=false; editing=true;
          for(int i=0;i<4;i++){playTone(1047,80);delay(50);}
          // Keep set values intact, reset display
          minutes=setMinutes; seconds=setSeconds;
          drawAll(); drawHints(); delay(800);
          continue;
        }
        drawAll();
      }
    }
    delay(20);
  }
}
