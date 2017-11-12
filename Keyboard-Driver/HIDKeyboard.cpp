#include "Arduino.h"
#include "Keyboard.h"
#include "HIDKeyboard.h"
#include <stdarg.h>

int HIDKeyboard::columns;
int HIDKeyboard::rows;
long HIDKeyboard::debounceDelay;
int HIDKeyboard::ledPin;
IntPtr HIDKeyboard::rowPins;
CharPtr *HIDKeyboard::keyMapping;
IntPtr *HIDKeyboard::currentValues;
IntPtr *HIDKeyboard::previousValues;
LongPtr *HIDKeyboard::lastDebounce;

/*Constructor
Sets the column and row pins based on the number of columns
and rows set in the HIDKeyboard.h file - Rows will be set first, followed
by the columns

Due to our PCB design, pin 8 has to be skipped, so the constructor
handles this

This constructor also takes 3 arguments so that other characteristics
of the keyboard can be initialized

dbDelay - sets the delay that will be used to debounce each key
prtDelay - set minimum amount of time between each printed character
LED - if true, pin 13 is set to be the led pin
      if flase, no led pin is set*/
HIDKeyboard::HIDKeyboard(long dbDelay, int numRows, int numColumns, int dataPin, int clkPin, int mdPin, bool led) {

  rows = numRows;
  columns = numColumns;
  rowPins = new int[rows];
  dataInPin = dataPin;
  clockPin = clkPin;
  modePin = mdPin;

  keyMapping = new CharPtr[rows];
  currentValues = new IntPtr[rows];
  previousValues = new IntPtr[rows];
  lastDebounce = new LongPtr[rows];

  for(int i = 0; i < rows; i++) {
    keyMapping[i] = new char[columns];
    currentValues[i] = new int[columns];
    previousValues[i] = new int[columns];
    lastDebounce[i] = new long[columns];
  }
  for(int j = 0; j < rows; j++) {
    rowPins[j] = j + 5;
  }

  ledPresent = led;
  if(ledPresent) {
    ledPin = 13;
  } else {
    ledPin = -1;
  }
  debounceDelay = dbDelay;

  for(int k = 0; k < rows; k++) {
    for(int l = 0; l < columns; l++) {
      currentValues[k][l] = 0;
      previousValues[k][l] = 0;
      lastDebounce[k][l] = 0;
    }
  }
}

/*This function should only be called withing the setup() function in the main file.
It sets the appropriate pins to either inputs or outputs. An array of characters
needs to be passed to the function in order for it to set the key mapping of the
keyboard.*/
void HIDKeyboard::initialize(CharPtr *keys) {
  for(int i = 0; i < rows; i++) {
    pinMode(rowPins[i], OUTPUT);
    digitalWrite(rowPins[i], LOW);
  }

  if(ledPresent) {
    pinMode(ledPin, OUTPUT);
    digitalWrite(ledPin, LOW);
  }

  for(int k = 0; k < rows; k++) {
    for(int l = 0; l < columns; l++) {
      keyMapping[k][l] = keys[k][l];
    }
  }

  pinMode(dataInPin, INPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(modePin, OUTPUT);
  digitalWrite(clockPin, LOW);
  digitalWrite(modePin, LOW);

  Keyboard.begin();
}

/*Turns the column that is HIGH LOW and turns the next column HIGH
Returns the last column that was HIGH*/
int HIDKeyboard::cycleOutputs() {
  static int lastRow = 0;
  if(lastRow == (rows - 1)) {
    digitalWrite(rowPins[lastRow], LOW);
    lastRow = 0;
    digitalWrite(rowPins[lastRow], HIGH);
  }
  else{
    digitalWrite(rowPins[lastRow], LOW);
    lastRow = lastRow + 1;
    digitalWrite(rowPins[lastRow], HIGH);
  }
  return lastRow;
}

/*Checks the value of each row in order to determine if a button has been pressed
or released. The index of the column that is currently HIGH needs to be passed to the
function so that it can determine if the correct key to press*/
void HIDKeyboard::pollMatrix(int onRow) {
  checkOverflow();
  blinkLed(false);
  decodeData(readData(), onRow);
}

/*Presses or releases the key that is passed to it. This function takes 3 arguments.
Pressed, which is either HIGH or LOW and it is the current value of any given column.
Row, which is the index of the row that is currently being checked. Column, which is
the index of the column that is currently HIGH*/
void HIDKeyboard::keyPress(int pressed, int row, int column) {
  blinkLed(true);
  if(pressed) {
    Keyboard.press(keyMapping[row][column]);
  } else {
    Keyboard.release(keyMapping[row][column]);
  }
}

/*Checks to see if millis() function has overflown.
millis() overfolows in about 50 days.
if millis() has overflown it calls reset function*/
void HIDKeyboard::checkOverflow() {
  static long errorCheck = millis();
  if(millis() < errorCheck) {
    resetVariables();
    errorCheck = millis();
  }
}

/*Sets all of the values of the arrays to zero in order to avoid
any potential errors that may occur due to the millis() function overflowing.
Additionally, any keys that are pressed at the time of the function call will
be released.*/
void HIDKeyboard::resetVariables() {
  for(int i = 0; i < rows; i++) {
    for(int j = 0; j < columns; j++) {
      currentValues[i][j] = 0;
      previousValues[i][j] = 0;
      lastDebounce[i][j] = 0;
    }
  }
  Keyboard.releaseAll();
}

/*Blinks the LED if state is not 0
Turns off the LED if state is 0.*/
void HIDKeyboard::blinkLed(bool state) {
  if(ledPresent) {
    static int currentState = LOW;
    static long ledTimer = millis();
    if(state && currentState == LOW) {
       digitalWrite(ledPin, HIGH);
       currentState = HIGH;
       ledTimer = millis();
    } else if(state && currentState == HIGH) {
      ledTimer = millis();
    } else if(!state && timer(ledTimer, 50)) {
      digitalWrite(ledPin, LOW);
      currentState = LOW;
    }
  }
}

/*Timer function that is used to check if a certain amount of milliseconds
has passed.*/
bool HIDKeyboard::timer(long previousMillis, long delayLength) {
  return ((millis() - previousMillis) >= delayLength);
}

/*
Serially reads the data from the column/row pins
*/
int HIDKeyboard::readData() {
  int bitIn = 0;
  int dataIn = 0;

  digitalWrite(clockPin, LOW);
  digitalWrite(modePin, HIGH);
  digitalWrite(clockPin, HIGH);
  digitalWrite(modePin, LOW);

  for(int i = 0; i < columns; i++) {
    digitalWrite(clockPin, LOW);
    bitIn = digitalRead(dataInPin);
    if(bitIn) {
      dataIn = dataIn | (1 << i);
    }
    digitalWrite(clockPin, HIGH);
  }
  return dataIn;
}

/*
Takes in a byte of data and decodes it to determine which rows/columns are
currently active, and stores these values into the currentValues matrix.
*/
void HIDKeyboard::decodeData(int data, int onRow) {
  int temp;
  for(int i = 0; i < columns; i++) {
    previousValues[onRow][i] = currentValues[onRow][i];
  }

  for(int j = 0; j < columns; j++) {
    temp = 1 & (data >> j);
    if(temp != previousValues[onRow][j])
      if(timer(lastDebounce[onRow][j], debounceDelay)) {
        currentValues[onRow][j] = temp;
        lastDebounce[onRow][j] = millis();
        keyPress(currentValues[onRow][j], onRow, j);
      }
  }
}

void HIDKeyboard::abort() {
  Keyboard.releaseAll();
}

/*Destructor for the HIDKeyboard class. Deletes any dynamically
allocated data.*/
HIDKeyboard::~HIDKeyboard() {
  for(int i = 0; i < rows; i++) {
    delete [] keyMapping[i];
    delete [] currentValues[i];
    delete [] previousValues[i];
    delete [] lastDebounce[i];
  }
  delete [] keyMapping;
  delete [] currentValues;
  delete [] previousValues;
  delete [] lastDebounce;
  delete [] rowPins;
  keyMapping = NULL;
  currentValues = NULL;
  previousValues = NULL;
  lastDebounce = NULL;
  rowPins = NULL;
}
