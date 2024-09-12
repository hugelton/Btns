// Btns 0.4
// by Leo Kuroshita for Hügelton instruments.

#include "MonomeSerialDevice.h"
#include <TM1640.h>
#include <TM16xxMatrix.h>
#include <Arduino.h>
#include <Adafruit_TinyUSB.h>

#define NUM_ROWS 8
#define NUM_COLS 8

#define BRIGHTNESS 127

#define DEFAULT_ROTATION 0
#define DEFAULT_FLIP_HORIZONTAL false
#define DEFAULT_FLIP_VERTICAL false



const uint8_t ROW_PINS[NUM_ROWS] = {0, 1, 2, 3, 4, 5, 6, 7};  // GPIO00-GPIO07
const uint8_t COL_PINS[NUM_COLS] = {8, 9, 10, 11, 12, 13, 14, 15};  // GPIO8-GPIO15

TM1640 module(18, 19);  // DIN=18, CLK=19
TM16xxMatrix matrix(&module, NUM_COLS, NUM_ROWS);

bool isInited = false;
String deviceID = "btns";
String serialNum = "m4216124";

char mfgstr[32] = "monome";
char prodstr[32] = "monome";
char serialstr[32] = "m4216124";

MonomeSerialDevice mdp;

bool buttonStates[NUM_ROWS][NUM_COLS] = {0};


int gridRotation = DEFAULT_ROTATION;
bool flipHorizontal = DEFAULT_FLIP_HORIZONTAL;
bool flipVertical = DEFAULT_FLIP_VERTICAL;
void mapButtonToLED(int buttonRow, int buttonCol, int &ledRow, int &ledCol) {
    // ボタンの行と列を0-7の範囲で扱う
    ledRow = buttonRow;
    ledCol = buttonCol;

    // 回転を適用
    switch (gridRotation) {
        case 0: // 回転なし
            break;
        case 1: // 90度回転
            {
                int temp = ledRow;
                ledRow = ledCol;
                ledCol = NUM_COLS - 1 - temp;
            }
            break;
        case 2: // 180度回転
            ledRow = NUM_ROWS - 1 - ledRow;
            ledCol = NUM_COLS - 1 - ledCol;
            break;
        case 3: // 270度回転
            {
                int temp = ledRow;
                ledRow = NUM_ROWS - 1 - ledCol;
                ledCol = temp;
            }
            break;
    }

    // 反転を適用
    if (flipHorizontal) ledCol = NUM_COLS - 1 - ledCol;
    if (flipVertical) ledRow = NUM_ROWS - 1 - ledRow;
}

bool detectGridOrientation() {
    bool buttonPressed = false;
    // スキャンしてどのボタンが押されているか確認
    for (int col = 0; col < NUM_COLS; col++) {
        digitalWrite(COL_PINS[col], LOW);
        for (int row = 0; row < NUM_ROWS; row++) {
            if (!digitalRead(ROW_PINS[row])) {
                buttonPressed = true;
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
                digitalWrite(COL_PINS[col], HIGH);
                return buttonPressed;
            }
        }
        digitalWrite(COL_PINS[col], HIGH);
    }
    // If no button is pressed, set orientation as if btn1 (top-left) was pressed
    gridRotation = 0;
    return buttonPressed;
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

    detectGridOrientation(); // This will set gridRotation to 0 if no button is pressed

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
    for (int row = 0; row < NUM_ROWS; row++) {
        for (int col = 0; col < NUM_COLS; col++) {
            digitalWrite(COL_PINS[col], LOW);
            bool currentState = !digitalRead(ROW_PINS[row]);
            digitalWrite(COL_PINS[col], HIGH);

            int ledRow, ledCol;
            mapButtonToLED(row, col, ledRow, ledCol);

            if (currentState != buttonStates[row][col]) {
                mdp.sendGridKey(ledCol, ledRow, currentState);
                buttonStates[row][col] = currentState;
            }
        }
    }
}

void updateLEDMatrix() {
    for (int row = 0; row < NUM_ROWS; row++) {
        for (int col = 0; col < NUM_COLS; col++) {
            int ledRow, ledCol;
            mapButtonToLED(row, col, ledRow, ledCol);
            matrix.setPixel(ledCol, ledRow, mdp.leds[row * NUM_COLS + col] > 0);
        }
    }
}
