// PIR.h

// provides PIR functionality

// Copyright (C) 2023 highvoltglow
// Licensed under the MIT License

#pragma once

#include <Arduino.h>
#include <Settings.h>

class PIR
{

public:
  PIR(Settings *settings)
      : _settings(settings)
  {
    _instance = this;
    _pirTimestamp = 0;
  }

  virtual ~PIR()
  {
  }

  void begin(uint8_t pinPIR)
  {
    _pinPIR = pinPIR;
    setParameters();
    attachInterrupt(_pinPIR, setPIRTimeout, HIGH);
  }

  void setParameters()
  {
    int value = 0;
    _settings->getSetting(setting_id::pirdelay, &value);
    _pirDelay = value * 1000 * 60; // convert to milliseconds
  }

  void handlePIRTimeout()
  {
    _pirTimestamp = millis();
  }

  bool process()
  {
    bool result = true;
    if (millis() - _pirTimestamp > (_pirDelay))
    {
      result = false;
    }
    return (result);
  }

private:
  uint8_t _pinPIR;
  unsigned long _pirTimestamp;
  unsigned long _pirDelay;
  Settings *_settings;
  static PIR *_instance;

  static void setPIRTimeout()
  {
    _instance->handlePIRTimeout();
  };
};

PIR *PIR::_instance;
