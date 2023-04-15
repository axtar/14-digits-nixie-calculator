// Clock.h

// provides clock mode functionality

// Copyright (C) 2023 highvoltglow
// Licensed under the MIT License

#pragma once

#include <Arduino.h>
#include <SoftwareSerial.h>
#include <Timezone.h>  // https://github.com/JChristensen/Timezone
#include <DS3232RTC.h> // https://github.com/JChristensen/DS3232RTC
#include <DisplayHandler.h>
#include <Settings.h>
#include <KeyboardHandler.h>
#include <KeyboardDecoder.h>

#define MAX_TIMER_INPUT 8
#define MAX_TIMER_INTERVAL (99 * 86400) + (23 * 3600) + (59 * 60) + 59

enum class stopwatch_mode : uint8_t
{
  zero,
  stopped,
  running
};

enum class timer_mode : uint8_t
{
  zero,
  set,
  stopped,
  running
};

enum class year_type : uint8_t
{
  none,
  partial,
  full
};

enum class input_mode : uint8_t
{
  none,
  time,
  timer
};

class Clock
{
public:
  Clock(Settings *settings, DisplayHandler *displayHandler)
      : _settings(settings),
        _displayHandler(displayHandler)
  {
    _timeZone = new Timezone(_dstRule, _stdRule);
    _stopwatchMode = stopwatch_mode::zero;
    _timerMode = timer_mode::zero;
    _inputMode = input_mode::none;
    _setMillis = 0;
    _timerMode = timer_mode::zero;
  }

  virtual ~Clock()
  {
    delete (_timeZone);
  }

  void begin()
  {
    setSettings();
    _rtc.begin();
    // sync time with RTC
    setSyncProvider(_rtc.get);
  }

  void setSettings()
  {
    _settings->getSetting(setting_id::clockmode, (int *)&_clockMode);
    _settings->getSetting(setting_id::hourmode, (int *)&_hourMode);
    _settings->getSetting(setting_id::leadingzero, (int *)&_leadingZero);
    _settings->getSetting(setting_id::dateformat, (int *)&_dateFormat);
    setTimeZone();
  }

  bool process()
  {
    static TimeElements tm;

    getCurrentTime(&tm);
    switch (_inputMode)
    {
    case input_mode::none:
      displayTime(tm);
      break;

    default:
      showInput(tm);
      break;
    }
    return (true);
  }

  void setTemperature(float temperature)
  {
    _temperature = temperature;
  }
  void setRTCTime(time_t utc)
  {
    _rtc.set(utc);
  }

  float getBoardTemperature()
  {
    return ((float)(_rtc.temperature() / 4));
  }

  void onKeyboardEvent(uint8_t keyCode, key_state keyState, bool functionKeyPressed)
  {

    operation op;
    uint8_t digit;
    key_function_type function;

    if (keyState == key_state::pressed)
    {
      KeyboardDecoder::decode(keyCode, functionKeyPressed, &function, &op, &digit);

      switch (function)
      {
      case key_function_type::numeric:
        numericInput(digit);
        break;

      case key_function_type::numericx2:
        numericInput(digit);
        numericInput(digit);
        break;

      case key_function_type::operation:
        operationInput(op);
        break;

      default:
        break;
      }
    }
  }

private:
  DS3232RTC _rtc;
  TimeChangeRule _dstRule{"DST", Last, Sun, Mar, 2, 120};
  TimeChangeRule _stdRule{"STD ", Last, Sun, Oct, 3, 60};
  Timezone *_timeZone;
  Settings *_settings;
  DisplayHandler *_displayHandler;
  String _display;
  clock_mode::clock_mode _clockMode;
  hour_mode::hour_mode _hourMode;
  leading_zero::leading_zero _leadingZero;
  date_format::date_format _dateFormat;
  stopwatch_mode _stopwatchMode;
  timer_mode _timerMode;
  input_mode _inputMode;
  float _temperature;
  // timer variables
  uint64_t _setMillis;
  uint64_t _startMillis;
  uint64_t _elapsedMillis;

  // current time
  void getCurrentTime(TimeElements *tm)
  {
    time_t utc, local;
    utc = now();
    // convert to local time
    local = _timeZone->toLocal(utc);
    // to time elements
    breakTime(local, *tm);
    _rtc.temperature();
  }

  // set time zone rules from settings
  void setTimeZone()
  {
    int value;
    strcpy(_dstRule.abbrev, "DST");
    _settings->getSetting(setting_id::dstdow, &value);
    _dstRule.dow = value;
    _settings->getSetting(setting_id::dsthour, &value);
    _dstRule.hour = value;
    _settings->getSetting(setting_id::dstmonth, &value);
    _dstRule.month = value;
    _settings->getSetting(setting_id::dstoffset, &value);
    _dstRule.offset = value;
    _settings->getSetting(setting_id::dstweek, &value);
    _dstRule.week = value;
    strcpy(_stdRule.abbrev, "STD");
    _settings->getSetting(setting_id::stddow, &value);
    _stdRule.dow = value;
    _settings->getSetting(setting_id::stdhour, &value);
    _stdRule.hour = value;
    _settings->getSetting(setting_id::stdmonth, &value);
    _stdRule.month = value;
    _settings->getSetting(setting_id::stdoffset, &value);
    _stdRule.offset = value;
    _settings->getSetting(setting_id::stdweek, &value);
    _stdRule.week = value;

    _timeZone->setRules(_dstRule, _stdRule);
  }

  void setRTCTime()
  {
    time_t local, utc;
    tmElements_t tm;

    if (_display.length() == _displayHandler->getDigitCount())
    {
      int temp = 0;
      temp = _display.substring(0, 4).toInt();
      tm.Year = CalendarYrToTm(temp);
      temp = _display.substring(4, 6).toInt();
      tm.Month = temp;
      temp = _display.substring(6, 8).toInt();
      tm.Day = temp;
      temp = _display.substring(8, 10).toInt();
      tm.Hour = temp;
      temp = _display.substring(10, 12).toInt();
      tm.Minute = temp;
      temp = _display.substring(12, 14).toInt();
      tm.Second = temp;
      local = makeTime(tm);
      utc = _timeZone->toUTC(local);
      _rtc.set(utc);
      setTime(utc);
    }
  }

  void showTime(TimeElements tm, uint8_t position, bool showSeconds, bool space)
  {
    int hour;
    bool pm;

    pm = (tm.Hour > 11);
    if (_hourMode == hour_mode::h12)
    {
      hour = convert24to12(tm.Hour);
    }
    else
    {
      hour = tm.Hour;
    }

    uint8_t currDigit = position;
    uint8_t seconds10 = tm.Second / 10;
    uint8_t seconds01 = tm.Second % 10;
    uint8_t minutes10 = tm.Minute / 10;
    uint8_t minutes01 = tm.Minute % 10;
    uint8_t hours10 = hour / 10;
    uint8_t hours01 = hour % 10;

    if ((_leadingZero == leading_zero::on) || (hours10 != 0))
    {
      _displayHandler->setDigit(currDigit, hours10);
    }
    currDigit++;
    _displayHandler->setDigit(currDigit, hours01);
    if (!space)
    {
      if (tm.Second % 2 == 0)
      {
        _displayHandler->setDecimalPoint(currDigit, decimal_point_state::on);
      }
      else
      {
        _displayHandler->setDecimalPoint(currDigit, decimal_point_state::off);
      }
    }
    else
    {
      currDigit++;
    }
    currDigit++;
    _displayHandler->setDigit(currDigit, minutes10);
    currDigit++;
    _displayHandler->setDigit(currDigit, minutes01);
    if (!space && showSeconds)
    {
      if (tm.Second % 2 == 0)
      {
        _displayHandler->setDecimalPoint(currDigit, decimal_point_state::on);
      }
      else
      {
        _displayHandler->setDecimalPoint(currDigit, decimal_point_state::off);
      }
    }
    else
    {
      currDigit++;
    }
    currDigit++;
    if (showSeconds)
    {
      _displayHandler->setDigit(currDigit, seconds10);
      currDigit++;
      _displayHandler->setDigit(currDigit, seconds01);
    }
  }

  void showDate(TimeElements tm, uint8_t position, year_type yearType, bool space)
  {
    int year = tm.Year + 1970;
    uint8_t currDigit = position;

    uint8_t day10 = tm.Day / 10;
    uint8_t day01 = tm.Day % 10;
    uint8_t month10 = tm.Month / 10;
    uint8_t month01 = tm.Month % 10;
    uint8_t year0001 = year % 10;
    year /= 10;
    uint8_t year0010 = year % 10;
    year /= 10;
    uint8_t year0100 = year % 10;
    uint8_t year1000 = year / 10;

    switch (_dateFormat)
    {
    case date_format::ddmmyy:
      _displayHandler->setDigit(currDigit, day10);
      currDigit++;
      _displayHandler->setDigit(currDigit, day01);
      if (space)
      {
        currDigit++;
      }
      else
      {
        _displayHandler->setDecimalPoint(currDigit, decimal_point_state::on);
      }
      currDigit++;
      _displayHandler->setDigit(currDigit, month10);
      currDigit++;
      _displayHandler->setDigit(currDigit, month01);
      if (space)
      {
        currDigit++;
      }
      else
      {
        if (yearType != year_type::none)
        {
          _displayHandler->setDecimalPoint(currDigit, decimal_point_state::on);
        }
      }
      currDigit++;
      switch (yearType)
      {
      case year_type::full:
        _displayHandler->setDigit(currDigit, year1000);
        currDigit++;
        _displayHandler->setDigit(currDigit, year0100);
        currDigit++;
        _displayHandler->setDigit(currDigit, year0010);
        currDigit++;
        _displayHandler->setDigit(currDigit, year0001);
        break;

      case year_type::partial:
        _displayHandler->setDigit(currDigit, year0010);
        currDigit++;
        _displayHandler->setDigit(currDigit, year0001);
        break;
      }
      break;

    case date_format::mmddyy:
      _displayHandler->setDigit(currDigit, month10);
      currDigit++;
      _displayHandler->setDigit(currDigit, month01);
      if (space)
      {
        currDigit++;
      }
      else
      {
        _displayHandler->setDecimalPoint(currDigit, decimal_point_state::on);
      }
      currDigit++;
      _displayHandler->setDigit(currDigit, day10);
      currDigit++;
      _displayHandler->setDigit(currDigit, day01);
      if (space)
      {
        currDigit++;
      }
      else
      {
        if (yearType != year_type::none)
        {
          _displayHandler->setDecimalPoint(currDigit, decimal_point_state::on);
        }
      }
      currDigit++;
      switch (yearType)
      {
      case year_type::full:
        _displayHandler->setDigit(currDigit, year1000);
        currDigit++;
        _displayHandler->setDigit(currDigit, year0100);
        currDigit++;
        _displayHandler->setDigit(currDigit, year0010);
        currDigit++;
        _displayHandler->setDigit(currDigit, year0001);
        break;

      case year_type::partial:
        _displayHandler->setDigit(currDigit, year0010);
        currDigit++;
        _displayHandler->setDigit(currDigit, year0001);
        break;
      }
      break;

    case date_format::yymmdd:
      switch (yearType)
      {
      case year_type::full:
        _displayHandler->setDigit(currDigit, year1000);
        currDigit++;
        _displayHandler->setDigit(currDigit, year0100);
        currDigit++;
        _displayHandler->setDigit(currDigit, year0010);
        currDigit++;
        _displayHandler->setDigit(currDigit, year0001);
        break;

      case year_type::partial:
        _displayHandler->setDigit(currDigit, year0010);
        currDigit++;
        _displayHandler->setDigit(currDigit, year0001);
        break;
      }
      if (space)
      {
        currDigit++;
      }
      else
      {
        if (yearType != year_type::none)
        {
          _displayHandler->setDecimalPoint(currDigit, decimal_point_state::on);
        }
      }
      currDigit++;
      _displayHandler->setDigit(currDigit, month10);
      currDigit++;
      _displayHandler->setDigit(currDigit, month01);
      if (space)
      {
        currDigit++;
      }
      else
      {
        _displayHandler->setDecimalPoint(currDigit, decimal_point_state::on);
      }
      currDigit++;
      _displayHandler->setDigit(currDigit, day10);
      currDigit++;
      _displayHandler->setDigit(currDigit, day01);
      break;
    }
  }

  void showTemperature(uint8_t position, float temperature)
  {
    uint8_t currDigit = position;
    char buffer[10];

    if ((temperature > (-100)) && (temperature < 200))
    {
      if (temperature < 0)
      {
        _displayHandler->setMinusSign(minus_sign_state::on);
      }
      else
      {
        _displayHandler->setMinusSign(minus_sign_state::off);
      }
      if (temperature < 100)
      {
        sprintf(buffer, "%.1f", abs(temperature));
      }
      else
      {
        sprintf(buffer, "%.0f", abs(temperature));
      }
      String s = buffer;
      for (int i = 0; i < strlen(buffer); i++)
      {
        char c = buffer[i];
        if (c == '.')
        {
          _displayHandler->setDecimalPoint(currDigit - 1, decimal_point_state::on);
        }
        else
        {
          _displayHandler->setDigit(currDigit, (uint8_t)(c - 48));
          currDigit++;
        }
      }
    }
  }

  void displayTime(TimeElements tm)
  {
    switch (_clockMode)
    {
    case clock_mode::time:
      showTimeDefault(tm);
      break;

    case clock_mode::time_no_seconds:
      showTimeNoSeconds(tm);
      break;

    case clock_mode::time_moving:
      showMovingTime(tm);
      break;

    case clock_mode::time_or_date:
      showDateOrTime(tm);
      break;

    case clock_mode::time_and_date:
      showTimeAndDate(tm);
      break;

    case clock_mode::time_and_temp:
      showTimeAndTemp(tm);
      break;

    case clock_mode::time_and_date_and_temp:
      showTimeAndDateAndTemp(tm);
      break;

    case clock_mode::date_and_time_raw:
      showDateTimeRaw(tm);
      break;

    case clock_mode::timer:
      showTimer();
      break;

    case clock_mode::stopwatch:
      showStopWatch();
      break;
    }
  }

  void showTimeDefault(TimeElements tm)
  {
    showTime(tm, 3, true, true);
  }

  void showTimeNoSeconds(TimeElements tm)
  {
    showTime(tm, 4, false, true);
  }

  void
  showMovingTime(TimeElements tm)
  {
    static int lastSecond = 0;
    static bool leftDirection = true;
    static int position = 3;

    if (tm.Second != lastSecond)
    {
      _displayHandler->clearDisplay();
      lastSecond = tm.Second;
      if (leftDirection)
      {
        if (position == 6)
        {
          leftDirection = false;
          position--;
        }
        else
        {
          position++;
        }
      }
      else
      {
        int final = 0;
        if ((_leadingZero == leading_zero::off) && ((tm.Hour) / 10 == 0))
        {
          final = (-1);
        }

        if (position == final)
        {
          leftDirection = true;
          position++;
        }
        else
        {
          position--;
        }
      }
    }
    showTime(tm, position, true, true);
  }

  void showDateOrTime(TimeElements tm)
  {
    static bool isDate = false;
    if ((tm.Second > 50) && (tm.Second < 55))
    {
      if (!isDate)
      {
        scrollOutTime(tm);
        _displayHandler->clearDisplay();
        isDate = true;
      }
      showDate(tm, 2, year_type::full, true);
    }
    else
    {
      if (isDate)
      {
        scrollOutDate(tm);
        _displayHandler->clearDisplay();
        isDate = false;
      }
      showTime(tm, 3, true, true);
    }
  }

  // scroll over every digits for poisoning prevention
  void antiPoisoning(bool mode)
  {
    _displayHandler->clearDisplay();
    for (int i = 0; i < 10; i++)
    {
      for (int j = 0; j < _displayHandler->getDigitCount(); j++)
      {
        _displayHandler->setDigit(j, i);
        _displayHandler->show();
      }
      delay(75);
    }
  }

  void scrollOutTime(TimeElements tm)
  {

    for (int i = 2; i > (-8); i--)
    {
      _displayHandler->clearDisplay();
      showTime(tm, i, true, true);
      _displayHandler->show();
      delay(75);
    }
  }

  void scrollOutDate(TimeElements tm)
  {
    for (int i = 1; i > (-10); i--)
    {
      _displayHandler->clearDisplay();
      showDate(tm, i, year_type::full, true);
      _displayHandler->show();
      delay(75);
    }
  }

  void showTimeAndDate(TimeElements tm)
  {
    showTime(tm, 0, true, false);
    showDate(tm, 8, year_type::partial, false);
  }

  void showTimer()
  {
    uint8_t position = 1;
    uint64_t remainingMillis;

    switch (_timerMode)
    {
    case timer_mode::set:
      remainingMillis = _setMillis;
      _elapsedMillis = 0;
      break;

    case timer_mode::zero:
      remainingMillis = 0;
      break;

    case timer_mode::running:
      remainingMillis = (_setMillis - _elapsedMillis) - ((esp_timer_get_time() / 1000ULL) - _startMillis);
      break;

    case timer_mode::stopped:
      remainingMillis = _setMillis - _elapsedMillis;
      break;
    }

    if (remainingMillis > _setMillis)
    {
      _timerMode = timer_mode::zero;
      remainingMillis = 0;
    }
    unsigned long ts = (remainingMillis / 1000) % 60;
    unsigned long tm = (remainingMillis / 60000) % 60;
    unsigned long th = (remainingMillis / 3600000) % 24;
    unsigned long td = (remainingMillis / 86400000) % 100;

    _displayHandler->setDigit(position + 0, td / 10);
    _displayHandler->setDigit(position + 1, td % 10);
    _displayHandler->setDigit(position + 3, th / 10);
    _displayHandler->setDigit(position + 4, th % 10);
    _displayHandler->setDigit(position + 6, tm / 10);
    _displayHandler->setDigit(position + 7, tm % 10);
    _displayHandler->setDigit(position + 9, ts / 10);
    _displayHandler->setDigit(position + 10, ts % 10);
  }

  void showStopWatch()
  {
    static uint64_t startMillis = 0;
    static uint64_t elsapsedMillis = 0;
    static bool running = false;
    int position = 1;

    switch (_stopwatchMode)
    {
    case stopwatch_mode::zero:
      elsapsedMillis = 0;
      startMillis = 0;
      running = false;
      break;

    case stopwatch_mode::running:
      if (running)
      {
        elsapsedMillis = (esp_timer_get_time() / 1000ULL) - startMillis;
      }
      else
      {
        running = true;
        if (startMillis == 0)
        {
          startMillis = (esp_timer_get_time() / 1000ULL);
          elsapsedMillis = 0;
        }
      }
      break;

    case stopwatch_mode::stopped:
      if (running)
      {
        running = false;
        elsapsedMillis = (esp_timer_get_time() / 1000ULL) - startMillis;
      }
      break;
    }

    // show elapsed
    unsigned long tc = elsapsedMillis % 1000;
    unsigned long ts = (elsapsedMillis / 1000) % 60;
    unsigned long tm = (elsapsedMillis / 60000) % 60;
    unsigned long th = (elsapsedMillis / 3600000) % 24;

    _displayHandler->setDigit(position, th / 10);
    _displayHandler->setDigit(position + 1, th % 10);
    _displayHandler->setDigit(position + 3, tm / 10);
    _displayHandler->setDigit(position + 4, tm % 10);
    _displayHandler->setDigit(position + 6, ts / 10);
    _displayHandler->setDigit(position + 7, ts % 10);
    _displayHandler->setDigit(position + 9, (tc / 10) / 10);
    _displayHandler->setDigit(position + 10, (tc / 10) % 10);
  }

  void showTimeAndTemp(TimeElements tm)
  {
    showTime(tm, 0, true, true);
    showTemperature(11, _temperature);
  }

  void showTimeAndDateAndTemp(TimeElements tm)
  {
    showTime(tm, 0, false, false);
    showDate(tm, 5, year_type::none, false);
    showTemperature(11, _temperature);
  }

  void showDateTimeRaw(TimeElements tm)
  {
    int year = tm.Year + 1970;
    uint8_t day10 = tm.Day / 10;
    uint8_t day01 = tm.Day % 10;
    uint8_t month10 = tm.Month / 10;
    uint8_t month01 = tm.Month % 10;
    uint8_t year0001 = year % 10;
    year /= 10;
    uint8_t year0010 = year % 10;
    year /= 10;
    uint8_t year0100 = year % 10;
    uint8_t year1000 = year / 10;
    uint8_t seconds10 = tm.Second / 10;
    uint8_t seconds01 = tm.Second % 10;
    uint8_t minutes10 = tm.Minute / 10;
    uint8_t minutes01 = tm.Minute % 10;
    uint8_t hours10 = tm.Hour / 10;
    uint8_t hours01 = tm.Hour % 10;
    _displayHandler->setDigit(0, year1000);
    _displayHandler->setDigit(1, year0100);
    _displayHandler->setDigit(2, year0010);
    _displayHandler->setDigit(3, year0001);
    _displayHandler->setDigit(4, month10);
    _displayHandler->setDigit(5, month01);
    _displayHandler->setDigit(6, day10);
    _displayHandler->setDigit(7, day01);
    _displayHandler->setDigit(8, hours10);
    _displayHandler->setDigit(9, hours01);
    _displayHandler->setDigit(10, minutes10);
    _displayHandler->setDigit(11, minutes01);
    _displayHandler->setDigit(12, seconds10);
    _displayHandler->setDigit(13, seconds01);
  }

  void showInput(TimeElements tm)
  {
    static unsigned long lastMillis = millis();
    static bool show = true;

    if (millis() - lastMillis > 250)
    {
      show = !show;
      lastMillis = millis();
    }
    if (show)
    {
      _displayHandler->show(_display);
    }
    else
    {
      _displayHandler->clearDisplay();
    }
  }

  void numericInput(uint8_t digit)
  {
    // during time input, process numeric input
    switch (_inputMode)
    {
    case input_mode::time:
      // avoid leading zeroes
      if (_display.equals("0"))
      {
        _display = char((digit + 48));
      }
      else
      {
        if (_display.length() < _displayHandler->getDigitCount())
        {
          _display += char((digit + 48));
        }
      }
      break;

    case input_mode::timer:
      if (_display.equals("0"))
      {
        _display = char((digit + 48));
      }
      else
      {
        if (_display.length() < MAX_TIMER_INPUT)
        {
          _display += char((digit + 48));
        }
      }
      break;

    case input_mode::none:
      switch ((clock_mode::clock_mode)digit)
      {

      case clock_mode::time:
      case clock_mode::time_no_seconds:
      case clock_mode::time_moving:
      case clock_mode::time_or_date:
      case clock_mode::time_and_date:
      case clock_mode::time_and_temp:
      case clock_mode::time_and_date_and_temp:
      case clock_mode::date_and_time_raw:
      case clock_mode::timer:
      case clock_mode::stopwatch:
        _displayHandler->clearDisplay();
        _clockMode = (clock_mode::clock_mode)digit;
        break;
      }
      break;
    }
  }

  void operationInput(operation op)
  {
    switch (op)
    {
    case operation::allclear:
      switch (_clockMode)
      {
      case clock_mode::stopwatch:
        break;

      case clock_mode::timer:
        if (_inputMode == input_mode::none)
        {
          _inputMode = input_mode::timer;
          _display = "0";
          _displayHandler->show(_display);
        }
        else
        {
          _inputMode = input_mode::none;
          _displayHandler->clearDigits();
        }
        break;

        break;

      default:
        if (_inputMode == input_mode::none)
        {
          _inputMode = input_mode::time;
          _display = "0";
          _displayHandler->show(_display);
        }
        else
        {
          _inputMode = input_mode::none;
          _displayHandler->clearDigits();
        }
        break;
      }
      break;

    case operation::clear:
      switch (_clockMode)
      {
      case clock_mode::stopwatch:
        _stopwatchMode = stopwatch_mode::zero;
        break;

      case clock_mode::timer:
        _timerMode = timer_mode::set;
        _elapsedMillis = 0;
        break;

      default:
        if (_inputMode != input_mode::none)
        {
          _display = "0";
          _displayHandler->show(_display);
        }
      }
      break;

    case operation::equals:
      switch (_clockMode)
      {
      case clock_mode::stopwatch:
        if (_stopwatchMode == stopwatch_mode::running)
        {
          _stopwatchMode = stopwatch_mode::stopped;
        }
        else
        {
          _stopwatchMode = stopwatch_mode::running;
        }
        break;

      case clock_mode::timer:
        if (_inputMode == input_mode::timer)
        {
          _inputMode = input_mode::none;
          _displayHandler->clearDigits();
          setTimer();
        }
        else
        {
          switch (_timerMode)
          {
          case timer_mode::running:
            _timerMode = timer_mode::stopped;
            _elapsedMillis += (esp_timer_get_time() / 1000ULL) - _startMillis;
            break;

          case timer_mode::set:
          case timer_mode::stopped:
            _timerMode = timer_mode::running;
            _startMillis = esp_timer_get_time() / 1000ULL;
            break;
          }
        }
        break;

      default:
        switch (_inputMode)
        {
        case input_mode::time:
          _inputMode = input_mode::none;
          _displayHandler->clearDigits();
          setRTCTime();
          break;
        }
        break;
      }
    default:
      break;
    }
  }

  void setTimer()
  {
    u_int64_t interval = 0;
    if (_display.length() <= MAX_TIMER_INPUT)
    {
      int count = MAX_TIMER_INPUT - _display.length();
      for (int i = 0; i < count; i++)
      {
        _display = "0" + _display;
      }
      int temp = 0;
      temp = _display.substring(0, 2).toInt();
      interval += temp * 86400;
      temp = _display.substring(2, 4).toInt();
      interval += temp * 3600;
      temp = _display.substring(4, 6).toInt();
      interval += temp * 60;
      temp = _display.substring(6, 8).toInt();
      interval += temp;
      if (interval <= MAX_TIMER_INTERVAL)
      {
        interval *= 1000;
        interval += 999;
        _setMillis = interval;
        _elapsedMillis = 0;
        _timerMode = timer_mode::set;
      }
    }
  }

  int convert24to12(int hour24)
  {
    int hour12;

    hour12 = hour24 % 12;
    if (hour12 == 0)
    {
      hour12 += 12;
    }
    return (hour12);
  }
};