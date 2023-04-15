// MenuHandler.h

//

// Copyright (C) 2023 highvoltglow
// Licensed under the MIT License

#pragma once

#include <Arduino.h>
#include <Settings.h>
#include <KeyboardHandler.h>
#include <KeyboardDecoder.h>
#include <DisplayHandler.h>
#include <map>

enum class rgb_part
{
  red,
  green,
  blue
};

enum class time_part
{
  hours,
  minutes
};

class MenuHandler
{
public:
  MenuHandler(Settings *settings) : _settings(settings),
                                    _settingsMap(_settings->getSettingsMap())
  {
    _display = "";
    _digitCount = 0;
  }

  virtual ~MenuHandler()
  {
  }

  void begin(uint8_t digitCount)
  {
    _it = _settingsMap.begin();
    _digitCount = digitCount;
    _it->second->setTempValue(_it->second->get());
    formatDisplay(_it->second);
    _rgbPart = rgb_part::red;
    _timePart = time_part::hours;
  }

  uint8_t getRed()
  {
    return (_red);
  }

  uint8_t getGreen()
  {
    return (_green);
  }

  uint8_t getBlue()
  {
    return (_blue);
  }

  String getDisplay()
  {
    return (_display);
  }

  void onKeyboardEvent(uint8_t keyCode, key_state keyState, bool functionKeyPressed)
  {
    if ((keyState == key_state::pressed) || (keyState == key_state::autorepeat))
    {
      switch (keyCode)
      {
      case KEY_MPLUS:
        setNextSetting();
        break;

      case KEY_MMINUS:
        setPrevSetting();
        break;

      case KEY_MINUS:
        setPrevValue();
        break;

      case KEY_PLUS:
        setNextValue();
        break;

      case KEY_EQUALS:
        commitValue();
        break;

      case KEY_C:
        revertValue();
        break;

      case KEY_AC:
        resetValue();
        break;
      }
    }
  }

private:
  String _display;
  Settings *_settings;
  const SETTINGSMAP &_settingsMap;
  SETTINGSMAP::const_iterator _it;
  uint8_t _digitCount;
  rgb_part _rgbPart;
  time_part _timePart;
  uint8_t _red;
  uint8_t _green;
  uint8_t _blue;

  void formatDisplay(Setting *setting)
  {
    char buffer[2 * _digitCount];
    uint8_t hours;
    uint8_t minutes;
    uint8_t red = 0;
    uint8_t green = 0;
    uint8_t blue = 0;

    switch (setting->getSettingType())
    {
    case setting_type::numeric:
      if (setting->getTempValue() < 0)
      {
        sprintf(buffer, "-%02d%*s%3d.", setting->getId(), _digitCount - 5, " ", abs(setting->getTempValue()));
      }
      else
      {
        sprintf(buffer, "%02d%*s%3d.", setting->getId(), _digitCount - 5, " ", setting->getTempValue());
      }
      if (setting->getTempValue() < 0)
      {
        _display = "-";
      }
      else
      {
        _display = "";
      }
      _display += buffer;
      break;

    case setting_type::time:
      intToTime(setting->getTempValue(), &hours, &minutes);
      switch (_timePart)
      {
      case time_part::hours:
        sprintf(buffer, "%02d%*s%2d. %2d", setting->getId(), _digitCount - 7, " ", hours, minutes);
        break;

      case time_part::minutes:
        sprintf(buffer, "%02d%*s%2d %2d.", setting->getId(), _digitCount - 7, " ", hours, minutes);
        break;
      }
      _display = buffer;
      break;

    case setting_type::rgb:
      intToRGB(setting->getTempValue(), &red, &green, &blue);
      switch (_rgbPart)
      {
      case rgb_part::red:
        sprintf(buffer, "%02d %3d. %3d %3d", setting->getId(), red, green, blue);
        break;

      case rgb_part::green:
        sprintf(buffer, "%02d %3d %3d. %3d", setting->getId(), red, green, blue);
        break;

      case rgb_part::blue:
        sprintf(buffer, "%02d %3d %3d %3d.", setting->getId(), red, green, blue);
        break;
      }
      _display = buffer;
      break;
    }
    _red = red;
    _green = green;
    _blue = blue;
  }

  void setNextSetting()
  {
    if (_it != _settingsMap.end())
    {
      _it++;
    }
    if (_it == _settingsMap.end())
    {
      _it--;
    }
    _it->second->setTempValue(_it->second->get());
    _rgbPart = rgb_part::red;
    _timePart = time_part::hours;
    formatDisplay(_it->second);
  }

  void setPrevSetting()
  {
    if (_it != _settingsMap.begin())
    {
      _it--;
    }
    _it->second->setTempValue(_it->second->get());
    _rgbPart = rgb_part::red;
    _timePart = time_part::hours;
    formatDisplay(_it->second);
  }

  void setPrevValue()
  {
    uint8_t hours;
    uint8_t minutes;
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    switch (_it->second->getSettingType())
    {
    case setting_type::numeric:
      if (_it->second->getTempValue() > _it->second->getMin())
      {
        _it->second->setTempValue(_it->second->getTempValue() - 1);
      }
      formatDisplay(_it->second);
      break;

    case setting_type::time:
      intToTime(_it->second->getTempValue(), &hours, &minutes);
      switch (_timePart)
      {
      case time_part::hours:
        if (hours > 0)
        {
          hours--;
        }
        break;

      case time_part::minutes:
        if (minutes > 0)
        {
          minutes--;
        }
        break;
      }
      _it->second->setTempValue(timeToInt(hours, minutes));
      formatDisplay(_it->second);
      break;

    case setting_type::rgb:
      intToRGB(_it->second->getTempValue(), &red, &green, &blue);
      switch (_rgbPart)
      {
      case rgb_part::red:
        if (red > 0)
        {
          red--;
        }
        break;

      case rgb_part::green:
        if (green > 0)
        {
          green--;
        }
        break;

      case rgb_part::blue:
        if (blue > 0)
        {
          blue--;
        }
        break;
      }
      _it->second->setTempValue(rgbToInt(red, green, blue));
      formatDisplay(_it->second);
      break;
    }
  }

  void setNextValue()
  {
    uint8_t hours;
    uint8_t minutes;
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    switch (_it->second->getSettingType())
    {
    case setting_type::numeric:
      if (_it->second->getTempValue() < _it->second->getMax())
      {
        _it->second->setTempValue(_it->second->getTempValue() + 1);
      }
      formatDisplay(_it->second);
      break;

    case setting_type::time:
      intToTime(_it->second->getTempValue(), &hours, &minutes);
      switch (_timePart)
      {
      case time_part::hours:
        if (hours < 23)
        {
          hours++;
        }
        break;

      case time_part::minutes:
        if (minutes < 59)
        {
          minutes++;
        }
        break;
      }
      _it->second->setTempValue(timeToInt(hours, minutes));
      formatDisplay(_it->second);
      break;

    case setting_type::rgb:
      intToRGB(_it->second->getTempValue(), &red, &green, &blue);
      switch (_rgbPart)
      {
      case rgb_part::red:
        if (red < 255)
        {
          red++;
        }
        break;

      case rgb_part::green:
        if (green < 255)
        {
          green++;
        }
        break;

      case rgb_part::blue:
        if (blue < 255)
        {
          blue++;
        }
        break;
      }
      _it->second->setTempValue(rgbToInt(red, green, blue));
      formatDisplay(_it->second);
      break;
    }
  }

  void commitValue()
  {
    switch (_it->second->getSettingType())
    {
    case setting_type::numeric:
      _it->second->set(_it->second->getTempValue());
      break;

    case setting_type::time:
      _it->second->set(_it->second->getTempValue());
      if (_timePart == time_part::hours)
      {
        _timePart = time_part::minutes;
      }
      else
      {
        _timePart = time_part::hours;
      }
      break;

    case setting_type::rgb:
      _it->second->set(_it->second->getTempValue());
      switch (_rgbPart)
      {
      case rgb_part::red:
        _rgbPart = rgb_part::green;
        break;

      case rgb_part::green:
        _rgbPart = rgb_part::blue;
        break;

      case rgb_part::blue:
        _rgbPart = rgb_part::red;
        break;
      }
    }
    formatDisplay(_it->second);
  }

  void revertValue()
  {
    _it->second->setTempValue(_it->second->get());
    formatDisplay(_it->second);
  }

  void resetValue()
  {
    _it->second->reset();
    _it->second->setTempValue(_it->second->get());
    formatDisplay(_it->second);
  }

  int rgbToInt(uint8_t red, uint8_t green, uint8_t blue)
  {
    return ((red * 256 * 256) + (green * 256) + blue);
  }

  void intToRGB(int value, uint8_t *red, uint8_t *green, uint8_t *blue)
  {
    *blue = value & 255;
    *green = (value >> 8) & 255;
    *red = (value >> 16) & 255;
  }

  int timeToInt(uint8_t hours, uint8_t minutes)
  {
    return ((hours * 60) + minutes);
  }

  void intToTime(int value, uint8_t *hours, uint8_t *minutes)
  {
    *minutes = value % 60;
    *hours = value / 60;
  }
};