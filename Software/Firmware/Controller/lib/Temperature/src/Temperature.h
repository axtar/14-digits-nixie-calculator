// Temperature.h

// provides temperature mode functionality

// Copyright (C) 2023 highvoltglow
// Licensed under the MIT License

#pragma once

#include <Arduino.h>
#include <DallasTemperature.h>
#include <Settings.h>

class Temperature
{
public:
  Temperature(uint8_t pinTemp, Settings *settings) : _oneWire(pinTemp),
                                                     _sensors(&_oneWire),
                                                     _settings(settings)
  {
    _currentTemperature = 0.0;
    _temperatureCheckInterval = 5000;   // 5 seconds
    _temperatureConversionDelay = 1000; // 1 second
    _temperatureCheckTimestamp = millis() + _temperatureCheckInterval;
    _requestPending = false;
  }

  virtual ~Temperature()
  {
  }

  void begin()
  {
    setSettings();
    _sensors.begin();
    _sensors.setResolution(12);
  }

  float getTemperature()
  {
    return (_currentTemperature);
  }

  void process()
  {
    checkTemperature();
  }

  void setSettings()
  {
    _settings->getSetting(setting_id::temperaturecf, (int *)&_temperatureCF);
  }

private:
  float _currentTemperature;
  OneWire _oneWire;
  Settings *_settings;
  DallasTemperature _sensors;
  unsigned long _temperatureCheckTimestamp;
  unsigned long _temperatureCheckInterval;
  unsigned long _temperatureConversionDelay;
  bool _requestPending;
  temperature_cf::temperature_cf _temperatureCF;

  void checkTemperature()
  {
    unsigned long currentMillis = millis();
    if (!_requestPending)
    {
      if (currentMillis - _temperatureCheckTimestamp > _temperatureCheckInterval)
      {
        _sensors.setWaitForConversion(false);
        _sensors.requestTemperatures();
        _sensors.setWaitForConversion(true);
        _temperatureCheckTimestamp = currentMillis;
        _requestPending = true;
      }
    }
    else
    {
      if (currentMillis - _temperatureCheckTimestamp > _temperatureConversionDelay)
      {
        if (_temperatureCF == temperature_cf::celsius)
        {
          _currentTemperature = _sensors.getTempCByIndex(0);
        }
        else
        {
          _currentTemperature = _sensors.getTempFByIndex(0);
        }
        _requestPending = false;
      }
    }
  }
};
