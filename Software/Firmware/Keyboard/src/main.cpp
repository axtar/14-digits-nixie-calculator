// Nixie Calculator Keyboard Firmware
// Version 0.1.3 (beta)

// Copyright (C) 2021 highvoltglow
// Licensed under the MIT License


// the keyboard receives commands from the controller using I2C
// keyboard events are sent to the controller using a serial interface

// includes
#include <Arduino.h>
#include <SoftwareSerial.h>
#include <Keypad.h>
#include <Wire.h>

// version information
#define VERSION_MAJOR 0
#define VERSION_MINOR 1
#define VERSION_REVISION 3

// I2C address
#define I2C_ADDRESS 2

// commands
#define CMDINDENTIFIER '@'
#define CMD_RESET 1
#define CMD_GETVERSION 2
#define CMD_SETHOLDTIME 3
#define CMD_SETDEBOUNCETIME 4
#define CMD_SETAUTOREPEATINTERVAL 5

// keys arranged in 7 x 5 matrix
#define ROWS ((const byte)7)
#define COLS ((const byte)5)

// pins
#define PIN_KINT 17
#define PIN_DUMMY_RX 2

// this is an additional KeyState used
// for generated repeat events
#define KEYSTATE_AUTOREPEAT 4;

typedef struct
{
  uint8_t keyCode;
  unsigned long holdTimestamp;
} HOLD_INFO;

// key values
char keys[ROWS][COLS] = {{1, 2, 3, 4, 5},
                         {6, 7, 8, 9, 10},
                         {11, 12, 13, 14, 15},
                         {16, 17, 18, 19, 20},
                         {21, 22, 23, 24, 25},
                         {26, 27, 28, 29, 30},
                         {31, 32, 33, 34, 35}};

// matrix pins
byte rowPins[ROWS] = {16, 15, 14, 13, 7, 6, 5};
byte colPins[COLS] = {12, 11, 10, 9, 8};

// keypad instance
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// serial interface for keyboard events
SoftwareSerial kSerial(PIN_DUMMY_RX, PIN_KINT);

// key hold timestamp array for auto repeat function
HOLD_INFO keyHoldInfo[LIST_MAX];

volatile uint holdTime = 1000;
volatile uint debounceTime = 10;
volatile uint autoRepeatInterval = 0;
volatile int pendingRequest;

// forward declarations
void (*reset)(void) = 0;
void receiveEvent(int count);
void requestEvent();
void onReset();
void onGetVersion();
void onSetHoldTime();
void onSetDebounceTime();
void onSetAutoRepeatInterval();
void deleteKeyHoldInfo(uint8_t keyCode);
void setKeyHoldInfo(uint8_t keyCode);
void initKeyHoldInfo();

void setup()
{
  //Serial.begin(9600);

  // init serial connection
  kSerial.begin(9600);

  // init the hold timestamp table
  initKeyHoldInfo();

  // set default values
  keypad.setHoldTime(1000);
  keypad.setDebounceTime(10);
  pendingRequest = -1;

  // init I2C connection
  Wire.begin(I2C_ADDRESS);
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);
}


void loop()
{
  uint8_t buffer[2];

  // check for keys
  if (keypad.getKeys())
  {
    for (uint8_t i = 0; i < LIST_MAX; i++)
    {
      if (keypad.key[i].stateChanged)
      {
        // state changed, send key and state
        buffer[0] = keypad.key[i].kchar;
        buffer[1] = keypad.key[i].kstate;
        kSerial.write(buffer, 2);

        // set/delete hold timestamp
        if (autoRepeatInterval > 0)
        {
          switch (keypad.key[i].kstate)
          {
          case KeyState::HOLD:
            setKeyHoldInfo((uint8_t)keypad.key[i].kchar);
            break;

          case KeyState::RELEASED:
            deleteKeyHoldInfo((uint8_t)keypad.key[i].kchar);
            break;

          default:
            break;
          }
        }
      }
    }
  }
  // check for auto repeat
  if (autoRepeatInterval > 0)
  {
    unsigned long currentMillis = millis();
    for (uint8_t i = 0; i < LIST_MAX; i++)
    {
      if (keyHoldInfo[i].keyCode != 0)
      {
        // check if it is time to generate a key repeat event
        if (currentMillis - keyHoldInfo[i].holdTimestamp > autoRepeatInterval)
        {
          buffer[0] = keyHoldInfo[i].keyCode;
          buffer[1] = KEYSTATE_AUTOREPEAT;
          kSerial.write(buffer, 2);
          keyHoldInfo[i].holdTimestamp = currentMillis;
        }
      }
    }
  }
}

// event handler for I2C commands
void receiveEvent(int count)
{
  while (Wire.available())
  {
    char c = Wire.read();
    if (c == CMDINDENTIFIER)
    {
      int command = Wire.read();
      switch (command)
      {
      case CMD_RESET:
        onReset();
        break;

      case CMD_GETVERSION:
        onGetVersion();
        break;

      case CMD_SETHOLDTIME:
        onSetHoldTime();
        break;

      case CMD_SETDEBOUNCETIME:
        onSetDebounceTime();
        break;

      case CMD_SETAUTOREPEATINTERVAL:
        onSetAutoRepeatInterval();
        break;

      default:
        break;
      }
    }
  }
}

// request handler
void requestEvent()
{
  switch (pendingRequest)
  {
  case CMD_GETVERSION:
    Wire.write(VERSION_MAJOR);
    Wire.write(VERSION_MINOR);
    Wire.write(VERSION_REVISION);
    pendingRequest = -1;
    break;
  }
}

// reset the keyboard
void onReset()
{
  reset();
}

// prepare to answer version request
void onGetVersion()
{
  pendingRequest = CMD_GETVERSION;
}

// set new hold time value
void onSetHoldTime()
{
  keypad.setHoldTime(Wire.read());
}

// set new debounce time value
void onSetDebounceTime()
{
  keypad.setDebounceTime(Wire.read());
}

// sets the key repeat interval
// set to 0 to disable auto repeat
void onSetAutoRepeatInterval()
{
  autoRepeatInterval = Wire.read();
  initKeyHoldInfo();
}

// deletes the hold timestamp for this key
void deleteKeyHoldInfo(uint8_t keyCode)
{
  for (int i = 0; i < LIST_MAX; i++)
  {
    if (keyHoldInfo[i].keyCode == keyCode)
    {
      keyHoldInfo[i].keyCode = 0;
      keyHoldInfo[i].holdTimestamp = 0;
      break;
    }
  }
}

// sets the hold timestamp for this key
void setKeyHoldInfo(uint8_t keyCode)
{
  for (int i = 0; i < LIST_MAX; i++)
  {
    if (keyHoldInfo[i].keyCode == 0)
    {
      keyHoldInfo[i].keyCode = keyCode;
      keyHoldInfo[i].holdTimestamp = millis() + autoRepeatInterval;
      break;
    }
  }
}

// initializes the key hold timestamp table
void initKeyHoldInfo()
{
  for (int i = 0; i < LIST_MAX; i++)
  {
    keyHoldInfo[i].keyCode = 0;
    keyHoldInfo[i].holdTimestamp = 0;
  }
}