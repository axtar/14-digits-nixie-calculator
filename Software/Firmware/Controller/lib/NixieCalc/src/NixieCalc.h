// NixieCalc.h

// provides the calculator functions
// uses double type, 64-bit on ESP32

// Copyright (C) 2023 highvoltglow
// Licensed under the MIT License

#pragma once

#include <Arduino.h>
#include <math.h>

// MIN/MAX VALUES, defined for 14-digit calculator
#define MAX_CALC_VALUE 99999999999999
#define MIN_CALC_VALUE -99999999999999
#define MAX_FACT 20

// enums
enum class operation : uint8_t
{
  none,
  addition,
  subtraction,
  division,
  multiplication,
  squareroot,
  percent,
  equals,
  memclear,
  memread,
  memstore,
  memsubtraction,
  memaddition,
  allclear,
  clear,
  switchsign,
  inv,
  pow,
  sin,
  cos,
  tan,
  log,
  ln,
  factorial,
  euler,
  pi
};

enum class operation_return_code : uint8_t
{
  success,
  overflow,
  divideByZero,
  domain,
  unknownoperation
};

enum class angle_mode : uint8_t
{
  deg,
  rad
};

// calculator engine class
class NixieCalc
{
public:
  NixieCalc();

  // user input
  void onOperation(operation op);
  void onNumericInput(double value);

  // get return code of math operation
  operation_return_code getOperationReturnCode();

  // get input/operation result
  double getDisplayValue();

  // get/set angle mode
  angle_mode getAngleMode();
  void setAngleMode(angle_mode angleMode);

private:
  // numeric registers
  double _displayValue;
  double _leftValue;
  double _rightValue;
  double _memoryValue;

  // return code of math operations
  operation_return_code _operationReturnCode;

  // angle mode
  angle_mode _angleMode;

  // state variables
  bool _numberEntered;
  bool _equalsEntered;

  // current operation
  operation _operation;

  // functions for operation types
  void onDualValueOperation(operation op);
  void onSingleValueOperation(operation op);
  void onMemoryOperation(operation op);
  void onConstantOperation(operation op);
  void onClearOperation(operation op);
  void onPercentOperation(operation op);
  void onEqualsOperation(operation op);

  // clean up
  void onAllClear();

  // math operations
  operation_return_code calculateValue(double *result, operation op, double leftValue, double rightValue = 0.0);
};
