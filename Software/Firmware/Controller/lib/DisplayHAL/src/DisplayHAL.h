// DisplayHAL.h

// provides an interface to the different
// hardware versions of the nixie display
// and driver boards.
// IN-12, IN-16, IN-17 and B-5870

// Copyright (C) 2023 highvoltglow
// Licensed under the MIT License

#pragma once

#include <Arduino.h>

#define REGISTER_COUNT 160

enum class register_type : uint8_t
{
  unknown,
  not_connected,
  not_used,
  decimal_point,
  number,
  minus_sign,
  plus_sign,
  menu_sign
};

enum class led_type : uint8_t
{
  smd,
  tht
};

typedef struct
{
  register_type rt;
  uint8_t digit;
  uint8_t number;
} TRANSLATION_TABLE_ENTRY;

class DisplayHAL
{
public:
  DisplayHAL()
  {
  }

  virtual ~DisplayHAL()
  {
  }

  // returns information about what is connected to a specific shift register output
  virtual register_type getRegisterInfo(uint8_t registerNumber, uint8_t *digit, uint8_t *number) = 0;
  virtual uint8_t getDigitCount() = 0;
  virtual uint8_t getDecimalPointCount() = 0;
  virtual bool hasPlusSign() = 0;
  virtual bool hasMenuSign() = 0;
  virtual uint8_t getLEDCount() = 0;
  // indicates if there is a LED for each nixie
  virtual bool hasLedPerDigit() = 0;
  virtual led_type getLedType() = 0;
};
