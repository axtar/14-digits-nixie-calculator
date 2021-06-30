// KeyboardHandler.h

// Copyright (C) 2021 highvoltglow
// Licensed under the MIT License

#pragma once

#include <Arduino.h>

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
#define KEY_PERCENT 16
#define KEY_SQUAREROOT 11
#define KEY_PLUSMINUS 21
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

enum class key_state : uint8_t
{
  idle,
  pressed,
  hold,
  released,
  autorepeat
};

class KeyboardHandler
{

protected:
  using notifyCallBack = void (*)(uint8_t key, key_state keyState, bool functionKeyPressed);

public:
  KeyboardHandler()
  {
    _serialPort = nullptr;
    _majorVersion = 0;
    _minorVersion = 0;
    _revision = 0;
    _functionKeyPressed = false;
  }

  virtual ~KeyboardHandler()
  {
  }

  void begin(Stream &serialPort)
  {
    _serialPort = &serialPort;
  }

  void attach(notifyCallBack callBack)
  {
    _notify = callBack;
  }

  void detach()
  {
    _notify = nullptr;
  }

  void process()
  {
    uint8_t buffer[2];
    if (_serialPort)
    {
      while (_serialPort->available())
      {
        _serialPort->readBytes(buffer, 2);
        if (_notify)
        {
          // check if function key is pressed
          if (buffer[0] == KEY_FUNCTION)
          {
            switch ((key_state)buffer[1])
            {
            case key_state::pressed:
            case key_state::hold:
            case key_state::autorepeat:
              _functionKeyPressed = true;
              break;

            default:
              _functionKeyPressed = false;
              break;
            }
          }
          _notify(buffer[0], (key_state)buffer[1], _functionKeyPressed);
        }
      }
    }
  }

  bool setHoldTime(unsigned int holdTime)
  {
    bool returnValue = false;
    Wire.beginTransmission(KEYBOARD_I2C_ADDRESS);
    Wire.write(KEYBOARD_CMDIDENTIFIER);
    Wire.write(KEYBOARD_CMD_SETHOLDTIME);
    Wire.write(holdTime);
    byte error = Wire.endTransmission();
    if (error == 0)
    {
      returnValue = true;
    }
    return returnValue;
  }

  bool setDebounceTime(unsigned int debounceTime)
  {
    bool returnValue = false;
    Wire.beginTransmission(KEYBOARD_I2C_ADDRESS);
    Wire.write(KEYBOARD_CMDIDENTIFIER);
    Wire.write(KEYBOARD_CMD_SETDEBOUNCETIME);
    Wire.write(debounceTime);
    byte error = Wire.endTransmission();
    if (error == 0)
    {
      returnValue = true;
    }
    return returnValue;
  }

  bool setAutoRepeatInterval(unsigned int interval)
  {
    bool returnValue = false;
    Wire.beginTransmission(KEYBOARD_I2C_ADDRESS);
    Wire.write(KEYBOARD_CMDIDENTIFIER);
    Wire.write(KEYBOARD_CMD_SETAUTOREPEATINTERVAL);
    Wire.write(interval);
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

private:
  Stream *_serialPort;
  unsigned int _autoRepeatInterval;
  notifyCallBack _notify;
  int _majorVersion;
  int _minorVersion;
  int _revision;
  bool _functionKeyPressed;
};
