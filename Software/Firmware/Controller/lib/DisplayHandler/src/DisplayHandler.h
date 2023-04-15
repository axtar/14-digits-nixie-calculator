// DisplayHandler.h

// provides formatting und display functions

// Copyright (C) 2023 highvoltglow
// Licensed under the MIT License

#pragma once

#include <Arduino.h>
#include <DisplayDriver.h>

class DisplayHandler : public DisplayDriver
{
public:
  using DisplayDriver::DisplayDriver;

  void show()
  {
    refresh();
  }

  void show(char *buffer)
  {
    String s = buffer;
    show(s);
  }

  void show(const String &s, bool leftZeroPadding = false)
  {
    bool prevDot = false;

    clear();

    if (leftZeroPadding)
    {
      for (int i = 0; i < getDigitCount(); i++)
      {
        setDigit(i, 0);
      }
    }
    int digit = getDigitCount() - 1;
    for (int i = s.length() - 1; i >= 0; i--)
    {
      switch (s[i])
      {
      case '-':
        setMinusSign(minus_sign_state::on);
        prevDot = false;
        break;

      case '+':
        setPlusSign(plus_sign_state::on);
        prevDot = false;
        break;

      case '.':
        if (prevDot)
        {
          digit--;
        }
        setDecimalPoint(digit, decimal_point_state::on);
        prevDot = true;
        break;

      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
        setDigit(digit, s[i] - '0');
        digit--;
        prevDot = false;
        break;

      default:
        digit--;
        prevDot = false;
        break;
      }
    }
    refresh();
  }

  void displayTest2()
  {
    clear();
    // test all digits
    for (int i = 0; i < getDigitCount(); i++)
    {
      setDigit(i, 0);
    }
    refresh();
  }

  void displayTest(int interval = 500)
  {
    clear();
    // test all digits
    for (int i = 0; i < getDigitCount(); i++)
    {
      clearDigits();
      for (int j = 0; j < 10; j++)
      {
        setDigit(i, j);
        refresh();
        delay(interval);
      }
    }
    clear();
    // test decimal points
    for (int i = 0; i < getDecimalPointCount(); i++)
    {
      clearDecimalPoints();
      setDecimalPoint(i, decimal_point_state::on);
      refresh();
      delay(interval);
    }
    // test minus sign
    clear();
    setMinusSign(minus_sign_state::on);
    refresh();
    delay(interval);
    clear();
    for (int i = 0; i < 2; i++)
    {
      // all together
      for (int j = 0; j < 10; j++)
      {
        clearDigits();
        for (int k = 0; k < getDigitCount(); k++)
        {
          setDigit(k, j);
        }
        refresh();
        delay(interval);
      }
    }
    clear();
    for (int i = 0; i < getDecimalPointCount(); i++)
    {
      setDecimalPoint(i, decimal_point_state::on);
    }
    refresh();
    delay(interval);
  }

private:
  String _currentDisplay;
};
