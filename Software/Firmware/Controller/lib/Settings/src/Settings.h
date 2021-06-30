// Settings.h

// stores and retrieves non-volatile settings

// Copyright (C) 2021 highvoltglow
// Licensed under the MIT License

// Settings.h

#pragma once

#include <Arduino.h>
#include <Timezone.h>
#include <Preferences.h>

class Settings
{
public:
  Settings()
  {
    //nvs_flash_init();
    _preferences.begin("CalcSettings", false);
  };

  virtual ~Settings()
  {
    _preferences.end();
  }

private:
  Preferences _preferences;

  TimeChangeRule _dstRule; // daylight saving time
  TimeChangeRule _stdRule; // standard time
};
