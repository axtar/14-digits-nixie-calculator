// main.cpp

// Nixie Calculator Firmware
// Version 0.1.1 (alpha) - June 30, 2021

// Copyright (C) 2021 highvoltglow
// Licensed under the MIT License

//- INCLUDES
#include <main.h>

// setup
void setup()
{
  Serial.begin(115200);

  // set pin modes
  pinMode(PIN_HVENABLE, OUTPUT);
  pinMode(PIN_HVLED, OUTPUT);
  pinMode(PIN_LEDCTL, OUTPUT);
  pinMode(PIN_PIR, INPUT);

  // make sure HV is off
  hvOFF();

  // init I2C
  Wire.begin();

  // init keyboard stuff
  keyboardCom.begin(9600);
  keyboard.begin(keyboardCom);
  keyboard.attach(onKeyboardEvent);

  displayHandler.clearDisplay();

  //nvs_flash_init();
  //pSettings = new Settings();

  // pir interrupt

  // init LEDs
  leds.begin();
  leds.clear();
  leds.show();
  Serial.println();

  // wait for keyboard start
  delay(1000);

  // requests version from keyboard
  keyboard.requestVersion();
  keyboard.setAutoRepeatInterval(500);

  // temperature sensor
  tempSensor.begin();
  tempSensorAvailable = getTempSensor();
  currentTemperature = 0;
  if (tempSensorAvailable)
  {
    tempSensor.setResolution(10);
    tempSensor.setWaitForConversion(false);
    temperatureCheckTimestamp = millis() + (temperatureCheckInterval * 1000);
  }

  // turn on high voltage
  hvON();

  // show version
  showVersion();
  delay(750);

  //
  switch (deviceMode)
  {
  case device_mode::calculator:
    displayHandler.Show(calculator.getDisplay());
    break;

  default:
    break;
  }
}

// main loop
void loop()
{
  keyboard.process();
  if (tempSensorAvailable)
  {
    checkTemperature();
  }
}

void onKeyboardEvent(uint8_t keyCode, key_state keyState, bool functionKeyPressed)
{
  switch (deviceMode)
  {
  case device_mode::calculator:
    // calculator is keyboard driven, send key event and update display
    calculator.onKeyboardEvent(keyCode, keyState, functionKeyPressed);
    displayHandler.Show(calculator.getDisplay());
    break;

  default:
    break;
  }
}

void showVersion()
{
  char buffer[20];
  sprintf(buffer, "%02u.%02u.%02u  %02u.%02u.%02u", MAJOR_VERSION, MINOR_VERSION, REVISION,
          keyboard.getMajorVersion(), keyboard.getMinorVersion(), keyboard.getRevision());
  displayHandler.Show(buffer);
}

void hvON()
{
  if (!highVoltageOn)
  {
    highVoltageOn = true;
    digitalWrite(PIN_HVENABLE, HIGH);
    digitalWrite(PIN_HVLED, HIGH);
  }
}

void hvOFF()
{
  if (highVoltageOn)
  {
    highVoltageOn = false;
    digitalWrite(PIN_HVENABLE, LOW);
    digitalWrite(PIN_HVLED, LOW);
  }
}

bool getTempSensor()
{
  bool result = false;
  Serial.println(tempSensor.getDeviceCount());
  for (int i = 0; i < tempSensor.getDeviceCount(); i++)
  {
    tempSensor.getAddress(tempSensorAddress, i);
    result = true;
  }
  return result;
}

void checkTemperature()
{
  static bool requestPending = false;
  unsigned long currentMillis = millis();
  if (!requestPending)
  {
    if (currentMillis - temperatureCheckTimestamp > (temperatureCheckInterval * 1000))
    {
      tempSensor.requestTemperatures();
      temperatureCheckTimestamp = currentMillis;
      requestPending = true;
    }
  }
  else
  {
    if (currentMillis - temperatureCheckTimestamp > (temperatureConversionDelay * 1000))
    {
      currentTemperature = tempSensor.getTempC(tempSensorAddress);
      requestPending = false;
      Serial.println(currentTemperature);
    }
  }
}