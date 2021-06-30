// DisplayDriver.h

// provides basic functions to drive
// the high voltage shift registers
// and functions to show numbers,
// decimal points and the negative sign

// Copyright (C) 2021 highvoltglow
// Licensed under the MIT License

#pragma once

#include <Arduino.h>
#include <displayHAL.h>
#include <displayHAL_IN16.h>
#include <DisplayHAL_IN17.h>

#define DIGIT_OFF 255

// shift transition
#define SHIFT_BEGIN HIGH
#define SHIFT_COMMIT LOW

// store transition
#define STORE_BEGIN LOW
#define STORE_COMMIT HIGH

enum class display_mode : uint8_t
{
  in16,
  in17,
  b5870,
  in16legacy,
  in17legacy
};

enum class decimal_point_state : uint8_t
{
  off,
  on
};

enum class negative_sign_state : uint8_t
{
  off,
  on
};

class DisplayDriver
{
public:
  DisplayDriver(uint8_t digitCount, uint8_t decimalPointCount,
                display_mode displayMode, uint8_t dataPin,
                uint8_t storePin, uint8_t shiftPin) : _digitCount(digitCount),
                                                      _decimalPointCount(decimalPointCount),
                                                      _displayMode(displayMode),
                                                      _dataPin(dataPin),
                                                      _storePin(storePin),
                                                      _shiftPin(shiftPin)

  {
    // array of digits
    // digit 0 is the most left nixie
    _digits = new uint8_t[_digitCount];

    // array of decimal points
    //decimal points are on the right side of the digits
    _decimalPoints = new decimal_point_state[_decimalPointCount];

    // status (on or off) of the negative sign
    _negativeSign = negative_sign_state::off;

    pinMode(dataPin, OUTPUT);
    pinMode(storePin, OUTPUT);
    pinMode(shiftPin, OUTPUT);

    // select HAL
    switch (_displayMode)
    {
    case display_mode::in16:
      dispHAL = new displayHAL_IN16();
      break;

    case display_mode::in17:
      dispHAL = new displayHAL_IN17();
      break;

    default:
      dispHAL = nullptr;
      break;
    }
  };

  virtual ~DisplayDriver()
  {
    delete[] _digits;
    delete[] _decimalPoints;
    if (dispHAL)
    {
      delete dispHAL;
    }
  }

  uint8_t getDigitCount()
  {
    return (_digitCount);
  }

  uint8_t getDecimalPointCount()
  {
    return (_decimalPointCount);
  }

  void setDigit(uint8_t digit, uint8_t value)
  {
    if (digit < _digitCount)
    {
      _digits[digit] = value;
    }
  }

  void setDecimalPoint(uint8_t decimalPoint, decimal_point_state state)
  {
    if (decimalPoint < _decimalPointCount)
    {
      _decimalPoints[decimalPoint] = state;
    }
  }

  void setNegativeSign(negative_sign_state state)
  {
    _negativeSign = state;
  }

  void refresh()
  {
    commitToRegisters();
  }

  void clear()
  {
    clearDecimalPoints();
    clearDigits();
    clearNegativeSign();
  }

  void clearDisplay()
  {
    clear();
    refresh();
  }

  void clearDecimalPoints()
  {
    for (uint8_t i = 0; i < _decimalPointCount; i++)
    {
      _decimalPoints[i] = decimal_point_state::off;
    }
  }

  void clearDigits()
  {
    for (uint8_t i = 0; i < _digitCount; i++)
    {
      _digits[i] = DIGIT_OFF;
    }
  }

  void clearNegativeSign()
  {
    setNegativeSign(negative_sign_state::off);
  }

private:
  uint8_t _digitCount;
  uint8_t _decimalPointCount;
  display_mode _displayMode;
  uint8_t *_digits;
  decimal_point_state *_decimalPoints;
  negative_sign_state _negativeSign;
  uint8_t _dataPin;
  uint8_t _storePin;
  uint8_t _shiftPin;
  displayHAL *dispHAL;

  void commitDigitNumber(uint8_t digit, uint8_t number)
  {
    if (_digits[digit] == number)
    {
      commitBit(HIGH);
    }
    else
    {
      commitBit(LOW);
    }
  }

  //
  void commitDecimalPoint(int8_t decimalPoint)
  {
    if (_decimalPoints[decimalPoint] == decimal_point_state::off)
    {
      commitBit(LOW);
    }
    else
    {
      commitBit(HIGH);
    }
  }

  void commitNegativeSign()
  {
    if (_negativeSign == negative_sign_state::off)
    {
      commitBit(LOW);
    }
    else
    {
      commitBit(HIGH);
    }
  }

  void commitBit(uint8_t value)
  {
    digitalWrite(_shiftPin, SHIFT_BEGIN);
    digitalWrite(_dataPin, value);
    delayMicroseconds(1);
    digitalWrite(_shiftPin, SHIFT_COMMIT);
  }

  // commits digits, decimal points and negative sign to shift registers
  void commitToRegisters()
  {
    register_type regType;
    uint8_t digit = 0;
    uint8_t number = 0;
    digitalWrite(_storePin, STORE_BEGIN);

    for (uint8_t i = REGISTER_COUNT; i > 0; i--)
    {
      regType = dispHAL->getRegisterInfo(i, &digit, &number);
      switch (regType)
      {
      case register_type::unknown:
        break;

      case register_type::negative_sign:
        commitNegativeSign();
        break;

      case register_type::decimal_point:
        commitDecimalPoint(digit);
        break;

      case register_type::number:
        commitDigitNumber(digit, number);
        break;

      case register_type::not_connected:
        commitBit(LOW);
        break;
      }
    }
    digitalWrite(_storePin, STORE_COMMIT);
  }
};