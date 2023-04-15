// SettingEnum.h

// holds seting value

// Copyright (C) 2023 highvoltglow
// Licensed under the MIT License

#pragma once

#define MAX_RGB_INT 16777215
#define MAX_TIME_INT 1439

// enums

// settings
namespace setting_id
{
  enum setting_id
  {
    startupmode = 1, // Start in calculator or in clock mode
    showversion,     // Show version during start-up
    autooffmode,     // Shutdown high voltage or switch to clock mode after a period of no keyboard activity
    autooffdelay,    // Delay in minutes for auto off mode
    clockmode,       // Predefined display format in clock mode
    hourmode,        // 12 or 24 hours mode
    leadingzero,     // Hours leading zero off or on
    dateformat,      // Date format
    pirmode,         // PIR off or on
    pirdelay,        // PIR delay time in minutes before shutting down the high voltage
    gpsmode,         // GPS time sync off or on
    gpsspeed,        // GPS communication baud rate
    gpssyncinterval, // GPS time sync interval in minutes
    temperaturemode, // Temperature sensor off or on
    temperaturecf,   // Temperature in C or F
    ledmode,         // LEDs on by time or always
    ledrange,        // All LEDs or only LEDs where the correspondent nixie is on
    calcrgbmode,     // Fixed colors, wheel, or random colors in calculator mode
    clockrgbmode,    // Fixed colors, wheel, or random colors in clock mode
    ledstarttime,    // Start time of LED lighting
    ledduration,     // Duration in minutes of LED lighting
    zeropadding,     // Left padding with zeroes in calculator mode off or on
    flickermode,     // Simulate display flickering of an old calculator off or on
    acpstarttime,    // Start time of cathode poisoning prevention
    acpduration,     // Duration in minutes of cathode poisoning prevention
    acpforceon,      // Force turning nixies on during cathode poisoning prevention
    negativecolor,   // RGB led color for negative numbers in calculator mode
    positivecolor,   // RGB led color for positive numbers in calculator mode
    errorcolor,      // RGB led color for error in calculator mode
    timecolor,       // RGB led color for time in clock mode
    datecolor,       // RGB led color for date in clock mode
    tempcolor,       // RGB led color for temperature in clock mode
    dstweek,         // Daylight saving time change, week of month
    dstdow,          // Daylight saving time change, day of week
    dstmonth,        // Daylight saving time change, month
    dsthour,         // Daylight saving time change, hour
    dstoffset,       // Daylight saving time change, offset to UTC in minutes
    stdweek,         // Standard time change, week of month
    stddow,          // Standard time change, day of week
    stdmonth,        // Standard time change, month
    stdhour,         // Standard time change, hour
    stdoffset        // Standard time change, offset to UTC in minutes
  };
}

namespace startup_mode
{
  enum startup_mode
  {
    calculator,
    clock
  };
}
namespace show_version
{
  enum show_version
  {
    off,
    on
  };
}

namespace auto_off_mode
{
  enum auto_off_mode
  {
    off,
    on,
    clock
  };
}

namespace clock_mode
{
  enum clock_mode
  {
    time,
    time_no_seconds,
    time_moving,
    time_or_date,
    time_and_date,
    time_and_temp,
    time_and_date_and_temp,
    date_and_time_raw,
    timer,
    stopwatch
  };
}

namespace hour_mode
{
  enum hour_mode
  {
    h12,
    h24
  };
}

namespace leading_zero
{
  enum leading_zero
  {
    off,
    on
  };
}

namespace date_format
{
  enum date_format
  {
    ddmmyy,
    yymmdd,
    mmddyy
  };
}

namespace pir_mode
{
  enum pir_mode
  {
    off,
    on
  };
}

namespace gps_mode
{
  enum gps_mode
  {
    off,
    on
  };
}

namespace gps_speed
{
  enum gps_speed
  {
    br_2400,
    br_4800,
    br_9600,
    br_19200,
    br_38400,
    br_57600,
    br_115200
  };
}

namespace temperature_mode
{
  enum temperature_mode
  {
    off,
    on
  };
}

namespace temperature_cf
{
  enum temperature_cf
  {
    celsius,
    fahrenheit
  };
}

namespace led_mode
{
  enum led_mode
  {
    time,
    always
  };
}

namespace led_range
{
  enum led_range
  {
    all,
    nixie
  };
}

namespace calc_rgb_mode
{
  enum calc_rgb_mode
  {
    off,
    fixed,
    wheel,
    random
  };
}

namespace clock_rgb_mode
{
  enum clock_rgb_mode
  {
    off,
    fixed,
    wheel,
    random
  };
}

namespace zero_padding
{
  enum zero_padding
  {
    off,
    on
  };
}

namespace flicker_mode
{
  enum flicker_mode
  {
    off,
    on
  };
}

namespace acp_force_on
{
  enum acp_force_on
  {
    off,
    on
  };
}