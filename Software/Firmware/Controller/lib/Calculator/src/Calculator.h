// Calculator.h

// provides calculator mode functionality

// Copyright (C) 2021 highvoltglow
// Licensed under the MIT License

#pragma once

#include <Arduino.h>
#include <NixieCalc.h>
#include <KeyboardHandler.h>

class Calculator
{
public:
  Calculator(uint8_t digitCount) : _digitCount(digitCount)
  {
    _display = "0";
    _error.clear();
    for (int i = 0; i < _digitCount; i++)
    {
      _error += '.';
    }
    _calcEngine.setAngleMode(angle_mode::deg);
    _inputPending = false;
  }

  virtual ~Calculator()
  {
  }

  String getDisplay()
  {
    return (_display);
  }

  void onKeyboardEvent(uint8_t keyCode, key_state keyState, bool functionKeyPressed)
  {
    if (keyState == key_state::pressed)
    {
      switch (keyCode)
      {
      case KEY_0:
        numericInput(0);
        break;

      case KEY_1:
        numericInput(1);
        break;

      case KEY_2:
        numericInput(2);
        break;

      case KEY_3:
        numericInput(3);
        break;

      case KEY_4:
        numericInput(4);
        break;

      case KEY_5:
        numericInput(5);
        break;

      case KEY_6:
        numericInput(6);
        break;

      case KEY_7:
        numericInput(7);
        break;

      case KEY_8:
        numericInput(8);
        break;

      case KEY_9:
        numericInput(9);
        break;

      case KEY_00:
        numericInput(0);
        numericInput(0);
        break;

      case KEY_DOT:
        decimalPointInput();
        break;

      case KEY_AC:
        operationInput(operation::allclear);
        break;

      case KEY_C:
        operationInput(operation::clear);
        break;

      case KEY_PLUSMINUS:
        operationInput(operation::switchsign);
        break;

      case KEY_PLUS:
        operationInput(operation::addition);
        break;

      case KEY_MINUS:
        operationInput(operation::subtraction);
        break;

      case KEY_EQUALS:
        operationInput(operation::equals);
        break;

      case KEY_DIV:
        operationInput(operation::division);
        break;

      case KEY_MUL:
        operationInput(operation::multiplication);
        break;

      case KEY_PERCENT:
        operationInput(operation::percent);
        break;

      case KEY_SQUAREROOT:
        operationInput(operation::squareroot);
        break;

      case KEY_MC:
        operationInput(operation::memclear);
        break;

      case KEY_MR:
        operationInput(operation::memread);
        break;

      case KEY_MS:
        operationInput(operation::memstore);
        break;

      case KEY_MMINUS:
        operationInput(operation::memsubtraction);
        break;

      case KEY_MPLUS:
        operationInput(operation::memaddition);
        break;

      case KEY_INV:
        operationInput(operation::inv);
        break;

      case KEY_POW:
        operationInput(operation::pow);
        break;

      case KEY_SIN:
        operationInput(operation::sin);
        break;

      case KEY_COS:
        operationInput(operation::cos);
        break;

      case KEY_TAN:
        operationInput(operation::tan);
        break;

      case KEY_LOG:
        operationInput(operation::log);
        break;

      case KEY_LN:
        operationInput(operation::ln);
        break;
      }
    }
  }

  // special floating number format routine
  String doubleToString(double number)
  {
    String s;
    char buffer[100];

    // stupid thing, but avoids displaying negative zeros
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
  uint8_t _digitCount;
  bool _inputPending;

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
