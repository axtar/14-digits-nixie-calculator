// KeyboardHandler.h

// Copyright (C) 2023 highvoltglow
// Licensed under the MIT License

#pragma once

#include <Arduino.h>
#include <Wire.h>

// keyboard

// keys values
#define KEY_DOT 25
#define KEY_0 15
#define KEY_1 14
#define KEY_2 19
#define KEY_3 24
#define KEY_4 13
#define KEY_5 18
#define KEY_6 23
#define KEY_7 12
#define KEY_8 17
#define KEY_9 22
#define KEY_PLUS 29
#define KEY_MINUS 28
#define KEY_EQUALS 30
#define KEY_DIV 26
#define KEY_MUL 27
#define KEY_PERCENT 21
#define KEY_SQUAREROOT 16
#define KEY_PLUSMINUS 11
#define KEY_C 3
#define KEY_AC 4
#define KEY_FUNCTION 5
#define KEY_MC 31
#define KEY_MR 32
#define KEY_MS 33
#define KEY_MMINUS 35
#define KEY_MPLUS 34
#define KEY_00 20
#define KEY_SIN 6
#define KEY_COS 7
#define KEY_TAN 8
#define KEY_LOG 9
#define KEY_LN 10
#define KEY_INV 2
#define KEY_POW 1

//
#define KEYBOARD_I2C_ADDRESS 2

#define KEYBOARD_CMDIDENTIFIER '@'
#define KEYBOARD_CMD_RESET 1
#define KEYBOARD_CMD_GETVERSION 2
#define KEYBOARD_CMD_SETHOLDTIME 3
#define KEYBOARD_CMD_SETDEBOUNCETIME 4
#define KEYBOARD_CMD_SETAUTOREPEATINTERVAL 5
#define KEYBOARD_CMD_SETFASTAUTOREPEATINTERVAL 6
#define KEYBOARD_CMD_SETFASTAUTOREPEATDELAY 7

enum class key_state : uint8_t
{
  idle,
  pressed,
  hold,
  released,
  autorepeat
};

enum class keyboard_event_category : uint8_t
{
  numeric,
  decimal,
  operation,
  function
};

enum class special_keyboard_event : uint8_t
{
  none,
  menu_mode,
  mode_switch
};

class KeyboardHandler
{

protected:
  using notifyCallBack = void (*)(void *obj, uint8_t key, key_state keyState, bool functionKeyPressed, special_keyboard_event specialEvent);
  using notifyRawCallBack = void (*)(void *obj, uint8_t key, key_state keyState);

public:
  KeyboardHandler()
  {
    _serialPort = nullptr;
    _majorVersion = 0;
    _minorVersion = 0;
    _revision = 0;
    _functionKeyPressed = false;
    _functionKeyHold = false;
    _keyPressed = false;
    _notify = nullptr;
    _notifyRaw = nullptr;
    _lastKeyTimestamp = millis();
  }

  virtual ~KeyboardHandler()
  {
  }

  void begin(Stream &serialPort)
  {
    _serialPort = &serialPort;
  }

  void attach(void *obj, notifyCallBack callBack)
  {
    _obj = obj;
    _notify = callBack;
  }

  void attachRaw(notifyRawCallBack callBack)
  {
    _notifyRaw = callBack;
  }

  void detach()
  {
    _notify = nullptr;
    _obj = nullptr;
  }

  unsigned long getLastKeyTimestamp()
  {
    return (_lastKeyTimestamp);
  }

  void detachRaw()
  {
    _notifyRaw = nullptr;
    _obj = nullptr;
  }

  void process()
  {
    uint8_t buffer[2];
    if (_serialPort)
    {
      while (_serialPort->available())
      {
        _serialPort->readBytes(buffer, 2);
        if (_notifyRaw)
        {
          _notifyRaw(_obj, buffer[0], (key_state)buffer[1]);
        }
        if (_notify)
        {
          notifyKeyboardEvent(buffer[0], (key_state)buffer[1]);
        }
      }
    }
  }

  //
  void notifyKeyboardEvent(uint8_t key, key_state state)
  {
    special_keyboard_event specialEvent = special_keyboard_event::none;
    // check if function key is pressed
    if (key == KEY_FUNCTION)
    {
      switch (state)
      {
      case key_state::released:
        if (!_functionKeyHold)
        {
          if (!_keyPressed)
          {
            specialEvent = special_keyboard_event::mode_switch;
          }
        }
        _functionKeyPressed = false;
        _functionKeyHold = false;
        _keyPressed = false;
        _lastKeyTimestamp = millis();
        break;

      case key_state::pressed:
        _functionKeyPressed = true;
        break;

      case key_state::hold:
        _functionKeyHold = true;
        if (!_keyPressed)
        {
          specialEvent = special_keyboard_event::menu_mode;
        }
        break;

      case key_state::autorepeat:
        _functionKeyPressed = true;
        break;

      default:
        _functionKeyPressed = false;
        break;
      }
    }
    else
    {
      switch (state)
      {
      case key_state::pressed:
        if (_functionKeyPressed)
        {
          _keyPressed = true;
        }
        break;

      case key_state::released:
        _lastKeyTimestamp = millis();
        break;
      }
    }
    _notify(_obj, key, state, _functionKeyPressed, specialEvent);
  }

  bool setHoldTime(uint16_t holdTime)
  {
    bool returnValue = false;
    Wire.beginTransmission(KEYBOARD_I2C_ADDRESS);
    Wire.write(KEYBOARD_CMDIDENTIFIER);
    Wire.write(KEYBOARD_CMD_SETHOLDTIME);
    writeUInt(holdTime);
    byte error = Wire.endTransmission();
    if (error == 0)
    {
      returnValue = true;
    }
    return returnValue;
  }

  bool setDebounceTime(uint16_t debounceTime)
  {
    bool returnValue = false;
    Wire.beginTransmission(KEYBOARD_I2C_ADDRESS);
    Wire.write(KEYBOARD_CMDIDENTIFIER);
    Wire.write(KEYBOARD_CMD_SETDEBOUNCETIME);
    writeUInt(debounceTime);
    byte error = Wire.endTransmission();
    if (error == 0)
    {
      returnValue = true;
    }
    return returnValue;
  }

  bool setAutoRepeatInterval(uint16_t interval)
  {
    bool returnValue = false;
    Wire.beginTransmission(KEYBOARD_I2C_ADDRESS);
    Wire.write(KEYBOARD_CMDIDENTIFIER);
    Wire.write(KEYBOARD_CMD_SETAUTOREPEATINTERVAL);
    writeUInt(interval);
    byte error = Wire.endTransmission();
    if (error == 0)
    {
      returnValue = true;
    }
    return returnValue;
  }

  bool setFastAutoRepeatInterval(uint16_t interval)
  {
    bool returnValue = false;
    Wire.beginTransmission(KEYBOARD_I2C_ADDRESS);
    Wire.write(KEYBOARD_CMDIDENTIFIER);
    Wire.write(KEYBOARD_CMD_SETFASTAUTOREPEATINTERVAL);
    writeUInt(interval);
    byte error = Wire.endTransmission();
    if (error == 0)
    {
      returnValue = true;
    }
    return returnValue;
  }

  bool setFastAutoRepeatDelay(uint16_t interval)
  {
    bool returnValue = false;
    Wire.beginTransmission(KEYBOARD_I2C_ADDRESS);
    Wire.write(KEYBOARD_CMDIDENTIFIER);
    Wire.write(KEYBOARD_CMD_SETFASTAUTOREPEATDELAY);
    writeUInt(interval);
    byte error = Wire.endTransmission();
    if (error == 0)
    {
      returnValue = true;
    }
    return returnValue;
  }

  boolean resetKeyboard()
  {
    bool returnValue = false;
    Wire.beginTransmission(KEYBOARD_I2C_ADDRESS);
    Wire.write(KEYBOARD_CMDIDENTIFIER);
    Wire.write(KEYBOARD_CMD_RESET);
    byte error = Wire.endTransmission();
    if (error == 0)
    {
      returnValue = true;
    }
    return returnValue;
  }

  void requestVersion()
  {
    Wire.beginTransmission(KEYBOARD_I2C_ADDRESS);
    Wire.write(KEYBOARD_CMDIDENTIFIER);
    Wire.write(KEYBOARD_CMD_GETVERSION);
    Wire.endTransmission();
    Wire.requestFrom(KEYBOARD_I2C_ADDRESS, (int)3);
    if (Wire.available())
    {
      _majorVersion = Wire.read();
      _minorVersion = Wire.read();
      _revision = Wire.read();
    }
  }

  int getMajorVersion()
  {
    return (_majorVersion);
  }

  int getMinorVersion()
  {
    return (_minorVersion);
  }

  int getRevision()
  {
    return (_revision);
  }

  void writeUInt(uint16_t value)
  {
    byte hiByte = (value) >> 8;
    byte loByte = value & 0x00FF;
    Wire.write(hiByte);
    Wire.write(loByte);
  }

private:
  Stream *_serialPort;
  unsigned int _autoRepeatInterval;
  notifyCallBack _notify;
  notifyRawCallBack _notifyRaw;
  void *_obj;
  int _majorVersion;
  int _minorVersion;
  int _revision;
  bool _functionKeyPressed;
  bool _functionKeyHold;
  bool _keyPressed;
  unsigned long _lastKeyTimestamp;
};
