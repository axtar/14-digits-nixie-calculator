// displayHAL_IN16.h

// implements the hardware abstraction layer interface
// for the IN-16 board (socket version)

// Copyright (C) 2021 highvoltglow
// Licensed under the MIT License

#pragma once

#include <Arduino.h>
#include <displayHAL.h>

class displayHAL_IN17 : public displayHAL
{
public:
  displayHAL_IN17()
  {
  }

  virtual ~displayHAL_IN17()
  {
  }

  // returns information about what is connected to a specific shift register output
  register_type getRegisterInfo(uint8_t registerNumber, uint8_t *digit, uint8_t *number)
  {
    *digit = 0;
    *number = 0;
    register_type regType = register_type::unknown;

    if ((registerNumber < 1) || (registerNumber > REGISTER_COUNT))
    {
      regType = register_type::unknown;
    }
    else if (registerNumber < 155)
    {
      *digit = registerNumber / 11;
      uint8_t value = registerNumber % 11;
      if (value == 0)
      {
        *digit = *digit - 1;
        regType = register_type::decimal_point;
      }
      else
      {
        *number = value - 1;
        regType = register_type::number;
      }
    }
    else if (registerNumber == 155)
    {
      regType = register_type::negative_sign;
    }
    else
    {
      regType = register_type::not_connected;
    }
    return (regType);
  }
};
