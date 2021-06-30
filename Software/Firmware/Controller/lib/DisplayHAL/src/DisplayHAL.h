// DisplayHAL.h

// provides an interface to the different
// hardware versions of the nixie display
// and driver boards.
// IN-16, IN-17, B-5870, ...

// Copyright (C) 2021 highvoltglow
// Licensed under the MIT License

#pragma once

#include <Arduino.h>

#define REGISTER_COUNT 160

enum class register_type : uint8_t
{
  unknown,
  not_connected,
  decimal_point,
  number,
  negative_sign
};

class displayHAL
{
public:
  displayHAL(){

  };

  virtual ~displayHAL(){

  };

  // returns information about what is connected to a specific shift register output
  virtual register_type getRegisterInfo(uint8_t registerNumber, uint8_t *digit, uint8_t *number) = 0;
};
