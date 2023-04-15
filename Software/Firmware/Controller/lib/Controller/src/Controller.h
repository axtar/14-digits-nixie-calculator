// Controller.h

// Provides

// Copyright (C) 2023 highvoltglow
// Licensed under the MIT License

#pragma once

#include <Arduino.h>
#include <time.h>
#include <FirmwareInfo.h>
#include <HardwareInfo.h>
#include <Errors.h>
#include <Wire.h>
#include <SoftwareSerial.h>
#include <KeyboardHandler.h>
#include <Settings.h>
#include <DisplayHandler.h>
#include <Calculator.h>
#include <Clock.h>
#include <PIR.h>
#include <GPS.h>
#include <Temperature.h>
#include <MenuHandler.h>

// pin definitions
#define PIN_HVENABLE 4
#define PIN_HVLED 5
#define PIN_SHIFT 17
#define PIN_STORE 16
#define PIN_DATA 18
#define PIN_BLANK 19
#define PIN_LEDCTL 14
#define PIN_GPSTX 33
#define PIN_GPSRX 32
#define PIN_PIR 35
#define PIN_KINT 27
#define PIN_TEMPERATURE 25
#define PIN_BUTTON1 34
#define PIN_NETACT 12

// ENUMS
enum class device_mode : uint8_t
{
  calculator,
  clock,
  menu
};

// initial device mode
device_mode deviceMode = device_mode::calculator;
device_mode prevDeviceMode = device_mode::calculator;

class Controller
{
public:
  Controller()
      : _keyboardCom(PIN_KINT),
        _displayHandler(DISPLAY_TYPE, PIN_DATA, PIN_STORE, PIN_SHIFT, PIN_BLANK, PIN_LEDCTL),
        _clock(&_settings, &_displayHandler),
        _calculator(&_settings),
        _pir(&_settings),
        _gps(&_settings),
        _temperature(PIN_TEMPERATURE, &_settings),
        _menuHandler(&_settings)
  {
    _highVoltageOn = true;
    _backLight = false;
    _autoOff = false;
  }

  virtual ~Controller()
  {
  }

  int begin()
  {
    int result = ERR_SUCCESS;
    // set high voltage off
    pinMode(PIN_HVENABLE, OUTPUT);
    pinMode(PIN_HVLED, OUTPUT);
    hvOFF();

    // define pin modes
    pinMode(PIN_PIR, INPUT);
    pinMode(PIN_DATA, OUTPUT);
    pinMode(PIN_STORE, OUTPUT);
    pinMode(PIN_SHIFT, OUTPUT);
    pinMode(PIN_BLANK, OUTPUT);
    pinMode(PIN_BUTTON1, INPUT);
    // blank registers
    digitalWrite(PIN_BLANK, LOW);
    digitalWrite(PIN_BLANK, HIGH);
    if (_settings.begin())
    {
      _settings.readSettings();
      setParameters();
      // init and clear display
      _displayHandler.begin();
      _displayHandler.clearLEDs();
      _displayHandler.clearDisplay();

      // init calculator
      _calculator.begin(_displayHandler.getDigitCount(), _displayHandler.getDecimalPointCount(), _displayHandler.hasPlusSign());

      // init clock
      _clock.begin();

      // init menu handler
      _menuHandler.begin(_displayHandler.getDigitCount());

      if (_pirMode == pir_mode::on)
      {
        // init PIR
        _pir.begin(PIN_PIR);
      }

      if (_gpsMode == gps_mode::on)
      {
        // init GPS part 1
        _gps.begin(PIN_GPSRX, PIN_GPSTX);
      }

      if (_temperatureMode == temperature_mode::on)
      {
        _temperature.begin();
      }

      // init I2C
      Wire.begin();

      // init keyboard stuff
      _keyboardCom.begin(9600);
      _keyboard.begin(_keyboardCom);
      _keyboard.attach(this, onKeyboardEventCallback);

      // give keyboard time to start
      delay(500);

      // init GPS part 1
      if (_gpsMode == gps_mode::on)
      {
        _gps.initialize();
        _gps.attach(this, onGPSTimeSyncEventCallback);
      }

      // requests version from keyboard
      _keyboard.requestVersion();
      _keyboard.setAutoRepeatInterval(0);
      _keyboard.setHoldTime(2000);
      // turn on high voltage
      hvON();

      // show version
      if (_showVersion == show_version::on)
      {
        showVersion();
        delay(1000);
      }

      // set startup mode
      switch (_startupMode)
      {
      case startup_mode::calculator:
        deviceMode = device_mode::calculator;
        prevDeviceMode = device_mode::calculator;
        break;

      case startup_mode::clock:
        deviceMode = device_mode::clock;
        prevDeviceMode = device_mode::clock;
        break;
      }

      // go to defined mode
      switch (deviceMode)
      {
      case device_mode::calculator:
        _displayHandler.show(_calculator.getDisplay());
        break;

      case device_mode::clock:
        _displayHandler.clear();
        break;

      default:
        break;
      }
    }
    else
    {
      result = ERR_INITSETTINGS;
    }
    return (result);
  }

  void process()
  {

    // process keyboard input
    _keyboard.process();
    checkAutoOff();

    if (_gpsMode == gps_mode::on)
    {
      _gps.process();
    }

    if (_pirMode == pir_mode::on)
    {
      if (_pir.process())
      {
        hvON();
        if (_backLight)
        {
          setBackLight();
        }
      }
      else
      {
        hvOFF();
        _displayHandler.clearLEDs();
      }
    }

    if (_temperatureMode == temperature_mode::on)
    {
      _temperature.process();
      _clock.setTemperature(_temperature.getTemperature());
    }

    switch (deviceMode)
    {
    case device_mode::clock:
      _clock.process();
      // the clock writes directly into the display buffer
      _displayHandler.show();
      break;

    default:
      break;
    }
  }

  void setParameters()
  {
    _settings.getSetting(setting_id::pirmode, (int *)&_pirMode);
    _settings.getSetting(setting_id::gpsmode, (int *)&_gpsMode);
    _settings.getSetting(setting_id::temperaturemode, (int *)&_temperatureMode);
    _settings.getSetting(setting_id::startupmode, (int *)&_startupMode);
    _settings.getSetting(setting_id::showversion, (int *)&_showVersion);
    _settings.getSetting(setting_id::autooffmode, (int *)&_autoOffMode);
    _settings.getSetting(setting_id::autooffdelay, &_autoOffDelay);
  }

  void hvON()
  {
    if (!_highVoltageOn)
    {
      _highVoltageOn = true;
      digitalWrite(PIN_HVENABLE, HIGH);
      digitalWrite(PIN_HVLED, HIGH);
    }
  }

  void hvOFF()
  {
    if (_highVoltageOn)
    {
      _highVoltageOn = false;
      digitalWrite(PIN_HVENABLE, LOW);
      digitalWrite(PIN_HVLED, LOW);
    }
  }

  bool isHVON()
  {
    return (_highVoltageOn);
  }

  // sets the device mode:
  // pressing the function key toogles between calculator and clock mode
  void switchDeviceMode()
  {
    _keyboard.setAutoRepeatInterval(0);
    _keyboard.setFastAutoRepeatDelay(0);
    _keyboard.setFastAutoRepeatInterval(0);
    _keyboard.setHoldTime(2000);
    _displayHandler.clearDisplay();
    switch (deviceMode)
    {
    case device_mode::calculator:
      deviceMode = device_mode::clock;
      break;

    case device_mode::clock:
      deviceMode = device_mode::calculator;
      break;

    case device_mode::menu:
      _settings.storeSettings();
      deviceMode = prevDeviceMode;
      _clock.setSettings();
      _temperature.setSettings();
      break;
    }
  }

  // switches to menu mode
  // pressing and holding the function key for more than 3 seconds
  void switchToMenuMode()
  {
    if (deviceMode != device_mode::menu)
    {
      _keyboard.setAutoRepeatInterval(250);
      _keyboard.setFastAutoRepeatInterval(25);
      _keyboard.setHoldTime(1000);
      _keyboard.setFastAutoRepeatDelay(15);
      prevDeviceMode = deviceMode;
      deviceMode = device_mode::menu;
    }
  }

  static void onKeyboardEventCallback(void *obj, uint8_t keyCode, key_state keyState, bool functionKeyPressed, special_keyboard_event specialEvent)
  {
    ((Controller *)obj)->onKeyboardEvent(keyCode, keyState, functionKeyPressed, specialEvent);
  }

  static void onGPSTimeSyncEventCallback(void *obj, time_t utc)
  {
    ((Controller *)obj)->onGPSTimeSyncEvent(utc);
  }

private:
  bool _highVoltageOn;
  bool _backLight;
  Settings _settings;
  DisplayHandler _displayHandler;
  KeyboardHandler _keyboard;
  SoftwareSerial _keyboardCom;
  Clock _clock;
  Calculator _calculator;
  PIR _pir;
  GPS _gps;
  Temperature _temperature;
  MenuHandler _menuHandler;
  // settings
  pir_mode::pir_mode _pirMode;
  gps_mode::gps_mode _gpsMode;
  temperature_mode::temperature_mode _temperatureMode;
  startup_mode::startup_mode _startupMode;
  show_version::show_version _showVersion;
  auto_off_mode::auto_off_mode _autoOffMode;
  int _autoOffDelay;
  bool _autoOff;

  void showVersion()
  {
    char buffer[20];
    sprintf(buffer, "%02u.%02u.%02u  %02u.%02u.%02u", MAJOR_VERSION, MINOR_VERSION, REVISION,
            _keyboard.getMajorVersion(), _keyboard.getMinorVersion(), _keyboard.getRevision());
    _displayHandler.show(buffer);
  }

  void onKeyboardEvent(uint8_t keyCode, key_state keyState, bool functionKeyPressed, special_keyboard_event specialEvent)
  {
    // Serial.print("Keycode: ");
    // Serial.print(keyCode);
    // Serial.print("  Keystate: ");
    // Serial.print((int)keyState);
    // Serial.print("  Special event: ");
    // Serial.print((int)specialEvent);
    // Serial.print("  Function key pressed: ");
    // Serial.println(functionKeyPressed);

    if (!_autoOff)
    {
      switch (specialEvent)
      {
      case special_keyboard_event::mode_switch:
        switchDeviceMode();
        break;

      case special_keyboard_event::menu_mode:
        switchToMenuMode();
        break;
      }

      if (functionKeyPressed && (keyState == key_state::pressed))
      {
        handleShortcuts(keyCode);
      }

      switch (deviceMode)
      {
      case device_mode::calculator:
        // calculator is keyboard driven, send key event and update display
        _calculator.onKeyboardEvent(keyCode, keyState, functionKeyPressed);
        _displayHandler.show(_calculator.getDisplay());
        break;

      case device_mode::clock:
        // the clock also needs some keyboard events for setting
        // the time and changing the clock mode
        _clock.onKeyboardEvent(keyCode, keyState, functionKeyPressed);
        break;

      case device_mode::menu:
        // menus are keyboard driven, send key event and update display
        _menuHandler.onKeyboardEvent(keyCode, keyState, functionKeyPressed);
        _displayHandler.show(_menuHandler.getDisplay());
        _displayHandler.setAllLED(_menuHandler.getRed(), _menuHandler.getGreen(), _menuHandler.getBlue());
        _displayHandler.updateLEDs();
        break;

      default:
        break;
      }
    }
  }

  void handleShortcuts(uint8_t keyCode)
  {
    switch (keyCode)
    {
    case KEY_C:
      if (!_backLight)
      {
        setBackLight();
        _backLight = true;
      }
      else
      {
        _displayHandler.clearLEDs();
        _backLight = false;
      }
      break;
    }
  }

  void onGPSTimeSyncEvent(time_t utc)
  {
    _clock.setRTCTime(utc);
  }

  void checkAutoOff()
  {
    if (_autoOffMode != auto_off_mode::off)
    {
      if (millis() - _keyboard.getLastKeyTimestamp() > (_autoOffDelay * 60 * 1000))
      {
        switch (_autoOffMode)
        {
        case auto_off_mode::on:
          _autoOff = true;
          hvOFF();
          break;

        case auto_off_mode::clock:
          deviceMode = device_mode::clock;
          _displayHandler.clear();
          break;
        }
      }
      else
      {
        hvON();
        _autoOff = false;
      }
    }
  }

  void setBackLight()
  {
    _displayHandler.setLED(0, 255, 0, 0);
    _displayHandler.setLED(1, 0, 255, 0);
    _displayHandler.setLED(2, 0, 0, 255);
    _displayHandler.setLED(3, 255, 254, 145);
    _displayHandler.setLED(4, 234, 63, 247);
    _displayHandler.setLED(5, 117, 249, 77);
    _displayHandler.setLED(6, 127, 130, 187);
    _displayHandler.setLED(7, 103, 14, 101);
    _displayHandler.setLED(8, 53, 167, 200);
    _displayHandler.setLED(9, 115, 251, 253);
    _displayHandler.setLED(10, 142, 64, 58);
    _displayHandler.setLED(11, 240, 134, 80);
    _displayHandler.setLED(12, 129, 127, 38);
    _displayHandler.setLED(13, 53, 128, 187);
    _displayHandler.updateLEDs();
  }
};
