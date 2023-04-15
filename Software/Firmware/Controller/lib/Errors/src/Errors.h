// Errors.h

// Definition of error codes

// Copyright (C) 2023 highvoltglow
// Licensed under the MIT License

#pragma once

#include <Arduino.h>

#define ERR_SUCCESS 0
#define ERR_INITSETTINGS 1

class Errors
{
public:
  Errors(){

  };

  static String getErrorText(int errorCode)
  {
    String text;

    switch (errorCode)
    {
    case ERR_SUCCESS:
      text = "Success";
      break;

    case ERR_INITSETTINGS:
      text = "Failed to initialize settings";
      break;
    }
    return (text);
  }
};
