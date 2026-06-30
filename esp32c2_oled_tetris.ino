#include <U8g2lib.h>
#include <Wire.h>

// ================== ピン設定 ==================
#define PIN_LEFT    4
#define PIN_RIGHT   7
#define PIN_ROTATE 10
#define PIN_BOOT    9     // 基板上のBOOTボタン(GPIO9) / ゲーム中は下ボタン(ソフトドロップ)に
#define PIN_DRAIN   3     // 仮想GND
#define PIN_LED     8     // オンボードLED

// ================== U8g2 (40x72 縦長モード) ==================
U8G2_SSD1306_72X40_ER_F_HW_I2C u8g2(U8G2_R1, U8X8_PIN_NONE, /*SCL*/6, /*SDA*/5);

// ================== ゲーム定数 ==================
const int BOARD_W = 10;
const int BOARD_H = 18;
const int BLOCK_SIZE = 2;
const int GAP = 1;                            // 縦長なので隙間1pxを確保
const int CELL = BLOCK_SIZE + GAP;           // 1マス = 3ピクセル

// 盤面サイズ：横 30px、縦 54px
const int BOARD_X = 5;                        // 左右余白
const int BOARD_Y = 15;                       // 上部にUIスペースを確保

// テトリミノ定義 [7種類][4回転状態][4行][4列] (ゲームボーイ版準拠)
const uint8_t tetrominoes[7][4][4][4] = {

  // 0: Iミノ (水平 ➔ 垂直 の2状態往復)
  {
    {
      {0, 0, 0, 0},
      {1, 1, 1, 1},
      {0, 0, 0, 0},
      {0, 0, 0, 0}
    },
    {
      {0, 0, 1, 0},
      {0, 0, 1, 0},
      {0, 0, 1, 0},
      {0, 0, 1, 0}
    },
    {
      {0, 0, 0, 0},
      {1, 1, 1, 1},
      {0, 0, 0, 0},
      {0, 0, 0, 0}
    },
    {
      {0, 0, 1, 0},
      {0, 0, 1, 0},
      {0, 0, 1, 0},
      {0, 0, 1, 0}
    }
  },

  // 1: Oミノ (回転しても形・位置ともに固定)
  {
    {
      {0, 0, 0, 0},
      {0, 1, 1, 0},
      {0, 1, 1, 0},
      {0, 0, 0, 0}
    },
    {
      {0, 0, 0, 0},
      {0, 1, 1, 0},
      {0, 1, 1, 0},
      {0, 0, 0, 0}
    },
    {
      {0, 0, 0, 0},
      {0, 1, 1, 0},
      {0, 1, 1, 0},
      {0, 0, 0, 0}
    },
    {
      {0, 0, 0, 0},
      {0, 1, 1, 0},
      {0, 1, 1, 0},
      {0, 0, 0, 0}
    }
  },

  // 2: Tミノ (3x3内での時計回り回転)
  {
    {
      {0, 0, 0, 0},
      {1, 1, 1, 0},
      {0, 1, 0, 0},
      {0, 0, 0, 0}
    },
    {
      {0, 1, 0, 0},
      {1, 1, 0, 0},
      {0, 1, 0, 0},
      {0, 0, 0, 0}
    },
    {
      {0, 1, 0, 0},
      {1, 1, 1, 0},
      {0, 0, 0, 0},
      {0, 0, 0, 0}
    },
    {
      {0, 1, 0, 0},
      {0, 1, 1, 0},
      {0, 1, 0, 0},
      {0, 0, 0, 0}
    },
  },

  // 3: Jミノ (3x3内での時計回り回転)
  {
    {
      {0, 0, 0, 0},
      {1, 1, 1, 0},
      {0, 0, 1, 0},
      {0, 0, 0, 0}
    },
    {
      {0, 1, 0, 0},
      {0, 1, 0, 0},
      {1, 1, 0, 0},
      {0, 0, 0, 0}
    },
    {
      {1, 0, 0, 0},
      {1, 1, 1, 0},
      {0, 0, 0, 0},
      {0, 0, 0, 0}
    },
    {
      {0, 1, 1, 0},
      {0, 1, 0, 0},
      {0, 1, 0, 0},
      {0, 0, 0, 0}
    },
  },

  // 4: Lミノ (3x3内での時計回り回転)
  {
    {
      {0, 0, 0, 0},
      {1, 1, 1, 0},
      {1, 0, 0, 0},
      {0, 0, 0, 0}
    },
    {
      {1, 1, 0, 0},
      {0, 1, 0, 0},
      {0, 1, 0, 0},
      {0, 0, 0, 0}
    },
    {
      {0, 0, 1, 0},
      {1, 1, 1, 0},
      {0, 0, 0, 0},
      {0, 0, 0, 0}
    },
    {
      {0, 1, 0, 0},
      {0, 1, 0, 0},
      {0, 1, 1, 0},
      {0, 0, 0, 0}
    },
  },

  // 5: Sミノ (2状態往復・垂直時は左寄り)
  {
    {
      {0, 1, 1, 0},
      {1, 1, 0, 0},
      {0, 0, 0, 0},
      {0, 0, 0, 0}
    },
    {
      {1, 0, 0, 0},
      {1, 1, 0, 0},
      {0, 1, 0, 0},
      {0, 0, 0, 0}
    },
    {
      {0, 1, 1, 0},
      {1, 1, 0, 0},
      {0, 0, 0, 0},
      {0, 0, 0, 0}
    },
    {
      {1, 0, 0, 0},
      {1, 1, 0, 0},
      {0, 1, 0, 0},
      {0, 0, 0, 0}
    }
  },

  // 6: Zミノ (2状態往復・垂直時は左寄り)
  {
    {
      {1, 1, 0, 0},
      {0, 1, 1, 0},
      {0, 0, 0, 0},
      {0, 0, 0, 0}
    },
    {
      {0, 1, 0, 0},
      {1, 1, 0, 0},
      {1, 0, 0, 0},
      {0, 0, 0, 0}
    },
    {
      {1, 1, 0, 0},
      {0, 1, 1, 0},
      {0, 0, 0, 0},
      {0, 0, 0, 0}
    },
    {
      {0, 1, 0, 0},
      {1, 1, 0, 0},
      {1, 0, 0, 0},
      {0, 0, 0, 0}
    }
  }
};

// NEXTを描画するオフセット
const int32_t next_offset_y[7] = {/*I*/0,/*O*/1, /*T*/1, /*J*/1, /*L*/1,/*S*/0, /*Z*/0};

int board[BOARD_H][BOARD_W] = {0};

int currentPiece, nextPiece, rotation;
int posX, posY;
int score = 0, level = 0;
unsigned long dropTimer = 0;
unsigned long dropInterval = 500;
bool gameOver = false, started = false;
bool isClearingLines = false;                 // ★新規追加：ライン消去演出中フラグ

// 前方宣言
void showLogo();
void startGame();
void spawnNewPiece();
bool collision();
bool moveDown();
void lockPiece();
void clearLines();
void handleInput();
void drawAll();
void drawBoard();
void drawCurrentPiece();
void drawNext();
void drawUI();
void drawGameOver();

void setup() {
  pinMode(PIN_LEFT, INPUT_PULLUP);
  pinMode(PIN_RIGHT, INPUT_PULLUP);
  pinMode(PIN_ROTATE, INPUT_PULLUP);
  pinMode(PIN_BOOT, INPUT_PULLUP);
  pinMode(PIN_DRAIN, OUTPUT_OPEN_DRAIN);
  digitalWrite(PIN_DRAIN, LOW);
  
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, HIGH);

  u8g2.begin();
  u8g2.setBusClock(400000);
  u8g2.setFlipMode(0);
  u8g2.setContrast(60);
  u8g2.setFont(u8g2_font_5x7_tf);

  randomSeed(millis());
  showLogo();
}

void loop() {
  if (!started) {
    if (digitalRead(PIN_BOOT) == LOW) { delay(200); startGame(); }
    return;
  }
  if (gameOver) { drawGameOver(); return; }

  handleInput();

  bool down = (digitalRead(PIN_LEFT) == LOW && digitalRead(PIN_RIGHT) == LOW) || (digitalRead(PIN_BOOT) == LOW);
  unsigned long currentInterval = down ? 40 : dropInterval;

  if (millis() - dropTimer > currentInterval) {
    dropTimer = millis();
    if (!moveDown()) lockPiece();
  }
  drawAll();
  u8g2.sendBuffer();
  delay(20);
}

// ================== コア関数 ==================
void showLogo() {
  u8g2.clearBuffer();
  u8g2.drawStr(5, 15, "TETRIS");
  u8g2.drawStr(10, 35, "BOOT");
  u8g2.drawStr(2, 45, "to start");
  u8g2.sendBuffer();
}

void startGame() {
  memset(board, 0, sizeof(board));
  score = 0; level = 0;
  dropInterval = 500;
  nextPiece = random(7);
  spawnNewPiece();
  started = true; gameOver = false;
  dropTimer = millis();
}

void spawnNewPiece() {
  currentPiece = nextPiece;
  nextPiece = random(7);
  rotation = 0;
  posX = BOARD_W / 2 - 2;
  posY = 0;
  if (collision()) gameOver = true;
}

bool collision() {
  for (int y = 0; y < 4; y++) {
    for (int x = 0; x < 4; x++) {
      if (tetrominoes[currentPiece][rotation][y][x]) {
        int bx = posX + x;
        int by = posY + y;
        if (bx < 0 || bx >= BOARD_W || by >= BOARD_H) return true;
        if (by >= 0 && board[by][bx]) return true;
      }
    }
  }
  return false;
}

bool moveDown() {
  posY++;
  if (collision()) { posY--; return false; }
  return true;
}

void lockPiece() {
  for (int y = 0; y < 4; y++) {
    for (int x = 0; x < 4; x++) {
      if (tetrominoes[currentPiece][rotation][y][x]) {
        int by = posY + y;
        if (by >= 0) board[by][posX + x] = currentPiece + 1;
      }
    }
  }
  clearLines();
  spawnNewPiece();
}

void clearLines() {
  bool lineFull[BOARD_H] = {false};
  int lines = 0;

  for (int y = 0; y < BOARD_H; y++) {
    bool full = true;
    for (int x = 0; x < BOARD_W; x++) {
      if (!board[y][x]) { full = false; break; }
    }
    if (full) {
      lineFull[y] = true;
      lines++;
    }
  }

  if (lines == 0) return;

  // 点滅演出の開始フラグを立てる（これで落下中ミノの上書き描画をストップ）
  isClearingLines = true; 

  int tempBoard[BOARD_H][BOARD_W];
  memcpy(tempBoard, board, sizeof(board));

  for (int i = 0; i < 3; i++) {
    // 【消灯】
    for (int y = 0; y < BOARD_H; y++) {
      if (lineFull[y]) {
        for (int x = 0; x < BOARD_W; x++) board[y][x] = 0;
      }
    }
    digitalWrite(PIN_LED, HIGH); 
    drawAll();
    u8g2.sendBuffer();
    delay(100);                  

    // 【点灯】
    memcpy(board, tempBoard, sizeof(board));
    digitalWrite(PIN_LED, LOW);  
    drawAll();
    u8g2.sendBuffer();
    delay(100);                  
  }
  
  digitalWrite(PIN_LED, HIGH);   
  isClearingLines = false;       // 演出終了、フラグを戻す

  // 実際の消去処理
  for (int y = BOARD_H-1; y >= 0; y--) {
    bool full = true;
    for (int x = 0; x < BOARD_W; x++) {
      if (!board[y][x]) { full = false; break; }
    }
    if (full) {
      for (int yy = y; yy > 0; yy--) {
        for (int x = 0; x < BOARD_W; x++) board[yy][x] = board[yy-1][x];
      }
      memset(board[0], 0, BOARD_W * sizeof(int));
      y++;
    }
  }

  if (lines) {
    score += lines;
    if (score / 10 > level) {
      level = score / 10;
      dropInterval = max(100, 500 - level * 30);
    }
  }
}

void handleInput() {
  static unsigned long last = 0;
  if (millis() - last < 130) return;

  if (digitalRead(PIN_LEFT) == LOW)  { posX--; if (collision()) posX++; last = millis(); }
  if (digitalRead(PIN_RIGHT) == LOW) { posX++; if (collision()) posX--; last = millis(); }
  if (digitalRead(PIN_ROTATE) == LOW) {
    rotation = (rotation + 1) % 4;
    if (collision()) rotation = (rotation + 3) % 4;
    last = millis();
  }
}

// ================== 描画 ==================
void drawAll() {
  u8g2.clearBuffer();
  drawBoard();
  drawCurrentPiece();
  drawNext();
  drawUI();
}

void drawBoard() {
  u8g2.drawFrame(BOARD_X - 2, BOARD_Y - 1, BOARD_W * CELL + 3, BOARD_H * CELL + 2);

  for (int y = 0; y < BOARD_H; y++) {
    for (int x = 0; x < BOARD_W; x++) {
      if (board[y][x]) {
        int px = BOARD_X + x * CELL;
        int py = BOARD_Y + y * CELL;
        u8g2.drawBox(px, py, BLOCK_SIZE, BLOCK_SIZE);
      }
    }
  }
}

void drawCurrentPiece() {
  if (isClearingLines) return; // ★演出中は動いているミノの重複描画をスキップ

  for (int y = 0; y < 4; y++) {
    for (int x = 0; x < 4; x++) {
      if (tetrominoes[currentPiece][rotation][y][x]) {
        int px = BOARD_X + (posX + x) * CELL;
        int py = BOARD_Y + (posY + y) * CELL;
        u8g2.drawBox(px, py, BLOCK_SIZE, BLOCK_SIZE);
      }
    }
  }
}

void drawNext() {
  const int pos_y = 1;
  const int pos_x = 28;

  int offset_y = next_offset_y[nextPiece];
  for (int y = 0; y < 2; y++) {
    for (int x = 0; x < 4; x++) {
      if (tetrominoes[nextPiece][0][y + offset_y][x]) {
        u8g2.drawBox(pos_x + 1 + x * 3, pos_y + 1 + y * 3, 2, 2);
      }
    }
  }
}

void drawUI() {
  u8g2.setCursor(2, 9);
  u8g2.print("L:");
  u8g2.print(score);
}

void drawGameOver() {
  u8g2.clearBuffer();
  u8g2.drawStr(10, 15, "GAME");
  u8g2.drawStr(10, 25, "OVER");
  u8g2.setCursor(5, 45);
  u8g2.print("LINES:");
  u8g2.setCursor(10, 55);
  u8g2.print(score);
  u8g2.sendBuffer();
  
  if (digitalRead(PIN_BOOT) == LOW) { delay(200); started = false; }
}