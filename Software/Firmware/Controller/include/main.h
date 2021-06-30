// main.h

// Nixie Calculator Firmware
// Version 0.1.1 (alpha) - June 30, 2021

// Copyright (C) 2021 highvoltglow
// Licensed under the MIT License

#pragma once

//- INCLUDES
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Neopixel.h> // https://github.com/adafruit/Adafruit_NeoPixel
#include <SoftwareSerial.h>    // https://github.com/plerup/espsoftwareserial
#include <OneWire.h>
#include <DallasTemperature.h>
#include <nvs_flash.h>
#include <DisplayHandler.h>
#include <KeyboardHandler.h>
#include <Settings.h>
#include <Calculator.h>
#include <Clock.h>

//- VERSION
#define MAJOR_VERSION 0
#define MINOR_VERSION 1
#define REVISION 1

//- DEFINITIONS

// pins
#define PIN_HVENABLE 4
#define PIN_HVLED 5
#define PIN_SHIFT 17
#define PIN_STORE 16
#define PIN_DATA 18
#define PIN_LEDCTL 14
#define PIN_GPSTX 33
#define PIN_GPSRX 32
#define PIN_PIR 35
#define PIN_KINT 27
#define PIN_TEMPERATURE 25

// shift transition
#define SHIFT_BEGIN HIGH
#define SHIFT_COMMIT LOW

// store transition
#define STORE_BEGIN LOW
#define STORE_COMMIT HIGH

// digits
#define DIGIT_COUNT 14
#define DIGIT_OFF 255

// neons
#define DECIMAL_POINT_COUNT 14

// leds
#define LED_COUNT 6

//- ENUMS
enum class sign : uint8_t
{
  plus,
  minus
};

enum class device_mode : uint8_t
{
  calculator,
  clock,
  menu
};

//- GLOBALS
Adafruit_NeoPixel leds(LED_COUNT, PIN_LEDCTL, NEO_GRB + NEO_KHZ800);
Settings *pSettings;
DisplayHandler displayHandler(DIGIT_COUNT, DECIMAL_POINT_COUNT, display_mode::in16, PIN_DATA, PIN_STORE, PIN_SHIFT);
KeyboardHandler keyboard;
SoftwareSerial keyboardCom(PIN_KINT);
Calculator calculator(DIGIT_COUNT);

uint32_t pirTimestamp = 0;
uint32_t pirInterval = 5 * 60; // 5 minutes
uint32_t gpsSyncTimestamp = 0;
uint32_t gpsSyncInterval = GPS_SYNC_INTERVAL_SHORT; // 15 seconds
uint32_t syncTimestamp = -5000;
bool highVoltageOn = false;

// calculator stuff
bool decimalMode = false;
bool numericMode = false;
sign currentNumberSign = sign::plus;
double currentNumber = 0.0;
String display;
device_mode deviceMode = device_mode::calculator;

// Temperature sensor
OneWire oneWire(PIN_TEMPERATURE);
DallasTemperature tempSensor(&oneWire);
DeviceAddress tempSensorAddress;
bool tempSensorAvailable = false;
volatile unsigned long temperatureCheckTimestamp;
unsigned long temperatureCheckInterval = 5;
unsigned long temperatureConversionDelay = 1;
float currentTemperature;

// declarations
void hvON();
void hvOFF();
bool getTempSensor();
void checkTemperature();
void showVersion();

// callback functions
void onKeyboardEvent(uint8_t keyCode, key_state keyState, bool functionKeyPressed);
