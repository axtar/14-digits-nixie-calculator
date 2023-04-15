// DisplayDriver.h

// provides basic functions to drive
// the high voltage shift registers
// and functions to show numbers,
// decimal points, the negative sign
// and backlight

// Copyright (C) 2023 highvoltglow
// Licensed under the MIT License

#pragma once

#include <Arduino.h>
#include <HardwareInfo.h>
#include <displayHAL.h>
#include <displayHAL_IN16.h>
#include <DisplayHAL_IN17.h>
#include <DisplayHAL_IN12.h>
#include <DisplayHAL_B5870.h>
#include <Adafruit_Neopixel.h> // https://github.com/adafruit/Adafruit_NeoPixel

#define DIGIT_OFF 255

// shift transition
#define SHIFT_BEGIN HIGH
#define SHIFT_COMMIT LOW

// store transition
#define STORE_BEGIN LOW
#define STORE_COMMIT HIGH

enum class decimal_point_state
{
  off,
  on
};

enum class minus_sign_state
{
  off,
  on
};

enum class plus_sign_state
{
  off,
  on
};

enum class menu_sign_state
{
  off,
  on
};

class DisplayDriver
{
public:
  DisplayDriver(display_type displayType, uint8_t dataPin,
                uint8_t storePin, uint8_t shiftPin, uint8_t blankPin,
                uint8_t ledCtlPin) : _displayType(displayType),
                                     _dataPin(dataPin),
                                     _storePin(storePin),
                                     _shiftPin(shiftPin),
                                     _blankPin(blankPin),
                                     _ledCtlPin(ledCtlPin)

  {

    // status (on or off) of the negative sign
    _minusSign = minus_sign_state::off;

    // status (on or off) of the postivie sign
    _plusSign = plus_sign_state::off;

    // select HAL
    switch (_displayType)
    {
    case display_type::in16:
      _dispHAL = new DisplayHAL_IN16();
      break;

    case display_type::in17:
      _dispHAL = new DisplayHAL_IN17();
      break;

    case display_type::in12:
      _dispHAL = new DisplayHAL_IN12();
      break;

    case display_type::b5870:
      _dispHAL = new DisplayHAL_B5870;
      break;
    }

    _digitCount = _dispHAL->getDigitCount();
    _decimalPointCount = _dispHAL->getDecimalPointCount();
    _ledCount = _dispHAL->getLEDCount();
    _hasPlusSign = _dispHAL->hasPlusSign();

    // array of digits
    // digit 0 is the most left nixie
    _digits = new uint8_t[_digitCount];

    // array of decimal points
    // decimal points are on the right side of the digits
    _decimalPoints = new decimal_point_state[_decimalPointCount];

    switch (_dispHAL->getLedType())
    {
    case led_type::smd:
      _leds = new Adafruit_NeoPixel(_ledCount, _ledCtlPin, NEO_GRB + NEO_KHZ800);
      break;

    case led_type::tht:
      _leds = new Adafruit_NeoPixel(_ledCount, _ledCtlPin, NEO_RGB + NEO_KHZ800);
      break;
    }
  };

  virtual ~DisplayDriver()
  {
    delete[] _digits;
    delete[] _decimalPoints;
    delete _dispHAL;
    delete _leds;
  }

  void begin()
  {
    // init LEDs
    _leds->begin();
    clearLEDs();
  }

  void clearLEDs()
  {
    _leds->clear();
    _leds->show();
  }

  void setLED(uint16_t ledID, uint8_t red, uint8_t green, uint8_t blue)
  {
    _leds->setPixelColor(ledID, red, green, blue);
  }

  void setAllLED(uint8_t red, uint8_t green, uint8_t blue)
  {
    for (int i = 0; i < _ledCount; i++)
    {
      setLED(i, red, green, blue);
    }
  }

  void updateLEDs()
  {
    _leds->show();
  }

  uint8_t getDigitCount()
  {
    return (_digitCount);
  }

  uint8_t getDecimalPointCount()
  {
    return (_decimalPointCount);
  }

  bool hasPlusSign()
  {
    return (_dispHAL->hasPlusSign());
  }

  bool hasMenuSign()
  {
    return (_dispHAL->hasMenuSign());
  }

  uint8_t getLedCount()
  {
    return (_ledCount);
  }

  void setDigit(uint8_t digit, uint8_t value)
  {
    if ((digit < _digitCount) && (digit > (-1)))
    {
      _digits[digit] = value;
    }
  }

  uint8_t getDigit(uint8_t digit)
  {
    if ((digit < _digitCount) && (digit > (-1)))
    {
      return (_digits[digit]);
    }
    return (DIGIT_OFF);
  }

  void setDecimalPoint(uint8_t decimalPoint, decimal_point_state state)
  {
    if (decimalPoint < _decimalPointCount)
    {
      _decimalPoints[decimalPoint] = state;
    }
  }

  void setMinusSign(minus_sign_state state)
  {
    _minusSign = state;
  }

  void setPlusSign(plus_sign_state state)
  {
    _plusSign = state;
  }

  void setMenuSign(menu_sign_state state)
  {
    _menuSign = state;
  }

  void refresh()
  {
    commitToRegisters();
  }

  void clear()
  {
    clearDecimalPoints();
    clearDigits();
    clearMinusSign();
    clearPlusSign();
    clearMenuSign();
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

  void clearMinusSign()
  {
    setMinusSign(minus_sign_state::off);
  }

  void clearMenuSign()
  {
    setMenuSign(menu_sign_state::off);
  }

  void clearPlusSign()
  {
    setPlusSign(plus_sign_state::off);
  }

private:
  uint8_t _digitCount;
  uint8_t _decimalPointCount;
  uint8_t _ledCount;
  display_type _displayType;
  uint8_t *_digits;
  decimal_point_state *_decimalPoints;
  minus_sign_state _minusSign;
  plus_sign_state _plusSign;
  menu_sign_state _menuSign;
  bool _hasPlusSign;
  uint8_t _dataPin;
  uint8_t _storePin;
  uint8_t _shiftPin;
  uint8_t _blankPin;
  uint8_t _ledCtlPin;
  DisplayHAL *_dispHAL;
  Adafruit_NeoPixel *_leds;

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

  void commitMinusSign()
  {
    if (_minusSign == minus_sign_state::off)
    {
      commitBit(LOW);
    }
    else
    {
      commitBit(HIGH);
    }
  }

  void commitPlusSign()
  {
    if (_plusSign == plus_sign_state::off)
    {
      commitBit(LOW);
    }
    else
    {
      commitBit(HIGH);
    }
  }

  void commitMenuSign()
  {
    if (_menuSign == menu_sign_state::off)
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
    // delayMicroseconds(5);
    digitalWrite(_shiftPin, SHIFT_COMMIT);
  }

  void blankRegisters()
  {
    digitalWrite(_blankPin, LOW);
    digitalWrite(_blankPin, HIGH);
  }

  // commits digits, decimal points and negative/plus sign to shift registers
  void commitToRegisters()
  {
    register_type regType;
    uint8_t digit = 0;
    uint8_t number = 0;
    digitalWrite(_storePin, STORE_BEGIN);

    for (uint8_t i = REGISTER_COUNT; i > 0; i--)
    {
      regType = _dispHAL->getRegisterInfo(i, &digit, &number);
      switch (regType)
      {
      case register_type::unknown:
        break;

      case register_type::minus_sign:
        commitMinusSign();
        break;

      case register_type::plus_sign:
        commitPlusSign();
        break;

      case register_type::menu_sign:
        commitMenuSign();
        break;

      case register_type::decimal_point:
        commitDecimalPoint(digit);
        break;

      case register_type::number:
        commitDigitNumber(digit, number);
        break;

      case register_type::not_used:
      case register_type::not_connected:
        commitBit(LOW);
        break;
      }
    }
    digitalWrite(_storePin, STORE_COMMIT);
  }
};