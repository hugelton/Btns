// Btns 0.3 
// by Leo Kuroshita for Hügelton instruments.

#include "MonomeSerialDevice.h"
#include <TM1640.h>
#include <TM16xxMatrix.h>
#include <Arduino.h>
#include <Adafruit_TinyUSB.h>
#include <EEPROM.h>

#define NUM_ROWS 8
#define NUM_COLS 8

#define BRIGHTNESS 127

const uint8_t ROW_PINS[NUM_ROWS] = { 0, 1, 2, 3, 4, 5, 6, 7 };        // GPIO00-GPIO07
const uint8_t COL_PINS[NUM_COLS] = { 8, 9, 10, 11, 12, 13, 14, 15 };  // GPIO8-GPIO15

TM1640 module(18, 19);  // DIN=18, CLK=19
TM16xxMatrix matrix(&module, NUM_COLS, NUM_ROWS);

bool isInited = false;
String deviceID = "btns";
String serialNum = "m4216124";

char mfgstr[32] = "monome";
char prodstr[32] = "monome";
char serialstr[32] = "m4216124";

MonomeSerialDevice mdp;

bool buttonStates[NUM_ROWS][NUM_COLS] = { 0 };

int gridRotation = 0;  // 0, 1, 2, or 3 representing 0, 90, 180, 270 degrees
bool flipHorizontal = false;
bool flipVertical = false;

void mapButtonToLED(int buttonRow, int buttonCol, int &ledRow, int &ledCol) {
  switch (gridRotation) {
    case 0:
      ledRow = buttonRow;
      ledCol = buttonCol;
      break;
    case 1:
      ledRow = buttonCol;
      ledCol = NUM_ROWS - 1 - buttonRow;
      break;
    case 2:
      ledRow = NUM_ROWS - 1 - buttonRow;
      ledCol = NUM_COLS - 1 - buttonCol;
      break;
    case 3:
      ledRow = NUM_COLS - 1 - buttonCol;
      ledCol = buttonRow;
      break;
  }
  if (flipHorizontal) ledCol = NUM_COLS - 1 - ledCol;
  if (flipVertical) ledRow = NUM_ROWS - 1 - ledRow;
}

void detectGridOrientation() {
  // スキャンしてどのボタンが押されているか確認
  for (int col = 0; col < NUM_COLS; col++) {
    digitalWrite(COL_PINS[col], LOW);
    for (int row = 0; row < NUM_ROWS; row++) {
      if (!digitalRead(ROW_PINS[row])) {
        // This button is pressed, set it as (0,0)
        if (row == 0 && col == 0) {
          gridRotation = 0;
        } else if (row == 0 && col == NUM_COLS - 1) {
          gridRotation = 1;
        } else if (row == NUM_ROWS - 1 && col == NUM_COLS - 1) {
          gridRotation = 2;
        } else if (row == NUM_ROWS - 1 && col == 0) {
          gridRotation = 3;
        }
        // EEPROMに保存
        EEPROM.write(0, gridRotation);
        return;
      }
    }
    digitalWrite(COL_PINS[col], HIGH);
  }
}

void setup() {
  USBDevice.setManufacturerDescriptor(mfgstr);
  USBDevice.setProductDescriptor(prodstr);
  USBDevice.setSerialDescriptor(serialstr);

  pinMode(LED_BUILTIN, OUTPUT);

  for (int i = 0; i < NUM_ROWS; i++) {
    pinMode(ROW_PINS[i], INPUT_PULLUP);
  }
  for (int i = 0; i < NUM_COLS; i++) {
    pinMode(COL_PINS[i], OUTPUT);
    digitalWrite(COL_PINS[i], HIGH);
  }

  // EEPROMから回転情報を読み込む
  gridRotation = EEPROM.read(0);
  if (gridRotation > 3) gridRotation = 0;  // 無効な値の場合

  detectGridOrientation();  // 起動時の向き検出

  module.clearDisplay();
  matrix.setAll(false);
  module.setupDisplay(true, BRIGHTNESS);

  mdp.isMonome = true;
  mdp.deviceID = deviceID;
  mdp.setupAsGrid(NUM_ROWS, NUM_COLS);

  isInited = true;
  mdp.poll();

  // グリッドサイズと回転情報を送信
  mdp.sendSysSize();
  mdp.sendSysRotation();
}

void loop() {
  static unsigned long lastCheck = 0;
  unsigned long currentMillis = millis();

  mdp.poll();

  if (currentMillis - lastCheck >= 15) {
    lastCheck = currentMillis;
    scanButtonMatrix();
    updateLEDMatrix();
  }
}

void scanButtonMatrix() {
  for (int col = 0; col < NUM_COLS; col++) {
    digitalWrite(COL_PINS[col], LOW);
    for (int row = 0; row < NUM_ROWS; row++) {
      bool currentState = !digitalRead(ROW_PINS[row]);
      int ledRow, ledCol;
      mapButtonToLED(row, col, ledRow, ledCol);
      if (currentState != buttonStates[row][col]) {
        mdp.sendGridKey(ledCol, ledRow, currentState);
        buttonStates[row][col] = currentState;
      }
    }
    digitalWrite(COL_PINS[col], HIGH);
  }
}

void updateLEDMatrix() {
  for (int row = 0; row < NUM_ROWS; row++) {
    for (int col = 0; col < NUM_COLS; col++) {
      int ledRow, ledCol;
      mapButtonToLED(row, col, ledRow, ledCol);
      matrix.setPixel(col, row, mdp.leds[ledRow * NUM_COLS + ledCol] > 0);
    }
  }
}