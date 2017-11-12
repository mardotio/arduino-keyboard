#ifndef HIDKeyboard_H
#define HIDKeyboard_H

typedef int* IntPtr;
typedef long* LongPtr;
typedef char* CharPtr;

class HIDKeyboard {
  public:
    HIDKeyboard(long dbDelay, int numRows, int numColumns, int dataPin, int clkPin, int mdPin, bool led);
    void initialize(CharPtr *keys);
    int cycleOutputs();
    void pollMatrix(int onRow);
    void blinkLed(bool state);
    bool timer(long previousMillis, long delayLength);
    void abort();
    ~HIDKeyboard();
  private:
    int dataInPin;
    int clockPin;
    int modePin;
    static int columns;
    static int rows;
    static long debounceDelay;
    static int ledPin;
    bool ledPresent;
    static IntPtr rowPins;
    static CharPtr *keyMapping;
    static CharPtr *functionKeyMapping;
    static IntPtr *currentValues;
    static IntPtr *previousValues;
    static LongPtr *lastDebounce;
    void keyPress(int pressed, int row, int column);
    void checkOverflow();
    void resetVariables();
    int readData();
    void decodeData(int data, int onRow);
};

#endif
