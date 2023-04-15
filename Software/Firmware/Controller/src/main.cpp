// main.cpp

// Nixie Calculator Firmware

// Copyright (C) 2023 highvoltglow
// Licensed under the MIT License

//- INCLUDES
#include <main.h>
#include <Errors.h>

// setup
void setup()
{

  Serial.begin(115200);

  // init controller
  int err = controller.begin();

  if (err != ERR_SUCCESS)
  {
    Serial.println(Errors::getErrorText(err));
    while (true)
      ;
  }
}

// main loop
void loop()
{
  controller.process();

  // relax
  delay(10);
}
