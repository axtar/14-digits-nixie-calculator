// Clock.h

// provides clock mode functionality

// Copyright (C) 2021 highvoltglow
// Licensed under the MIT License

#pragma once

#include <Arduino.h>
#include <Timezone.h> // https://github.com/JChristensen/Timezone
#include <SoftwareSerial.h>
#include <KeyboardHandler.h>
#include <DS3232RTC.h> // https://github.com/JChristensen/DS3232RTC
#include <ubGPSTime.h>

#define GPS_MSG_INTERVAL 60            // 60 seconds
#define GPS_SYNC_INTERVAL_SHORT 15     // 15 seconds
#define GPS_SYNC_INTERVAL_LONG 60 * 60 // 1 hour
#define BLINK_ON_SYNC_INTERVAL 8       // 8 seconds

class Clock
{
public:
  Clock()
  {
    setDefaultTimeChangePoint();
    _timeZone = new Timezone(_dstRule, _stdRule);
  }

  virtual ~Clock()
  {
    delete (_timeZone);
  }

  void begin(int8_t rxPin, int8_t txPin)
  {
    // sync time with RTC
    setSyncProvider(_rtc.get);
    _gpsCom.begin(9600, SWSERIAL_8N1, rxPin, txPin);
  }

  bool process()
  {
    return (true);
  }

private:
  SoftwareSerial _gpsCom;
  DS3232RTC _rtc;
  ubGPSTime _gps;
  TimeChangeRule _dstRule;
  TimeChangeRule _stdRule;
  Timezone *_timeZone;

  void setDefaultTimeChangePoint()
  {
    // central european summer time
    strcpy(_dstRule.abbrev, "DST");
    _dstRule.week = week_t::Last;
    _dstRule.dow = dow_t::Sun;
    _dstRule.month = month_t::Mar;
    _dstRule.hour = 2;
    _dstRule.offset = 120;

    // central european standard time
    strcpy(_stdRule.abbrev, "STD");
    _stdRule.week = week_t::Last;
    _stdRule.dow = dow_t::Sun;
    _stdRule.month = month_t::Oct;
    _stdRule.hour = 3;
    _stdRule.offset = 60;
  }

  // current time
  void getCurrentTime(TimeElements *tm)
  {
    time_t utc, local;
    utc = now();
    // convert to local time
    local = _timeZone->toLocal(utc);
    // to time elements
    breakTime(local, *tm);
  }
};