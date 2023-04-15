// Settings.h

// stores and retrieves non-volatile settings

// Copyright (C) 2023 highvoltglow
// Licensed under the MIT License

// Settings.h

#pragma once

#include <Arduino.h>
#include <Timezone.h>
#include <nvs_flash.h>
#include <Preferences.h>
#include <Setting.h>
#include <map>

// definitions
#define SETTINGS_NAMESPACE "CalcSettings"
#define SETTINGS_VERSION 1

typedef std::map<setting_id::setting_id, Setting *> SETTINGSMAP;

class Settings
{
public:
  Settings()
  {
    _settings[setting_id::startupmode] = new Setting(setting_id::startupmode, setting_type::numeric, startup_mode::calculator, startup_mode::calculator, startup_mode::clock);
    _settings[setting_id::showversion] = new Setting(setting_id::showversion, setting_type::numeric, show_version::on, show_version::off, show_version::on);
    _settings[setting_id::autooffmode] = new Setting(setting_id::autooffmode, setting_type::numeric, auto_off_mode::clock, auto_off_mode::off, auto_off_mode::clock);
    _settings[setting_id::autooffdelay] = new Setting(setting_id::autooffdelay, setting_type::numeric, 5, 1, 720);
    _settings[setting_id::clockmode] = new Setting(setting_id::clockmode, setting_type::numeric, clock_mode::time, clock_mode::time, clock_mode::stopwatch);
    _settings[setting_id::hourmode] = new Setting(setting_id::hourmode, setting_type::numeric, hour_mode::h24, hour_mode::h12, hour_mode::h24);
    _settings[setting_id::leadingzero] = new Setting(setting_id::leadingzero, setting_type::numeric, leading_zero::on, leading_zero::off, leading_zero::on);
    _settings[setting_id::dateformat] = new Setting(setting_id::dateformat, setting_type::numeric, date_format::ddmmyy, date_format::ddmmyy, date_format::mmddyy);
    _settings[setting_id::pirmode] = new Setting(setting_id::pirmode, setting_type::numeric, pir_mode::off, pir_mode::off, pir_mode::on);
    _settings[setting_id::pirdelay] = new Setting(setting_id::pirdelay, setting_type::numeric, 5, 1, 720);
    _settings[setting_id::gpsmode] = new Setting(setting_id::gpsmode, setting_type::numeric, gps_mode::off, gps_mode::off, gps_mode::on);
    _settings[setting_id::gpsspeed] = new Setting(setting_id::gpsspeed, setting_type::numeric, gps_speed::br_38400, gps_speed::br_2400, gps_speed::br_115200);
    _settings[setting_id::gpssyncinterval] = new Setting(setting_id::gpssyncinterval, setting_type::numeric, 60, 1, 720);
    _settings[setting_id::temperaturemode] = new Setting(setting_id::temperaturemode, setting_type::numeric, temperature_mode::off, temperature_mode::off, temperature_mode::on);
    _settings[setting_id::temperaturecf] = new Setting(setting_id::temperaturecf, setting_type::numeric, temperature_cf::celsius, temperature_cf::celsius, temperature_cf::fahrenheit);
    _settings[setting_id::ledmode] = new Setting(setting_id::ledmode, setting_type::numeric, led_mode::always, led_mode::time, led_mode::always);
    _settings[setting_id::ledrange] = new Setting(setting_id::ledrange, setting_type::numeric, led_range::all, led_range::all, led_range::nixie);
    _settings[setting_id::calcrgbmode] = new Setting(setting_id::calcrgbmode, setting_type::numeric, calc_rgb_mode::off, calc_rgb_mode::off, calc_rgb_mode::random);
    _settings[setting_id::clockrgbmode] = new Setting(setting_id::clockrgbmode, setting_type::numeric, clock_rgb_mode::off, clock_rgb_mode::off, clock_rgb_mode::random);
    _settings[setting_id::ledstarttime] = new Setting(setting_id::ledstarttime, setting_type::time, 0, 0, MAX_TIME_INT);
    _settings[setting_id::ledduration] = new Setting(setting_id::ledduration, setting_type::numeric, 0, 0, 720);
    _settings[setting_id::zeropadding] = new Setting(setting_id::zeropadding, setting_type::numeric, zero_padding::off, zero_padding::off, zero_padding::on);
    _settings[setting_id::flickermode] = new Setting(setting_id::flickermode, setting_type::numeric, flicker_mode::off, flicker_mode::off, flicker_mode::on);
    _settings[setting_id::acpstarttime] = new Setting(setting_id::acpstarttime, setting_type::time, 0, 0, MAX_TIME_INT);
    _settings[setting_id::acpduration] = new Setting(setting_id::acpduration, setting_type::numeric, 0, 0, 720);
    _settings[setting_id::acpforceon] = new Setting(setting_id::acpforceon, setting_type::numeric, acp_force_on::on, acp_force_on::off, acp_force_on::on);
    _settings[setting_id::negativecolor] = new Setting(setting_id::negativecolor, setting_type::rgb, 0, 0, MAX_RGB_INT);
    _settings[setting_id::positivecolor] = new Setting(setting_id::positivecolor, setting_type::rgb, 0, 0, MAX_RGB_INT);
    _settings[setting_id::errorcolor] = new Setting(setting_id::errorcolor, setting_type::rgb, 0, 0, MAX_RGB_INT);
    _settings[setting_id::timecolor] = new Setting(setting_id::timecolor, setting_type::rgb, 0, 0, MAX_RGB_INT);
    _settings[setting_id::datecolor] = new Setting(setting_id::datecolor, setting_type::rgb, 0, 0, MAX_RGB_INT);
    _settings[setting_id::tempcolor] = new Setting(setting_id::tempcolor, setting_type::rgb, 0, 0, MAX_RGB_INT);
    _settings[setting_id::dstweek] = new Setting(setting_id::dstweek, setting_type::numeric, week_t::Last, week_t::Last, week_t::Fourth);
    _settings[setting_id::dstdow] = new Setting(setting_id::dstdow, setting_type::numeric, dow_t::Sun, dow_t::Sun, dow_t::Sat);
    _settings[setting_id::dstmonth] = new Setting(setting_id::dstmonth, setting_type::numeric, month_t::Mar, month_t::Jan, month_t::Dec);
    _settings[setting_id::dsthour] = new Setting(setting_id::dsthour, setting_type::numeric, 2, 0, 23);
    _settings[setting_id::dstoffset] = new Setting(setting_id::dstoffset, setting_type::numeric, 120, -720, 840);
    _settings[setting_id::stdweek] = new Setting(setting_id::stdweek, setting_type::numeric, week_t::Last, week_t::Last, week_t::Fourth);
    _settings[setting_id::stddow] = new Setting(setting_id::stddow, setting_type::numeric, dow_t::Sun, dow_t::Sun, dow_t::Sat);
    _settings[setting_id::stdmonth] = new Setting(setting_id::stdmonth, setting_type::numeric, month_t::Oct, month_t::Jan, month_t::Dec);
    _settings[setting_id::stdhour] = new Setting(setting_id::stdhour, setting_type::numeric, 3, 0, 23);
    _settings[setting_id::stdoffset] = new Setting(setting_id::stdoffset, setting_type::numeric, 60, -720, 840);
  }

  virtual ~Settings()
  {
    // free memory
    for (const auto &value : _settings)
    {
      delete value.second;
    }
    _settings.clear();
  }

  bool begin()
  {
    bool retVal = _preferences.begin(SETTINGS_NAMESPACE, false);
    if (!retVal)
    {

      // try to initialize
      nvs_flash_init();
      retVal = _preferences.begin(SETTINGS_NAMESPACE, false);
    }
    return (retVal);
  }

  void end()
  {
    _preferences.end();
  }

  void readSettings()
  {
    for (const auto &value : _settings)
    {
      int temp = getSetting(value.first, value.second->getDefault());
      if ((temp > value.second->getMax()) || (temp < value.second->getMin()))
      {
        // value is not valid, set to default
        temp = value.second->getDefault();
      }
      value.second->set(temp);
      value.second->resetModified();
    }
  }

  const SETTINGSMAP &getSettingsMap() const
  {
    return (_settings);
  }

  void storeSettings()
  {
    Serial.println("store settings");
    for (const auto &value : _settings)
    {
      // store only if modified
      if (value.second->modified())
      {
        setSetting(value.first, value.second->get());
        // reset modified flag
        value.second->resetModified();
        Serial.println("modified");
      }
    }
  }

  bool getSetting(setting_id::setting_id id, int *result)
  {
    *result = 0;
    bool success = false;
    SETTINGSMAP::iterator it;
    it = _settings.find(id);
    if (it != _settings.end())
    {
      *result = it->second->get();
      success = true;
    }
    return (success);
  }

private:
  Preferences _preferences;
  SETTINGSMAP _settings;

  void resetDefaults()
  {
    for (const auto &value : _settings)
    {
      value.second->reset();
    }
  }

  int getSetting(setting_id::setting_id id, int defValue)
  {
    String s(id);
    return (_preferences.getInt(s.c_str(), defValue));
  }

  void setSetting(setting_id::setting_id id, int value)
  {
    String s(id);
    _preferences.putInt(s.c_str(), value);
  }
};
