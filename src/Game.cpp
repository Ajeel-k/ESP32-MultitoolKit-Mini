/*
 * ============================================================
 *  Game.ino v3  —  Simon / Dino / Snake / Flappy Bird
 *
 *  Forward declarations needed because Arduino IDE compiles
 *  tabs alphabetically (G before M/S/U).
 * ============================================================
 */
#include "main.hpp"
#include "Menu.hpp"

// ── Forward declarations ──────────────────────────────────────
void returnToMenu(int level);
void drawHeader(const char* title, uint16_t col);
void patchText(int x, int y, int w, int h, String val, uint16_t col);
void playTone(int freq, int dur);
void playClick();
void playNav();
void playBack();
void drawCentered(int y, const char* txt, uint16_t col, int sz);

// ── Simon colour tables ───────────────────────────────────────
const uint16_t SIM_BRIGHT[4] = { ST7735_RED, ST7735_BLUE, ST7735_GREEN, ST7735_YELLOW };
const uint16_t SIM_DARK[4]   = { 0x7800,     0x000F,      0x03E0,       0x7BE0 };
const int      SIM_TONES[4]  = { 262,        330,          392,          523    };
const char*    SIM_KEYS[4]   = { "K2",       "K1",        "K3",         "K4"   };

// ── Simon: draw base 4-quadrant grid WITH key labels ─────────
void simDrawGrid(int scoreVal = -1) {
  tft.fillRect(  0,  0, 64, 80, SIM_DARK[0]);
  tft.fillRect( 64,  0, 64, 80, SIM_DARK[1]);
  tft.fillRect(  0, 80, 64, 80, SIM_DARK[2]);
  tft.fillRect( 64, 80, 64, 80, SIM_DARK[3]);

  // Key labels — always on top
  tft.setTextSize(1); tft.setTextColor(ST7735_WHITE);
  tft.setCursor(23, 35); tft.print(SIM_KEYS[0]);
  tft.setCursor(87, 35); tft.print(SIM_KEYS[1]);
  tft.setCursor(23,115); tft.print(SIM_KEYS[2]);
  tft.setCursor(87,115); tft.print(SIM_KEYS[3]);

  // Centre cross
  tft.drawLine(  0, 80, 128, 80, ST7735_BLACK);
  tft.drawLine( 64,  0,  64, 160, ST7735_BLACK);

  // Score (always visible in centre)
  if (scoreVal >= 0) {
    tft.fillRect(54, 74, 20, 12, ST7735_BLACK);
    tft.setTextColor(ST7735_WHITE); tft.setTextSize(1);
    tft.setCursor(56, 77); tft.print(scoreVal);
  }
}

// ── Flash quadrant and restore label immediately after ────────
void simFlash(int id, int dur, int scoreVal) {
  int x = (id == 1 || id == 3) ? 64 : 0;
  int y = (id == 2 || id == 3) ? 80 : 0;

  tft.fillRect(x, y, 64, 80, SIM_BRIGHT[id]);
  // Redraw label over bright colour
  tft.setTextSize(1); tft.setTextColor(ST7735_BLACK);
  tft.setCursor(x+23, y+35); tft.print(SIM_KEYS[id]);
  // Score stays visible
  tft.fillRect(54, 74, 20, 12, ST7735_BLACK);
  tft.setTextColor(ST7735_WHITE);
  tft.setCursor(56, 77); tft.print(scoreVal);

  if (soundOn) tone(PIN_BUZZ, SIM_TONES[id], dur);
  delay(dur);

  tft.fillRect(x, y, 64, 80, SIM_DARK[id]);
  // Restore label after dim
  tft.setTextColor(ST7735_WHITE);
  tft.setCursor(x+23, y+35); tft.print(SIM_KEYS[id]);
  // Restore centre cross line
  if (y == 0) tft.drawLine(0, 80, 128, 80, ST7735_BLACK);
  if (x == 0) tft.drawLine(64,  0,  64, 160, ST7735_BLACK);
  // Restore score
  tft.fillRect(54, 74, 20, 12, ST7735_BLACK);
  tft.setTextColor(ST7735_WHITE);
  tft.setCursor(56, 77); tft.print(scoreVal);

  noTone(PIN_BUZZ);
  delay(60);
}

int simReadButton() {
  while (true) {
    if (digitalRead(PIN_K1_UP)  == LOW) return 1;
    if (digitalRead(PIN_K4_SEL) == LOW) return 3;
    int v = analogRead(PIN_A_BTN);
    if (v < 200)                return 0;
    if (v >= 1000 && v <= 2600) return 2;
    delay(8);
  }
}

void simWaitRelease(int id) {
  while (true) {
    bool still = false;
    if (id==1 && digitalRead(PIN_K1_UP) ==LOW) still=true;
    if (id==3 && digitalRead(PIN_K4_SEL)==LOW) still=true;
    if (id==0||id==2) {
      int v=analogRead(PIN_A_BTN);
      if(id==0&&v<200)               still=true;
      if(id==2&&v>=1000&&v<=2600)    still=true;
    }
    if (!still) break;
    delay(8);
  }
}

// ════════════════════════════════════════════════════════════
//  SIMON SAYS
// ════════════════════════════════════════════════════════════
void runSimonSays() {
  int hiScore = prefs.getInt("simon_hi", 0);

  tft.fillScreen(ST7735_BLACK);
  tft.setTextColor(ST7735_WHITE); tft.setTextSize(2);
  tft.setCursor(14,35); tft.print("SIMON");
  tft.setCursor(20,58); tft.print(" SAYS");
  tft.setTextSize(1);
  tft.setTextColor(SIM_DARK[3]);
  tft.setCursor(16,88); tft.print("Best: "+String(hiScore));
  tft.setTextColor(ST7735_WHITE);
  tft.setCursor(10,108); tft.print("Press K1 to start");
  tft.setCursor(10,124); tft.print("Speed: "+String(simonSpeed)+"ms");

  while (digitalRead(PIN_K1_UP)==HIGH) delay(8);
  while (digitalRead(PIN_K1_UP)==LOW)  delay(8);

  #define SIM_MAX 50
  uint8_t seq[SIM_MAX];
  int len=0; bool dead=false;

  tft.fillScreen(ST7735_BLACK);
  simDrawGrid(0);
  delay(600);

  while (!dead) {
    seq[len]=(uint8_t)random(0,4); len++;

    // Update score
    simDrawGrid(len-1);
    delay(300);

    for (int i=0;i<len;i++) simFlash(seq[i], simonSpeed, len-1);

    for (int i=0;i<len;i++) {
      int guess=simReadButton();
      simFlash(guess, 140, len-1);
      simWaitRelease(guess);
      if (guess!=seq[i]) { dead=true; break; }
    }

    if (!dead) {
      for (int q=0;q<4;q++)
        tft.fillRect((q==1||q==3)?64:0,(q==2||q==3)?80:0,64,80,SIM_BRIGHT[q]);
      if(soundOn){tone(PIN_BUZZ,880,80);delay(80);noTone(PIN_BUZZ);}
      delay(120);
      simDrawGrid(len);
    }
  }

  if(soundOn){tone(PIN_BUZZ,250,200);delay(250);tone(PIN_BUZZ,180,200);delay(250);
              tone(PIN_BUZZ,120,400);delay(500);noTone(PIN_BUZZ);}

  int score=len-1;
  if(score>hiScore){hiScore=score; prefs.putInt("simon_hi",hiScore);}

  tft.fillScreen(ST7735_BLACK);
  tft.setTextSize(2); tft.setTextColor(ST7735_RED);
  tft.setCursor(14,30); tft.print("GAME");
  tft.setCursor(14,54); tft.print("OVER");
  tft.setTextSize(1); tft.setTextColor(ST7735_WHITE);
  tft.setCursor(10,85);  tft.print("Score:  "+String(score));
  tft.setCursor(10,100); tft.print("Best:   "+String(hiScore));
  if(score>=hiScore&&score>0){tft.setTextColor(SIM_BRIGHT[3]);tft.setCursor(10,115);tft.print("NEW HIGH SCORE!");}
  tft.setTextColor(ST7735_WHITE);
  tft.setCursor(10,135); tft.print("K1 = Play again");
  tft.setCursor(10,148); tft.print("K3 = Main menu");

  while(true){
    if(digitalRead(PIN_K1_UP)==LOW){while(digitalRead(PIN_K1_UP)==LOW)delay(8);
      currentState=STATE_APP; delay(200); return;}
    int v=analogRead(PIN_A_BTN);
    if(v>=1000&&v<=2600){returnToMenu(1);return;}
    delay(10);
  }
}

// ════════════════════════════════════════════════════════════
//  CHROME DINO
// ════════════════════════════════════════════════════════════
void runChromeDino() {
  tft.fillScreen(COL_BG);
  drawHeader("DINO RUN", COL_HEAD);
  int hiScore=prefs.getInt("dino_hi",0);
  int score=0, dinoY=108, velY=0, obstX=128, speed=3, frameCount=0;
  bool jumping=false;
  const int GND=118;
  tft.drawLine(0,GND+1,128,GND+1,COL_DIM);

  while(true){
    if(btnBack()){returnToMenu(1);return;}
    if((btnSel()||btnUp())&&!jumping){velY=-9;jumping=true;if(soundOn)tone(PIN_BUZZ,600,40);}
    if(jumping){dinoY+=velY;velY+=1;if(dinoY>=GND-12){dinoY=GND-12;jumping=false;velY=0;}}
    obstX-=speed;
    if(obstX<-10){obstX=128+random(0,40);score++;if(score%5==0&&speed<7)speed++;}
    bool hit=(obstX>14&&obstX<28&&dinoY>GND-22);
    if(hit){if(soundOn){tone(PIN_BUZZ,150,400);delay(400);noTone(PIN_BUZZ);}
      if(score>hiScore){hiScore=score;prefs.putInt("dino_hi",hiScore);}break;}
    tft.fillRect(0,19,128,GND-18,COL_BG);
    bool legAlt=(frameCount/4)%2;
    tft.fillRect(15,dinoY,12,12,COL_FG);
    tft.fillRect(15,dinoY-4,6,4,COL_FG);
    tft.fillRect(15+(legAlt?4:0),GND-12,4,4,COL_DIM);
    tft.fillRect(15+(legAlt?0:4),GND-12,4,4,COL_DIM);
    tft.fillRect(obstX,GND-14,7,14,0x0400);
    tft.fillRect(obstX-3,GND-10,4,8,0x0400);
    tft.fillRect(obstX+7,GND-10,4,8,0x0400);
    tft.drawLine(0,GND+1,128,GND+1,COL_DIM);
    patchText(4,19,60,9,"SC:"+String(score),COL_FG);
    patchText(68,19,60,9,"HI:"+String(hiScore),COL_GOLD);
    frameCount++; delay(28);
  }

  tft.fillScreen(COL_BG); drawHeader("GAME OVER",COL_WARN);
  tft.setTextSize(2); patchText(10,40,108,22,"Score",COL_FG);
  patchText(10,64,108,22,String(score),COL_OK);
  tft.setTextSize(1);
  patchText(10,92,108,10,"Best: "+String(hiScore),COL_GOLD);
  patchText(10,110,108,10,"SEL=retry  BACK=menu",COL_DIM);
  while(true){if(btnSel()){delay(200);currentState=STATE_APP;return;}if(btnBack()){returnToMenu(1);return;}delay(20);}
}

// ════════════════════════════════════════════════════════════
//  SNAKE
// ════════════════════════════════════════════════════════════
#define SN_CELL  4
#define SN_COLS  (128/SN_CELL)
#define SN_ROWS  ((160-20)/SN_CELL)
#define SN_MAX   80

void runSnakeGame() {
  tft.fillScreen(COL_BG);
  drawHeader("SNAKE", COL_OK);
  int hiScore=prefs.getInt("snake_hi",0);

  auto startGame=[&]()->bool{
    int sX[SN_MAX],sY[SN_MAX],sLen=4,dX=1,dY=0;
    for(int i=0;i<sLen;i++){sX[i]=SN_COLS/2-i;sY[i]=SN_ROWS/2;}
    auto newFood=[&](int&fx,int&fy){
      do{fx=random(1,SN_COLS-1);fy=random(1,SN_ROWS-1);
        bool on=false;for(int i=0;i<sLen;i++)if(sX[i]==fx&&sY[i]==fy){on=true;break;}
        if(!on)break;}while(true);};
    int fX,fY; newFood(fX,fY);
    tft.fillRect(0,19,128,141,COL_BG);
    tft.drawRect(0,19,128,141,COL_DIM);
    int score=0,frameDelay=130;
    unsigned long lastMove=millis();
    while(true){
      if(btnUp()  &&dY==0){dX= 0;dY=-1;}
      if(btnDown()&&dY==0){dX= 0;dY= 1;}
      if(btnBack()&&dX==0){dX=-1;dY= 0;}
      if(btnSel() &&dX==0){dX= 1;dY= 0;}
      if(millis()-lastMove<(unsigned long)frameDelay){delay(10);continue;}
      lastMove=millis();
      tft.fillRect(sX[sLen-1]*SN_CELL+1,20+sY[sLen-1]*SN_CELL,SN_CELL-1,SN_CELL-1,COL_BG);
      for(int i=sLen-1;i>0;i--){sX[i]=sX[i-1];sY[i]=sY[i-1];}
      sX[0]+=dX;sY[0]+=dY;
      if(sX[0]<0||sX[0]>=SN_COLS||sY[0]<0||sY[0]>=SN_ROWS)return false;
      for(int i=1;i<sLen;i++)if(sX[i]==sX[0]&&sY[i]==sY[0])return false;
      if(sX[0]==fX&&sY[0]==fY){
        if(sLen<SN_MAX)sLen++;score++;frameDelay=max(50,130-score*3);
        playTone(1800,20);newFood(fX,fY);
        tft.fillRect(fX*SN_CELL+1,20+fY*SN_CELL,SN_CELL-1,SN_CELL-1,COL_WARN);
        patchText(4,19,60,9,"Sc:"+String(score),COL_FG);
        patchText(68,19,60,9,"Hi:"+String(max(score,hiScore)),COL_GOLD);
      }
      tft.fillRect(sX[0]*SN_CELL+1,20+sY[0]*SN_CELL,SN_CELL-1,SN_CELL-1,COL_OK);
      for(int i=1;i<sLen;i++)tft.fillRect(sX[i]*SN_CELL+1,20+sY[i]*SN_CELL,SN_CELL-1,SN_CELL-1,0x03E0);
      tft.fillRect(fX*SN_CELL+1,20+fY*SN_CELL,SN_CELL-1,SN_CELL-1,COL_WARN);
      if(score>hiScore){hiScore=score;prefs.putInt("snake_hi",hiScore);}
    }
  };

  while(true){
    startGame();
    for(int i=0;i<3;i++){playTone(200,100);delay(50);}
    tft.fillScreen(COL_BG); drawHeader("GAME OVER",COL_WARN);
    tft.setTextSize(2); tft.setTextColor(COL_FG); tft.setCursor(10,40); tft.print("SNAKE");
    tft.setTextSize(1);
    patchText(10,74,108,10,"Best: "+String(hiScore),COL_GOLD);
    patchText(10,90,108,10,"SEL=retry",COL_OK);
    patchText(10,104,108,10,"BACK=menu",COL_DIM);
    bool retry=false;
    while(true){if(btnSel()){retry=true;delay(200);break;}if(btnBack()){retry=false;delay(200);break;}delay(20);}
    if(!retry){returnToMenu(1);return;}
    tft.fillScreen(COL_BG);
    drawHeader("SNAKE",COL_OK);
  }
}

// ════════════════════════════════════════════════════════════
//  FLAPPY BIRD
//  K1 (UP) or SEL = flap.  BACK = exit.
//  Bird is a 6x6 pixel square. Pipes are two rects with a gap.
// ════════════════════════════════════════════════════════════
void runFlappyBird() {
  tft.fillScreen(COL_BG);
  drawHeader("FLAPPY BIRD", COL_GOLD);

  int hiScore = prefs.getInt("flappy_hi", 0);

  // Splash
  tft.setTextColor(COL_FG); tft.setTextSize(1);
  tft.setCursor(14,38); tft.print("Press UP to flap!");
  tft.setCursor(14,52); tft.print("Avoid the pipes.");
  tft.setCursor(14,70); tft.setTextColor(COL_GOLD);
  tft.print("Best: "); tft.print(hiScore);
  tft.setTextColor(COL_DIM); tft.setCursor(14,90); tft.print("BACK = exit");

  // Wait for K1 or exit
  while(true){
    if(btnUp()||btnSel()) break;
    if(btnBack()){returnToMenu(1);return;}
    delay(10);
  }
  delay(150);

  // Game constants
  const int PLAY_TOP  = 19;    // below header
  const int PLAY_H    = 141;   // 160 - 19
  const int BIRD_X    = 20;
  const int BIRD_W    = 6;
  const int BIRD_H    = 6;
  const int PIPE_W    = 10;
  const int GAP_H     = 36;    // gap height in pixels

  // State
  float birdY     = PLAY_TOP + PLAY_H / 2.0f;
  float velY      = 0;
  float gravity   = 0.45f;
  float flapForce = -3.8f;

  int   pipeX     = 128;
  int   pipeGapY  = PLAY_TOP + 30 + random(0, PLAY_H - GAP_H - 40);
  int   score     = 0;
  int   frameSkip = 0;   // intro grace period

  tft.fillRect(0, PLAY_TOP, 128, PLAY_H, COL_BG);

  while(true){
    if(btnBack()){returnToMenu(1);return;}

    // Flap
    if((btnUp()||btnSel()) && frameSkip>4){
      velY = flapForce;
      if(soundOn) tone(PIN_BUZZ,800,25);
    }

    // Physics
    velY  += gravity;
    birdY += velY;

    // Clamp to play area
    if(birdY < PLAY_TOP)              { birdY = PLAY_TOP; velY = 0; }
    if(birdY+BIRD_H > PLAY_TOP+PLAY_H){ birdY = PLAY_TOP+PLAY_H-BIRD_H; velY=0; }

    // Move pipe
    pipeX -= 2;
    if(pipeX < -PIPE_W){
      pipeX    = 128;
      pipeGapY = PLAY_TOP + 20 + random(0, PLAY_H - GAP_H - 40);
      score++;
      if(score>hiScore){hiScore=score;prefs.putInt("flappy_hi",hiScore);}
      if(soundOn) tone(PIN_BUZZ,1200,20);
    }

    // Collision: bird vs pipe column
    int bx1 = BIRD_X, bx2 = BIRD_X+BIRD_W;
    int by1 = (int)birdY, by2 = (int)birdY+BIRD_H;
    int px1 = pipeX,      px2 = pipeX+PIPE_W;
    int gapTop = pipeGapY, gapBot = pipeGapY + GAP_H;

    bool hitsX = (bx2 > px1 && bx1 < px2);
    bool hitsY = (by1 < gapTop || by2 > gapBot);
    if(hitsX && hitsY) break;

    // Draw
    tft.fillRect(0, PLAY_TOP, 128, PLAY_H, COL_BG);

    // Pipes (dark green)
    tft.fillRect(pipeX, PLAY_TOP,  PIPE_W, gapTop-PLAY_TOP, 0x0400);
    tft.fillRect(pipeX, gapBot,    PIPE_W, PLAY_TOP+PLAY_H-gapBot, 0x0400);
    // Pipe caps (brighter green)
    tft.fillRect(pipeX-1, gapTop-4,      PIPE_W+2, 4, 0x0780);
    tft.fillRect(pipeX-1, gapBot,         PIPE_W+2, 4, 0x0780);

    // Bird (yellow square with small eye)
    tft.fillRect(BIRD_X, (int)birdY, BIRD_W, BIRD_H, COL_GOLD);
    tft.fillRect(BIRD_X+4, (int)birdY+1, 1, 1, ST7735_BLACK); // eye

    // Score
    patchText(4,19,60,9,"SC:"+String(score),COL_FG);
    patchText(68,19,60,9,"HI:"+String(hiScore),COL_GOLD);

    frameSkip++; delay(25);
  }

  // Death
  if(soundOn){tone(PIN_BUZZ,150,400);delay(400);noTone(PIN_BUZZ);}

  tft.fillScreen(COL_BG); drawHeader("GAME OVER", COL_WARN);
  tft.setTextSize(2); patchText(10,36,108,22,"FLAPPY",COL_GOLD);
  tft.setTextSize(1);
  patchText(10,66,108,10,"Score: "+String(score),COL_FG);
  patchText(10,80,108,10,"Best:  "+String(hiScore),COL_GOLD);
  patchText(10,98,108,10,"SEL=retry  BACK=menu",COL_DIM);

  while(true){
    if(btnSel()){delay(200);currentState=STATE_APP;return;}
    if(btnBack()){returnToMenu(1);return;}
    delay(20);
  }
}
