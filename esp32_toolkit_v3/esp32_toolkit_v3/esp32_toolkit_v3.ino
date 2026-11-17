/*
  ESP32 + 3.5" TFT (ILI9486 480x320) + 4x4 Tus Matrisi
  -------------------------------------------------------
  Flipper-tarzi liste menu + birden fazla oyun.
  Dokunmatik KULLANILMIYOR, sadece tus matrisi.

  Gerekli:
    - TFT_eSPI kutuphanesi (ve ayni sketch icinde "tft_setup.h" dosyasi)

  Kart secimi (Tools > Board): "ESP32 Dev Module"

  ONEMLI: Asagidaki KEY_UP / KEY_DOWN / KEY_OK / KEY_BACK
  degerleri varsayilan. Once menudeki "Tus Testi" ile hangi
  fiziksel tusun hangi numarayi verdigini gorup, gerekirse
  bu 4 degeri degistir.
*/

#include <SPI.h>
#include <TFT_eSPI.h>

TFT_eSPI tft = TFT_eSPI();

// ---------- TUS MATRISI PINLERI ----------
const int rowPins[4] = {25, 26, 27, 14}; // R1 R2 R3 R4
const int colPins[4] = {32, 33, 21, 22}; // C1 C2 C3 C4

// ---------- FONKSIYON TUS NUMARALARI (gerekirse degistir) ----------
int KEY_UP   = 1;
int KEY_DOWN = 2;
int KEY_OK   = 3;
int KEY_BACK = 4;

// ---------- RENKLER ----------
#define BG      TFT_BLACK
#define FG      TFT_WHITE
#define HILITE  TFT_GREEN
#define ACCENT  TFT_CYAN
#define BOXBG   0x18C3

// ---------- EKRANLAR ----------
enum Screen { MENU, SNAKE_GAME, PONG_GAME, FLAPPY_GAME, BRICKS_GAME, KEY_TEST, ABOUT };
Screen currentScreen = MENU;

const char* menuItems[] = {"Yilan", "Pinpon", "Flappy Kus", "Tuglalar", "Tus Testi", "Hakkinda"};
const int menuCount = 6;
int menuIndex = 0;

// ---------- TUS MATRISI OKUMA ----------
unsigned long lastPress = 0;
const unsigned long debounceMs = 160;

int readKeyRaw() {
  for (int r = 0; r < 4; r++) {
    digitalWrite(rowPins[r], LOW);
    for (int c = 0; c < 4; c++) {
      if (digitalRead(colPins[c]) == LOW) {
        digitalWrite(rowPins[r], HIGH);
        return r * 4 + c + 1; // 1..16
      }
    }
    digitalWrite(rowPins[r], HIGH);
  }
  return 0;
}

// Debounce'lu, "bir kere basildi" bilgisi veren fonksiyon
int lastKey = 0;
int getKeyPress() {
  int k = readKeyRaw();
  if (k != 0 && millis() - lastPress > debounceMs) {
    lastPress = millis();
    lastKey = k;
    return k;
  }
  return 0;
}

bool keyIsHeld(int keyNum) {
  return readKeyRaw() == keyNum;
}

// ========================================================
void setup() {
  Serial.begin(115200);

  for (int i = 0; i < 4; i++) {
    pinMode(rowPins[i], OUTPUT);
    digitalWrite(rowPins[i], HIGH);
  }
  for (int i = 0; i < 4; i++) {
    pinMode(colPins[i], INPUT_PULLUP);
  }

  tft.init();
  tft.setRotation(1); // yatay mod
  tft.fillScreen(BG);
  randomSeed(analogRead(0));

  drawMenu();
}

void loop() {
  switch (currentScreen) {
    case MENU:        loopMenu(); break;
    case SNAKE_GAME:  loopSnake(); break;
    case PONG_GAME:   loopPong(); break;
    case FLAPPY_GAME: loopFlappy(); break;
    case BRICKS_GAME: loopBricks(); break;
    case KEY_TEST:    loopKeyTest(); break;
    case ABOUT:       loopAbout(); break;
  }
}

// ---------------- ANA MENU ----------------
void drawMenu() {
  tft.fillScreen(BG);
  tft.setTextSize(2);
  tft.setCursor(10, 8);
  tft.setTextColor(ACCENT, BG);
  tft.println("ANA MENU");
  tft.drawFastHLine(0, 34, tft.width(), FG);

  int boxH = 34;
  int gap = 6;
  int startY = 42;
  for (int i = 0; i < menuCount; i++) {
    int y = startY + i * (boxH + gap);
    uint16_t boxColor = (i == menuIndex) ? HILITE : BOXBG;
    tft.drawRoundRect(10, y, tft.width() - 20, boxH, 6, boxColor);
    tft.setTextColor(i == menuIndex ? HILITE : FG, BG);
    tft.setTextSize(2);
    tft.setCursor(25, y + 9);
    tft.println(menuItems[i]);
  }
}

void loopMenu() {
  int k = getKeyPress();
  if (k == KEY_UP)   { menuIndex = (menuIndex - 1 + menuCount) % menuCount; drawMenu(); }
  if (k == KEY_DOWN) { menuIndex = (menuIndex + 1) % menuCount; drawMenu(); }
  if (k == KEY_OK) {
    if (menuIndex == 0) { currentScreen = SNAKE_GAME; initSnake(); }
    if (menuIndex == 1) { currentScreen = PONG_GAME; initPong(); }
    if (menuIndex == 2) { currentScreen = FLAPPY_GAME; initFlappy(); }
    if (menuIndex == 3) { currentScreen = BRICKS_GAME; initBricks(); }
    if (menuIndex == 4) { currentScreen = KEY_TEST; drawKeyTest(); }
    if (menuIndex == 5) { currentScreen = ABOUT; drawAbout(); }
  }
}

// ---------------- TUS TESTI ----------------
void drawKeyTest() {
  tft.fillScreen(BG);
  tft.setTextSize(2);
  tft.setTextColor(ACCENT, BG);
  tft.setCursor(10, 10);
  tft.println("TUS TESTI");
  tft.drawFastHLine(0, 35, tft.width(), FG);
  tft.setTextSize(1);
  tft.setTextColor(FG, BG);
  tft.setCursor(10, 50);
  tft.println("Herhangi bir tusa bas,");
  tft.setCursor(10, 65);
  tft.println("numarasi asagida gorunecek.");
  tft.setCursor(10, 85);
  tft.println("Cikmak icin: BACK tusunu (varsayilan 4) dene.");
}

void loopKeyTest() {
  int k = getKeyPress();
  if (k != 0) {
    tft.fillRect(0, 110, tft.width(), 100, BG);
    tft.setTextSize(6);
    tft.setTextColor(HILITE, BG);
    tft.setCursor(40, 120);
    tft.print("Tus: ");
    tft.println(k);
    if (k == 4) { currentScreen = MENU; drawMenu(); } // sabit cikis, BACK henuz dogrulanmadigi icin
  }
}

// ---------------- HAKKINDA ----------------
void drawAbout() {
  tft.fillScreen(BG);
  tft.setTextSize(2);
  tft.setTextColor(ACCENT, BG);
  tft.setCursor(10, 10);
  tft.println("HAKKINDA");
  tft.drawFastHLine(0, 35, tft.width(), FG);
  tft.setTextSize(1);
  tft.setTextColor(FG, BG);
  tft.setCursor(10, 55);
  tft.println("ESP32 + TFT Oyun Menusu");
  tft.setCursor(10, 75);
  tft.println("Kendi yapim, deneysel amacli.");
  tft.setCursor(10, tft.height() - 20);
  tft.println("BACK: Menuye don");
}

void loopAbout() {
  if (getKeyPress() == KEY_BACK) { currentScreen = MENU; drawMenu(); }
}

// ================= YILAN (SNAKE) =================
#define GRID 10
int gridW, gridH;
int snakeX[300], snakeY[300];
int snakeLen, dirX, dirY, foodX, foodY;
unsigned long lastSnakeMove;
bool snakeOver;

void initSnake() {
  gridW = tft.width() / GRID;
  gridH = tft.height() / GRID;
  snakeLen = 3;
  for (int i = 0; i < snakeLen; i++) { snakeX[i] = 10 - i; snakeY[i] = 10; }
  dirX = 1; dirY = 0;
  foodX = random(0, gridW); foodY = random(0, gridH);
  snakeOver = false;
  lastSnakeMove = millis();
  tft.fillScreen(BG);
}

void loopSnake() {
  int k = getKeyPress();
  if (k == KEY_BACK) { currentScreen = MENU; drawMenu(); return; }
  if (snakeOver) { if (k == KEY_OK) initSnake(); return; }

  if (k == KEY_UP    && dirY == 0) { dirX = 0; dirY = -1; }
  if (k == KEY_DOWN  && dirY == 0) { dirX = 0; dirY = 1; }
  if (k == KEY_OK    && dirX == 0) { dirX = 1; dirY = 0; }

  if (millis() - lastSnakeMove < 130) return;
  lastSnakeMove = millis();

  for (int i = snakeLen - 1; i > 0; i--) { snakeX[i] = snakeX[i-1]; snakeY[i] = snakeY[i-1]; }
  snakeX[0] += dirX; snakeY[0] += dirY;

  if (snakeX[0] < 0 || snakeX[0] >= gridW || snakeY[0] < 0 || snakeY[0] >= gridH) snakeOver = true;
  for (int i = 1; i < snakeLen; i++) if (snakeX[0] == snakeX[i] && snakeY[0] == snakeY[i]) snakeOver = true;

  if (snakeOver) {
    tft.fillScreen(BG);
    tft.setTextSize(3);
    tft.setTextColor(TFT_RED, BG);
    tft.setCursor(60, 130);
    tft.println("OYUN BITTI");
    tft.setTextSize(1);
    tft.setTextColor(FG, BG);
    tft.setCursor(60, 170);
    tft.println("OK: Tekrar   BACK: Menu");
    return;
  }

  if (snakeX[0] == foodX && snakeY[0] == foodY) {
    snakeLen++;
    foodX = random(0, gridW); foodY = random(0, gridH);
  }

  tft.fillScreen(BG);
  for (int i = 0; i < snakeLen; i++)
    tft.fillRect(snakeX[i] * GRID, snakeY[i] * GRID, GRID - 1, GRID - 1, HILITE);
  tft.fillRect(foodX * GRID, foodY * GRID, GRID - 1, GRID - 1, TFT_RED);
}

// ================= PINPON (PONG) =================
int paddleY, ballX, ballY, ballDX, ballDY, pongScore;
bool pongOver;
unsigned long lastPongMove;

void initPong() {
  paddleY = tft.height() / 2 - 20;
  ballX = tft.width() / 2; ballY = tft.height() / 2;
  ballDX = -3; ballDY = 2;
  pongScore = 0; pongOver = false;
  lastPongMove = millis();
  tft.fillScreen(BG);
}

void loopPong() {
  int k = getKeyPress();
  if (k == KEY_BACK) { currentScreen = MENU; drawMenu(); return; }
  if (pongOver) { if (k == KEY_OK) initPong(); return; }

  if (keyIsHeld(KEY_UP))   paddleY -= 4;
  if (keyIsHeld(KEY_DOWN)) paddleY += 4;
  paddleY = constrain(paddleY, 0, tft.height() - 40);

  if (millis() - lastPongMove < 18) return;
  lastPongMove = millis();

  ballX += ballDX; ballY += ballDY;
  if (ballY <= 0 || ballY >= tft.height() - 6) ballDY = -ballDY;
  if (ballX >= tft.width() - 6) ballDX = -ballDX;

  if (ballX <= 14) {
    if (ballY >= paddleY && ballY <= paddleY + 40) { ballDX = -ballDX; pongScore++; }
    else pongOver = true;
  }

  if (pongOver) {
    tft.fillScreen(BG);
    tft.setTextSize(3);
    tft.setTextColor(TFT_RED, BG);
    tft.setCursor(80, 120);
    tft.println("KAYBETTIN");
    tft.setTextSize(2);
    tft.setTextColor(FG, BG);
    tft.setCursor(100, 160);
    tft.print("Skor: "); tft.println(pongScore);
    tft.setTextSize(1);
    tft.setCursor(80, 200);
    tft.println("OK: Tekrar   BACK: Menu");
    return;
  }

  tft.fillScreen(BG);
  tft.fillRect(8, paddleY, 6, 40, HILITE);
  tft.fillRect(ballX, ballY, 6, 6, ACCENT);
  tft.setTextSize(1);
  tft.setTextColor(FG, BG);
  tft.setCursor(tft.width() - 70, 5);
  tft.print("Skor:"); tft.print(pongScore);
}

// ================= FLAPPY KUS =================
float birdY, birdV;
int pipeX, gapY;
const int gapSize = 70;
int flapScore;
bool flapOver;
unsigned long lastFlapMove;

void initFlappy() {
  birdY = tft.height() / 2; birdV = 0;
  pipeX = tft.width();
  gapY = random(40, tft.height() - 40 - gapSize);
  flapScore = 0; flapOver = false;
  lastFlapMove = millis();
  tft.fillScreen(BG);
}

void loopFlappy() {
  int k = getKeyPress();
  if (k == KEY_BACK) { currentScreen = MENU; drawMenu(); return; }
  if (flapOver) { if (k == KEY_OK) initFlappy(); return; }
  if (k == KEY_OK) birdV = -4.5;

  if (millis() - lastFlapMove < 25) return;
  lastFlapMove = millis();

  birdV += 0.35;
  birdY += birdV;
  pipeX -= 3;

  if (pipeX < -20) {
    pipeX = tft.width();
    gapY = random(40, tft.height() - 40 - gapSize);
    flapScore++;
  }

  bool hitPipe = (pipeX < 30 && pipeX > 0) &&
                 (birdY < gapY || birdY > gapY + gapSize);
  bool hitEdge = (birdY < 0 || birdY > tft.height());

  if (hitPipe || hitEdge) flapOver = true;

  if (flapOver) {
    tft.fillScreen(BG);
    tft.setTextSize(3);
    tft.setTextColor(TFT_RED, BG);
    tft.setCursor(70, 120);
    tft.println("CARPTIN!");
    tft.setTextSize(2);
    tft.setTextColor(FG, BG);
    tft.setCursor(100, 160);
    tft.print("Skor: "); tft.println(flapScore);
    tft.setTextSize(1);
    tft.setCursor(80, 200);
    tft.println("OK: Tekrar   BACK: Menu");
    return;
  }

  tft.fillScreen(BG);
  tft.fillRect(0, 0, 20, gapY, TFT_DARKGREEN);
  tft.fillRect(0, gapY + gapSize, 20, tft.height() - (gapY + gapSize), TFT_DARKGREEN);
  // pipeX kullanarak boru konumunu ciz (sabit x=10, gorsel olarak sola kayan boru efekti)
  tft.fillRect(pipeX, 0, 20, gapY, TFT_DARKGREEN);
  tft.fillRect(pipeX, gapY + gapSize, 20, tft.height() - (gapY + gapSize), TFT_DARKGREEN);
  tft.fillCircle(40, (int)birdY, 8, ACCENT);
  tft.setTextSize(1);
  tft.setTextColor(FG, BG);
  tft.setCursor(tft.width() - 70, 5);
  tft.print("Skor:"); tft.print(flapScore);
}

// ================= TUGLALAR (BREAKOUT) =================
int brickPaddleX;
int brPaddleW = 50;
float brBallX, brBallY, brBallDX, brBallDY;
bool bricks[6][8];
int brickW, brickH;
int brScore;
bool brOver, brWin;

void initBricks() {
  brickPaddleX = tft.width() / 2 - brPaddleW / 2;
  brBallX = tft.width() / 2; brBallY = tft.height() - 30;
  brBallDX = 3; brBallDY = -3;
  brScore = 0; brOver = false; brWin = false;
  brickW = tft.width() / 8;
  brickH = 14;
  for (int r = 0; r < 6; r++) for (int c = 0; c < 8; c++) bricks[r][c] = true;
  tft.fillScreen(BG);
}

void loopBricks() {
  int k = getKeyPress();
  if (k == KEY_BACK) { currentScreen = MENU; drawMenu(); return; }
  if (brOver || brWin) { if (k == KEY_OK) initBricks(); return; }

  if (keyIsHeld(KEY_UP))   brickPaddleX -= 5;
  if (keyIsHeld(KEY_DOWN)) brickPaddleX += 5;
  brickPaddleX = constrain(brickPaddleX, 0, tft.width() - brPaddleW);

  brBallX += brBallDX; brBallY += brBallDY;
  if (brBallX <= 0 || brBallX >= tft.width() - 6) brBallDX = -brBallDX;
  if (brBallY <= 0) brBallDY = -brBallDY;

  int paddleTopY = tft.height() - 12;
  if (brBallY >= paddleTopY - 6 && brBallY <= paddleTopY &&
      brBallX >= brickPaddleX && brBallX <= brickPaddleX + brPaddleW) {
    brBallDY = -abs(brBallDY);
  }
  if (brBallY > tft.height()) brOver = true;

  // tugla carpismasi
  int br = (int)(brBallY - 20) / brickH;
  int bc = (int)brBallX / brickW;
  if (br >= 0 && br < 6 && bc >= 0 && bc < 8 && bricks[br][bc]) {
    bricks[br][bc] = false;
    brBallDY = -brBallDY;
    brScore++;
  }

  bool anyLeft = false;
  for (int r = 0; r < 6; r++) for (int c = 0; c < 8; c++) if (bricks[r][c]) anyLeft = true;
  if (!anyLeft) brWin = true;

  if (brOver || brWin) {
    tft.fillScreen(BG);
    tft.setTextSize(3);
    tft.setTextColor(brWin ? HILITE : TFT_RED, BG);
    tft.setCursor(60, 120);
    tft.println(brWin ? "KAZANDIN!" : "OYUN BITTI");
    tft.setTextSize(2);
    tft.setTextColor(FG, BG);
    tft.setCursor(100, 160);
    tft.print("Skor: "); tft.println(brScore);
    tft.setTextSize(1);
    tft.setCursor(80, 200);
    tft.println("OK: Tekrar   BACK: Menu");
    return;
  }

  tft.fillScreen(BG);
  for (int r = 0; r < 6; r++) {
    for (int c = 0; c < 8; c++) {
      if (bricks[r][c]) {
        uint16_t col = (r % 2 == 0) ? ACCENT : HILITE;
        tft.fillRect(c * brickW + 1, 20 + r * brickH, brickW - 2, brickH - 2, col);
      }
    }
  }
  tft.fillRect(brickPaddleX, tft.height() - 12, brPaddleW, 6, FG);
  tft.fillCircle((int)brBallX, (int)brBallY, 4, TFT_RED);
  tft.setTextSize(1);
  tft.setTextColor(FG, BG);
  tft.setCursor(tft.width() - 70, 5);
  tft.print("Skor:"); tft.print(brScore);
}
