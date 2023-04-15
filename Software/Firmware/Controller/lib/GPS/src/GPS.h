// PIR.h

// provides GPS functionality

// Copyright (C) 2023 highvoltglow
// Licensed under the MIT License

#pragma once

#include <Arduino.h>
#include <Timezone.h>
#include <SoftwareSerial.h>
#include <Settings.h>
#include <ubGPSTime.h>

#define GPS_SYNC_INTERVAL_SHORT 15 * 1000 // the initial interval is 15 seconds
#define GPS_MSG_INTERVAL 60               // one msg every 60 seconds

class GPS
{

protected:
  using notifyCallBack = void (*)(void *obj, time_t utc);

public:
  GPS(Settings *settings)
      : _settings(settings)
  {
    _obj = nullptr;
    _notify = nullptr;
    _gpsSyncTimestamp = 0;
    _gpsSyncIntervalActive = _gpsSyncIntervalActive;
    _gpsInitialized = false;
    _gpsMessageInterval = GPS_MSG_INTERVAL;
  }

  virtual ~GPS()
  {
  }

  void begin(uint8_t pinRX, uint8_t pinTX)
  {
    _pinRX = pinRX;
    _pinTX = pinTX;
    setParameters();
    _gpsCom.begin(_gpsCommSpeed, SWSERIAL_8N1, _pinRX, _pinTX);
    _uGPS.begin(_gpsCom);
    _uGPS.initialize(false);
  }

  bool initialize()
  {
    bool retVal = false;
    if (_uGPS.isInitialized())
    {

      _uGPS.disableDefaultNMEA();
      // subscriptions
      _uGPS.subscribeGPSStatus(0); // we don't need status messages
      _uGPS.subscribeTimeUTC(_gpsMessageInterval);
      _uGPS.attach(this, onGPSMessageCallback);
      // Serial.println();
      // Serial.println("GPS module successfully initialized");
      // Serial.print("Software version: ");
      // Serial.println(_uGPS.getModuleVersion().swVersion);
      // Serial.print("Hardware version: ");
      // Serial.println(_uGPS.getModuleVersion().hwVersion);
      // for (uint8_t i = 0; i < MAX_EXTENSIONS; i++)
      // {
      //   Serial.printf("Extension %u: ", i + 1);
      //   Serial.println(_uGPS.getModuleVersion().extensions[i]);
      // }
      _gpsInitialized = true;
      retVal = true;
    }
    return (retVal);
  }

  void end()
  {
    _gpsCom.end();
    _gpsInitialized = false;
    _uGPS.detach();
  }

  void attach(void *obj, notifyCallBack callBack)
  {
    _obj = obj;
    _notify = callBack;
  }

  void detach()
  {
    _obj = nullptr;
    _notify = nullptr;
  }

  void setParameters()
  {
    int value = 0;
    _settings->getSetting(setting_id::gpsspeed, (int *)&_gpsSpeed);
    _gpsCommSpeed = getSpeed(_gpsSpeed);
    _settings->getSetting(setting_id::gpssyncinterval, &value);
    _gpsSyncInterval = value * 60 * 1000; // convert to milliseconds
  }

  void process()
  {
    if (!_gpsInitialized)
    {
      initialize();
    }
    _uGPS.process();
  }

  static void onGPSMessageCallback(void *obj, UBXMESSAGE *message)
  {
    ((GPS *)obj)->onGPSMessage(message);
  }

private:
  Settings *_settings;
  gps_speed::gps_speed _gpsSpeed;
  int _gpsCommSpeed;
  unsigned long _gpsMessageInterval;
  unsigned long _gpsSyncInterval;
  unsigned long _gpsSyncIntervalActive;
  unsigned long _gpsSyncTimestamp;
  uint8_t _pinRX;
  uint8_t _pinTX;
  SoftwareSerial _gpsCom;
  ubGPSTime _uGPS;
  void *_obj;
  notifyCallBack _notify;
  bool _gpsInitialized;

  int getSpeed(gps_speed::gps_speed speed)
  {
    int value;

    switch (speed)
    {
    case gps_speed::br_2400:
      value = 2400;
      break;

    case gps_speed::br_4800:
      value = 4800;
      break;

    case gps_speed::br_9600:
      value = 9600;
      break;

    case gps_speed::br_19200:
      value = 19200;
      break;

    case gps_speed::br_38400:
      value = 38400;
      break;

    case gps_speed::br_57600:
      value = 57600;
      break;

    case gps_speed::br_115200:
      value = 115200;
      break;

    default:
      value = 9600;
      break;
    }
    return value;
  }

  void onGPSMessage(UBXMESSAGE *message)
  {
    if (message->msgClass == UBX_NAV)
    {
      switch (message->msgID)
      {
      case UBX_NAV_TIMEUTC:
        gpsTimeSync(_uGPS.getTimeUTC());
        break;
      }
    }
  }

  void gpsTimeSync(TIMEUTC timeUTC)
  {
    TimeElements tm;
    if (millis() - _gpsSyncTimestamp > (_gpsSyncIntervalActive))
    {
      time_t utc = now();
      breakTime(utc, tm);
      if (timeUTC.timeOfWeekValid)
      {

        tm.Second = timeUTC.second;
        tm.Minute = timeUTC.minute;
        tm.Hour = timeUTC.hour;
      }
      if (timeUTC.weekNumberValid)
      {
        tm.Day = timeUTC.day;
        tm.Month = timeUTC.month;
        tm.Year = timeUTC.year - 1970;
      }
      if (timeUTC.timeOfWeekValid || timeUTC.weekNumberValid)
      {
        utc = makeTime(tm);
        setTime(utc);
        if (_notify)
        {
          _notify(_obj, utc);
        }
      }
      if (timeUTC.timeOfWeekValid && timeUTC.weekNumberValid)
      {
        _gpsSyncIntervalActive = _gpsSyncInterval;
      }
      else
      {
        _gpsSyncIntervalActive = GPS_SYNC_INTERVAL_SHORT;
      }
      _gpsSyncTimestamp = millis();
    }
  }
};
