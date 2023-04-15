// Nixie Calculator Keyboard Firmware

// Copyright (C) 2023 highvoltglow
// Licensed under the MIT License

// the keyboard receives commands from the controller using I2C
// keyboard events are sent to the controller using a serial interface

// commands:
// CMD_RESET                      -> resets the keyboard
// CMD_GETVERSION                 -> prepares for version request
// CMD_SETHOLDTIME                -> sets the time (ms) after a pressed key changes to hold state
// CMD_SETDEBOUNCETIME            -> sets the debounce time (ms), modify only if you get more events as expected
// CMD_SETAUTOREPEATINTERVAL      -> sets the time (ms) between autorepeat events if a key is hold
// CMD_SETFASTAUTOREPEATINTERVAL  -> sets the time (ms) between autorepeat events after the fast autorepeat delay
// CMD_SETFASTAUTOREPEATDELAY     -> sets the number of autorepeat events before changing to the fast autorepeat interval

// default event sequence:
// key pressed  -> "pressed" event
// key released -> "released" event, "idle" event

// event sequence with default values and hold:
// ------------------------------------------------------------------------------------------------
// | holdTime = 1000, autoRepeatInterval = 0, fastAutoRepeatInterval = 0, fastAutoRepeatDelay = 0 |
// ------------------------------------------------------------------------------------------------
// key pressed  -> "pressed" event
// time +1000   -> "hold" event
// ...
// key released -> "released" event, "idle" event

// event sequence example with auto repeat:
// ----------------------------------------------------------------------------------------------------
// | holdTime = 1000, autoRepeatInterval = 500, fastAutoRepeatInterval = 250, fastAutoRepeatDelay = 3 |
// ----------------------------------------------------------------------------------------------------
// key pressed  -> "pressed" event
// time +1000   -> "hold" event, "autorepeat" event
// time +1500   -> "autorepeat" event
// time +2000   -> "autorepeat" event
// time +2250   -> "autorepeat" event (fast)
// time +2500   -> "autorepeat" event (fast)
// ...
// key released -> "released" event, "idle" event

// includes
#include <Arduino.h>
#include <SoftwareSerial.h>
#include <Keypad.h>
#include <Wire.h>
#include <FirmwareInfo.h>

// #define SERIALDEBUG

// I2C address
#define I2C_ADDRESS 2

// commands
#define CMDINDENTIFIER '@'
#define CMD_RESET 1
#define CMD_GETVERSION 2
#define CMD_SETHOLDTIME 3
#define CMD_SETDEBOUNCETIME 4
#define CMD_SETAUTOREPEATINTERVAL 5
#define CMD_SETFASTAUTOREPEATINTERVAL 6
#define CMD_SETFASTAUTOREPEATDELAY 7

// keys arranged in 7 x 5 matrix
#define ROWS ((const byte)7)
#define COLS ((const byte)5)

// pins
#define PIN_KINT 17
#define PIN_DUMMY_RX 2

// this is an additional KeyState used
// for generated repeat events
#define KEYSTATE_AUTOREPEAT 4

typedef struct
{
  uint8_t keyCode;
  unsigned long holdTimestamp;
  uint autoRepeatCount;
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

volatile uint16_t holdTime = 1000;
volatile uint16_t debounceTime = 10;
volatile uint16_t autoRepeatInterval = 0;
volatile uint16_t fastAutoRepeatInterval = 0;
volatile uint16_t fastAutoRepeatDelay = 0;
volatile int pendingRequest = -1;

// forward declarations
void (*reset)(void) = 0;
void receiveEvent(int count);
void requestEvent();
void onReset();
void onGetVersion();
void onSetHoldTime();
void onSetDebounceTime();
void onSetAutoRepeatInterval();
void onSetFastAutoRepeatInterval();
void onSetFastAutoRepeatDelay();

void deleteKeyHoldInfo(uint8_t keyCode);
void setKeyHoldInfo(uint8_t keyCode);
void initKeyHoldInfo();
uint16_t readUInt();

void setup()
{
#ifdef SERIALDEBUG
  Serial.begin(9600);
#endif

  // init serial connection
  kSerial.begin(9600);

  // init the hold timestamp table
  initKeyHoldInfo();

  // set default values
  keypad.setHoldTime(holdTime);
  keypad.setDebounceTime(debounceTime);

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

#ifdef SERIALDEBUG
        Serial.print("Millis: ");
        Serial.print(millis());
        Serial.print(" ");
        Serial.print("Key: ");
        Serial.print((int)keypad.key[i].kchar);
        Serial.print(" ");
        Serial.print("State: ");
        Serial.println(keypad.key[i].kstate);
#endif

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
        bool genEvent = false;
        if ((keyHoldInfo[i].autoRepeatCount < fastAutoRepeatDelay) || (fastAutoRepeatDelay == 0))
        {
          // check if it is time to generate a key repeat event
          if (currentMillis - keyHoldInfo[i].holdTimestamp > autoRepeatInterval)
          {
            genEvent = true;
          }
        }
        else
        {
          // check if it is time to generate a key fast repeat event
          if (fastAutoRepeatInterval > 0)
          {
            if (currentMillis - keyHoldInfo[i].holdTimestamp > fastAutoRepeatInterval)
            {
              genEvent = true;
            }
          }
        }
        if (genEvent)
        {

#ifdef SERIALDEBUG
          Serial.print("Millis: ");
          Serial.print(millis());
          Serial.print(" ");
          Serial.print("Key: ");
          Serial.print(keyHoldInfo[i].keyCode);
          Serial.print(" ");
          Serial.print("State: ");
          Serial.println(KEYSTATE_AUTOREPEAT);
#endif
          buffer[0] = keyHoldInfo[i].keyCode;
          buffer[1] = KEYSTATE_AUTOREPEAT;
          kSerial.write(buffer, 2);
          keyHoldInfo[i].holdTimestamp = currentMillis;
          if (fastAutoRepeatInterval > 0)
          {
            keyHoldInfo[i].autoRepeatCount++;
          }
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

      case CMD_SETFASTAUTOREPEATINTERVAL:
        onSetFastAutoRepeatInterval();
        break;

      case CMD_SETFASTAUTOREPEATDELAY:
        onSetFastAutoRepeatDelay();
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
    Wire.write(MAJOR_VERSION);
    Wire.write(MINOR_VERSION);
    Wire.write(REVISION);
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
  holdTime = readUInt();
  keypad.setHoldTime(holdTime);
}

// set new debounce time value
void onSetDebounceTime()
{
  debounceTime = readUInt();
  keypad.setDebounceTime(debounceTime);
}

// sets the key repeat interval
// set to 0 to disable auto repeat
void onSetAutoRepeatInterval()
{
  autoRepeatInterval = readUInt();
  initKeyHoldInfo();
#ifdef SERIALDEBUG
  Serial.println(autoRepeatInterval);
#endif
}

// sets the key fast repeat interval
// set to 0 to disable fast auto repeat
void onSetFastAutoRepeatInterval()
{
  fastAutoRepeatInterval = readUInt();
}

// sets the delay before changing to fast repeat interval
void onSetFastAutoRepeatDelay()
{
  fastAutoRepeatDelay = readUInt();
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
      keyHoldInfo[i].autoRepeatCount = 0;
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
      keyHoldInfo[i].autoRepeatCount = 0;
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
    keyHoldInfo[i].autoRepeatCount = 0;
  }
}

// reads a unsigned int from the wire
uint16_t readUInt()
{
  uint16_t result = 0;
  byte loByte;
  byte hiByte;

  hiByte = Wire.read();
  loByte = Wire.read();

  result = hiByte << 8 | loByte;
  return (result);
}