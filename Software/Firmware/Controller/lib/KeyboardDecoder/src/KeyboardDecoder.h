// KeyboardDecoder.h

// Copyright (C) 2023 highvoltglow
// Licensed under the MIT License

#pragma once

#include <Arduino.h>
#include <KeyboardHandler.h>
#include <NixieCalc.h>

enum class key_function_type
{
  unknown,
  numeric,
  numericx2,
  dp,
  operation,
  function
};

class KeyboardDecoder
{
public:
  KeyboardDecoder()
  {
  }

  virtual ~KeyboardDecoder()
  {
  }

  static void decode(uint8_t keyCode, bool functionKeyPressed, key_function_type *function, operation *op, uint8_t *digit)
  {

    *op = operation::none;
    *digit = -1;
    *function = key_function_type::unknown;

    if (!functionKeyPressed)
    {
      switch (keyCode)
      {
      case KEY_0:
        *function = key_function_type::numeric;
        *digit = 0;
        break;

      case KEY_1:
        *function = key_function_type::numeric;
        *digit = 1;
        break;

      case KEY_2:
        *function = key_function_type::numeric;
        *digit = 2;
        break;

      case KEY_3:
        *function = key_function_type::numeric;
        *digit = 3;
        break;

      case KEY_4:
        *function = key_function_type::numeric;
        *digit = 4;
        break;

      case KEY_5:
        *function = key_function_type::numeric;
        *digit = 5;
        break;

      case KEY_6:
        *function = key_function_type::numeric;
        *digit = 6;
        break;

      case KEY_7:
        *function = key_function_type::numeric;
        *digit = 7;
        break;

      case KEY_8:
        *function = key_function_type::numeric;
        *digit = 8;
        break;

      case KEY_9:
        *function = key_function_type::numeric;
        *digit = 9;
        break;

      case KEY_00:
        *function = key_function_type::numericx2;
        *digit = 0;
        break;

      case KEY_DOT:
        *function = key_function_type::dp;
        break;

      case KEY_AC:
        *function = key_function_type::operation;
        *op = operation::allclear;
        break;

      case KEY_C:
        *function = key_function_type::operation;
        *op = operation::clear;
        break;

      case KEY_PLUSMINUS:
        *function = key_function_type::operation;
        *op = operation::switchsign;
        break;

      case KEY_PLUS:
        *function = key_function_type::operation;
        *op = operation::addition;
        break;

      case KEY_MINUS:
        *function = key_function_type::operation;
        *op = operation::subtraction;
        break;

      case KEY_EQUALS:
        *function = key_function_type::operation;
        *op = operation::equals;
        break;

      case KEY_DIV:
        *function = key_function_type::operation;
        *op = operation::division;
        break;

      case KEY_MUL:
        *function = key_function_type::operation;
        *op = operation::multiplication;
        break;

      case KEY_PERCENT:
        *function = key_function_type::operation;
        *op = operation::percent;
        break;

      case KEY_SQUAREROOT:
        *function = key_function_type::operation;
        *op = operation::squareroot;
        break;

      case KEY_MC:
        *function = key_function_type::operation;
        *op = operation::memclear;
        break;

      case KEY_MR:
        *function = key_function_type::operation;
        *op = operation::memread;
        break;

      case KEY_MS:
        *function = key_function_type::operation;
        *op = operation::memstore;
        break;

      case KEY_MMINUS:
        *function = key_function_type::operation;
        *op = operation::memsubtraction;
        break;

      case KEY_MPLUS:
        *function = key_function_type::operation;
        *op = operation::memaddition;
        break;

      case KEY_INV:
        *function = key_function_type::operation;
        *op = operation::inv;
        break;

      case KEY_POW:
        *function = key_function_type::operation;
        *op = operation::pow;
        break;

      case KEY_SIN:
        *function = key_function_type::operation;
        *op = operation::sin;
        break;

      case KEY_COS:
        *function = key_function_type::operation;
        *op = operation::cos;
        break;

      case KEY_TAN:
        *function = key_function_type::operation;
        *op = operation::tan;
        break;

      case KEY_LOG:
        *function = key_function_type::operation;
        *op = operation::log;
        break;

      case KEY_LN:
        *function = key_function_type::operation;
        *op = operation::ln;
        break;
      }
    }
    else
    {
      switch (keyCode)
      {
      case KEY_INV:
        *function = key_function_type::operation;
        *op = operation::factorial;
        break;

      case KEY_LN:
        *function = key_function_type::operation;
        *op = operation::euler;
        break;

      case KEY_SIN:
        *function = key_function_type::operation;
        *op = operation::pi;
        break;
      }
    }
  }
};