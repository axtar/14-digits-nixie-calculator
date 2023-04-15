// Setting.h

// holds seting value

// Copyright (C) 2023 highvoltglow
// Licensed under the MIT License

#pragma once
#include <SettingEnum.h>

enum class setting_type
{
  time,
  rgb,
  numeric
};

class Setting
{
public:
  Setting(setting_id::setting_id id, setting_type settingType, int defaultValue, int minValue, int maxValue)
      : _id(id),
        _settingType(settingType),
        _defaultValue(defaultValue),
        _minValue(minValue),
        _maxValue(maxValue)

  {

    _value = _defaultValue;
    _modified = true;
  }

  virtual ~Setting()
  {
  }

  uint getId()
  {
    return (_id);
  }

  void setTempValue(int value)
  {
    if ((value <= _maxValue) && (value >= _minValue))
    {
      _tempValue = value;
    }
  }

  void set(int value)
  {
    if (value != _value)
    {
      if ((value <= _maxValue) && (value >= _minValue))
      {
        _value = value;
        _modified = true;
      }
    }
  }

  int getTempValue()
  {
    return (_tempValue);
  }

  int get()
  {
    return (_value);
  }

  int getDefault()
  {
    return (_defaultValue);
  }

  int getMin()
  {
    return (_minValue);
  }

  int getMax()
  {
    return (_maxValue);
  }

  void reset()
  {
    _value = _defaultValue;
    _modified = true;
  }

  void resetModified()
  {
    _modified = false;
  }

  bool modified()
  {
    return (_modified);
  }

  setting_type getSettingType()
  {
    return (_settingType);
  }

private:
  setting_id::setting_id _id;
  int _defaultValue;
  int _minValue;
  int _maxValue;
  int _value;
  int _tempValue;
  bool _modified;

  setting_type _settingType;
};