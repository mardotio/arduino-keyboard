/*
Arduino Keyboard

  Program is written for the Arduino Micro/Leonardo, and can be used as an HID keyboard

The Circuit:

  - 22 buttons
  - 7 5-10 kΩ Resistors
  - 1 .5-1 kΩ Resistor
  - ATmega32u4 (bootloaded with Leonardo/Micro)
  - 16 MHz crystal
  - 1 LED
  - 1 Reset Button
  - 2 22 Ω Resistors
  - 2 22 pF Capacitors
  - 1 uF Capacitor

Written By: Mario Lopez
Last Revision: 3/26/2017
Version 0.0.3
*/


#include "HIDKeyboard.h"
#include "KeyboardVariables.h"

HIDKeyboard MyKeyboard(DEBOUNCEDELAY, ROWS, COLUMNS, DATAPIN, CLOCKPIN, MODEPIN, true);

void setup() {
  CharPtr *charArray = new CharPtr[ROWS];
  for(int i = 0; i < ROWS; i++){
    charArray[i] = new char[COLUMNS];
  }

  charArray[0][0] = ESC;
  charArray[0][1] = ONE;
  charArray[0][2] = TWO;
  charArray[0][3] = THREE;
  charArray[0][4] = FOUR;
  charArray[0][5] = FIVE;
  charArray[0][6] = SIX;
  charArray[0][7] = SEVEN;
  charArray[0][8] = EIGHT;
  charArray[0][9] = NINE;
  charArray[0][10] = ZERO;
  charArray[0][11] = DASH;
  charArray[0][12] = EQUALS;
  charArray[0][13] = BACKSPACE;

  charArray[1][0] = TAB;
  charArray[1][1] = KEY_Q;
  charArray[1][2] = KEY_W;
  charArray[1][3] = KEY_E;
  charArray[1][4] = KEY_R;
  charArray[1][5] = KEY_T;
  charArray[1][6] = KEY_Y;
  charArray[1][7] = KEY_U;
  charArray[1][8] = KEY_I;
  charArray[1][9] = KEY_O;
  charArray[1][10] = KEY_P;
  charArray[1][11] = LEFT_BRACKET;
  charArray[1][12] = RIGHT_BRACKET;
  charArray[1][13] = BACKSLASH;

  charArray[2][0] = CAPS_LOCK;
  charArray[2][1] = KEY_A;
  charArray[2][2] = KEY_S;
  charArray[2][3] = KEY_D;
  charArray[2][4] = KEY_F;
  charArray[2][5] = KEY_G;
  charArray[2][6] = KEY_H;
  charArray[2][7] = KEY_J;
  charArray[2][8] = KEY_K;
  charArray[2][9] = KEY_L;
  charArray[2][10] = SEMICOLON;
  charArray[2][11] = SINGLE_QUOTE;
  charArray[2][12] = RETURN;

  charArray[3][0] = LEFT_SHIFT;
  charArray[3][1] = KEY_Z;
  charArray[3][2] = KEY_X;
  charArray[3][3] = KEY_C;
  charArray[3][4] = KEY_V;
  charArray[3][5] = KEY_B;
  charArray[3][6] = KEY_N;
  charArray[3][7] = KEY_M;
  charArray[3][8] = COMMA;
  charArray[3][9] = PERIOD;
  charArray[3][10] = FORWARDSLASH;
  charArray[3][11] = RIGHT_SHIFT;

  charArray[4][0] = LEFT_CTRL;
  charArray[4][1] = LEFT_WIN;
  charArray[4][2] = LEFT_ALT;
  charArray[4][3] = SPACE;
  charArray[4][4] = RIGHT_ALT;
  charArray[4][5] = RIGHT_WIN;
  charArray[4][6] = BACK_TICK;
  charArray[4][7] = RIGHT_CTRL;

  MyKeyboard.initialize(charArray);
  for(int j = 0; j < ROWS; j++) {
    delete [] charArray[j];
  }
  delete [] charArray;
  charArray = NULL;

  pinMode(10, INPUT);
}

void loop() {
  /*The processor only needs to cycle the columns about once every second.
  Cycling the columns any faster is unnecessary as it is impossible to press
  a button fast enough. Furthermore, lowering the amount of cycles the
  processor completes per second can help keep the temperature of the chip
  low, which can in turn extend the life of the microcontroller*/
  static long lastCycle = millis();
  if(digitalRead(10) == HIGH) {
    if(MyKeyboard.timer(lastCycle, 1)) {
      int last = MyKeyboard.cycleOutputs();
      MyKeyboard.pollMatrix(last);
      lastCycle = millis();
    }
  } else {
    MyKeyboard.abort();
  }
}
