// Calculator.h

// provides calculator mode functionality

// Copyright (C) 2023 highvoltglow
// Licensed under the MIT License

#pragma once

#include <Arduino.h>
#include <NixieCalc.h>
#include <KeyboardHandler.h>
#include <KeyboardDecoder.h>

class Calculator
{
public:
  Calculator(Settings *settings)
      : _settings(settings)
  {
    _display = "0";
    _error.clear();
    _calcEngine.setAngleMode(angle_mode::deg);
    _inputPending = false;
    _hasPlusSign = false;
  }

  virtual ~Calculator()
  {
  }

  void begin(uint8_t digitCount, uint8_t decimalPointCount, bool hasPlusSign)
  {
    setParameters();
    _digitCount = digitCount;
    _decimalPointCount = decimalPointCount;
    _hasPlusSign = hasPlusSign;
    for (int i = 0; i < _decimalPointCount; i++)
    {
      _error += '.';
    }
  }

  void setParameters()
  {
  }

  String getDisplay()
  {
    return (_display);
  }

  void onKeyboardEvent(uint8_t keyCode, key_state keyState, bool functionKeyPressed)
  {
    operation op;
    uint8_t digit;
    key_function_type function;

    if (keyState == key_state::pressed)
    {
      KeyboardDecoder::decode(keyCode, functionKeyPressed, &function, &op, &digit);

      switch (function)
      {
      case key_function_type::numeric:
        numericInput(digit);
        break;

      case key_function_type::numericx2:
        numericInput(digit);
        numericInput(digit);
        break;

      case key_function_type::dp:
        decimalPointInput();
        break;

      case key_function_type::operation:
        operationInput(op);
        break;

      default:
        break;
      }
    }
  }

  // special floating number format routine
  String doubleToString(double number)
  {
    String s;
    char buffer[100];

    // stupid thing, but avoids displaying negative zero
    if (number == 0)
    {
      number = 0;
    }
    // format with maximum number of decimals
    sprintf(buffer, "%-.*f", _digitCount, number);
    s = buffer;
    // get number of digits before the decimal point
    int index = s.indexOf('.');
    if (number < 0)
    {
      index--;
    }
    // get number of decimals to display
    int decimals = _digitCount - index;
    // format again with the correct mumber of decimals
    sprintf(buffer, "%-.*f", decimals, number);
    s = buffer;
    index = s.indexOf('.');

    if (index != -1)
    {
      // remove trailing zeroes after decimal point
      while (s[s.length() - 1] == '0')
      {
        s.remove(s.length() - 1);
      }
      // remove decimal point if no decimals left
      if (s[s.length() - 1] == '.')
      {
        s.remove(s.length() - 1);
      }
    }
    return (s);
  }

private:
  String _display;
  String _error;
  NixieCalc _calcEngine;
  Settings *_settings;
  uint8_t _digitCount;
  uint8_t _decimalPointCount;
  bool _inputPending;
  bool _hasPlusSign;

  void numericInput(uint8_t digit)
  {
    // accept input only if not in error state
    if (_calcEngine.getOperationReturnCode() == operation_return_code::success)
    {
      if (!_inputPending)
      {
        _display.clear();
        _inputPending = true;
      }
      if (_display.equals("0"))
      {
        _display = (char)(digit + 48);
      }
      else
      {
        if (getUsedDigits() < _digitCount)
        {
          _display += (char)(digit + 48);
        }
      }
    }
  }

  void decimalPointInput()
  {
    // accept input only if not in error state
    if (_calcEngine.getOperationReturnCode() == operation_return_code::success)
    {

      if (_inputPending)
      {
        if (_display.indexOf('.') == -1)
        {
          _display += '.';
        }
      }
      else
      {
        _display = "0.";
      }
      _inputPending = true;
    }
  }

  void operationInput(operation op)
  {
    // accept input only if not in error state
    if (_calcEngine.getOperationReturnCode() == operation_return_code::success)
    {
      switch (op)
      {
      case operation::memclear:
        _calcEngine.onOperation(op);
        break;

      case operation::memstore:
      case operation::memsubtraction:
      case operation::memaddition:
        _calcEngine.onNumericInput(_display.toDouble());
        _calcEngine.onOperation(op);
        break;

      default:
        if (_inputPending)
        {
          _calcEngine.onNumericInput(_display.toDouble());
        }
        _calcEngine.onOperation(op);
        if (_calcEngine.getOperationReturnCode() == operation_return_code::success)
        {
          _display = doubleToString(_calcEngine.getDisplayValue());
          if (_hasPlusSign)
          {
            if (op == operation::switchsign)
            {
              if (_calcEngine.getDisplayValue() > 0)
              {
                _display = "+" + _display;
              }
            }
          }
        }
        else
        {
          _display = _error;
        }
      }
    }
    else
    {
      // always accept all clear
      if (op == operation::allclear)
      {
        _calcEngine.onOperation(op);
        _display = doubleToString(_calcEngine.getDisplayValue());
      }
    }
    _inputPending = false;
  }

  uint8_t getUsedDigits()
  {
    uint8_t result = _display.length();
    if (_display.indexOf('-') != (-1))
    {
      result--;
    }
    if (_display.indexOf('.') != (-1))
    {
      result--;
    }
    return result;
  }
};
