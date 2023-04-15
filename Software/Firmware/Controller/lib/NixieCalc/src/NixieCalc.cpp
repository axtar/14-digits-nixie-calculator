// NixieCalc.cpp

// provides the calculator functions
// uses double type, 64-bit on ESP32

// Copyright (C) 2023 highvoltglow
// Licensed under the MIT License

#include "NixieCalc.h"

// constructor
NixieCalc::NixieCalc() : _angleMode(angle_mode::deg)
{
  onAllClear();
}

// returns the operation return code: success or an error code
operation_return_code NixieCalc::getOperationReturnCode()
{
  return (_operationReturnCode);
}

// returns the current result or input value
double NixieCalc::getDisplayValue()
{
  return (_displayValue);
}

// returns angle mode, deg or rad
angle_mode NixieCalc::getAngleMode()
{
  return (_angleMode);
}

// sets the angle mode for trigonometric operations, deg (default) or rad
void NixieCalc::setAngleMode(angle_mode angleMode)
{
  _angleMode = angleMode;
}

// call to enter a numeric value
void NixieCalc::onNumericInput(double value)
{
  // accept numeric input only if no previous error
  //  use allclear operation to reset error
  if (_operationReturnCode == operation_return_code::success)
  {
    if (_equalsEntered)
    {
      // numeric input after equals, clean up
      _equalsEntered = false;
      _operation = operation::none;
      _leftValue = 0.0;
      _rightValue = 0.0;
    }
    _displayValue = value;
    _numberEntered = true;
  }
}

// call to enter an operation
void NixieCalc::onOperation(operation op)
{
  // accept operation only if no previous error,
  //  use allclear operation to reset error
  if (_operationReturnCode == operation_return_code::success)
  {
    switch (op)
    {
    // dual value operation
    case operation::addition:
    case operation::subtraction:
    case operation::multiplication:
    case operation::division:
    case operation::pow:
      onDualValueOperation(op);
      break;

    // single value operation
    case operation::squareroot:
    case operation::inv:
    case operation::sin:
    case operation::cos:
    case operation::tan:
    case operation::log:
    case operation::ln:
    case operation::switchsign:
    case operation::factorial:
      onSingleValueOperation(op);
      break;

    // memory operations
    case operation::memclear:
    case operation::memread:
    case operation::memstore:
    case operation::memsubtraction:
    case operation::memaddition:
      onMemoryOperation(op);
      break;

    case operation::euler:
    case operation::pi:
      onConstantOperation(op);
      break;

    // clear operations
    case operation::allclear:
    case operation::clear:
      onClearOperation(op);
      break;

    // special operations
    case operation::percent:
      onPercentOperation(op);
      break;

    // equals
    case operation::equals:
      onEqualsOperation(op);
      break;

    default:
      break;
    }
  }
  else if (op == operation::allclear)
  {
    // always accept allclear operation
    onClearOperation(op);
  }
}

// performs operation with 2 values
void NixieCalc::onDualValueOperation(operation op)
{
  if (_operation == operation::none)
  {
    // no operation entered yet,
    _leftValue = _displayValue;
    _operation = op;
  }
  else if (!_numberEntered)
  {
    if (_equalsEntered)
    {
      // new operation after equals
      _leftValue = _displayValue;
    }
    // no new number entered, just change operation
    _operation = op;
  }
  else
  {
    // do calculation
    double result;
    _operationReturnCode = calculateValue(&result, _operation, _leftValue, _displayValue);
    _displayValue = result;
    _leftValue = result;
    _operation = op;
  }
  _numberEntered = false;
  _equalsEntered = false;
}

// performs an operation with a single value
void NixieCalc::onSingleValueOperation(operation op)
{
  if (_operation == operation::none)
  {
    double result;
    _operationReturnCode = calculateValue(&result, op, _displayValue);
    _leftValue = result;
    _displayValue = result;
  }
  else
  {
    double result;
    _operationReturnCode = calculateValue(&result, op, _displayValue);
    _displayValue = result;
  }
}

// equals operation,
void NixieCalc::onEqualsOperation(operation op)
{
  if (_operation != operation::none)
  {
    // we have an operation
    if (!_equalsEntered)
    {
      // first equals
      double result;
      _operationReturnCode = calculateValue(&result, _operation, _leftValue, _displayValue);
      _equalsEntered = true;
      // store value for repeating operations
      _rightValue = _displayValue;
      _displayValue = result;
    }
    else
    {
      // equals after equals, repeat previous operation
      double result;
      _operationReturnCode = calculateValue(&result, _operation, _displayValue, _rightValue);
      _leftValue = _displayValue;
      _displayValue = result;
    }
    _numberEntered = false;
  }
}

// performs memory operations
void NixieCalc::onMemoryOperation(operation op)
{
  switch (op)
  {
  case operation::memclear:
    _memoryValue = 0.0;
    break;

  case operation::memread:
    // as entered by user
    onNumericInput(_memoryValue);
    break;

  case operation::memstore:
    _memoryValue = _displayValue;
    break;

  case operation::memaddition:
    _memoryValue = _memoryValue + _displayValue;
    break;

  case operation::memsubtraction:
    _memoryValue = _memoryValue - _displayValue;
    break;

  default: // avoid warnings
    break;
  }
}

void NixieCalc::onConstantOperation(operation op)
{
  switch (op)
  {
  case operation::euler:
    onNumericInput(EULER);
    break;

  case operation::pi:
    onNumericInput(PI);
    break;
  }
}

// clear operations
void NixieCalc::onClearOperation(operation op)
{
  switch (op)
  {
  case operation::allclear:
    onAllClear();
    break;

  case operation::clear:
    _displayValue = 0.0;
    break;

  default: // avoid warnings
    break;
  }
}

// separate function for percent, percent has a special behavior
void NixieCalc::onPercentOperation(operation op)
{
  if ((_operation == operation::none) || _equalsEntered)
  {
    // no previous operation, just divide by 100
    _displayValue = _displayValue / 100;
    _leftValue = _displayValue;
    _numberEntered = false;
  }
  else
  {
    switch (_operation)
    {
    // if previous operation
    case operation::addition:
    case operation::subtraction:
      _displayValue = _leftValue * _displayValue / 100;
      break;

    case operation::multiplication:
    case operation::division:
      _displayValue = _displayValue / 100;
      break;

    default: // avoid warnings
      break;
    }
  }
}

// clear all values except memory
void NixieCalc::onAllClear()
{
  _leftValue = 0.0;
  _rightValue = 0.0;
  _displayValue = 0.0;
  _operationReturnCode = operation_return_code::success;
  _operation = operation::none;
  _numberEntered = false;
  _equalsEntered = false;
}

// does the math and catches some basic errors
operation_return_code NixieCalc::calculateValue(double *result, operation op, double leftValue, double rightValue)
{
  operation_return_code retVal = operation_return_code::success;
  *result = 0.0;
  double absolute = 0.0;

  switch (op)
  {
  case operation::addition:
    *result = leftValue + rightValue;
    break;

  case operation::subtraction:
    *result = leftValue - rightValue;
    break;

  case operation::multiplication:
    *result = leftValue * rightValue;
    break;

  case operation::division:
    if (rightValue != 0)
    {
      *result = leftValue / rightValue;
    }
    else
    {
      retVal = operation_return_code::divideByZero;
    }
    break;

  case operation::squareroot:
    if (leftValue >= 0)
    {
      *result = sqrt(leftValue);
    }
    else
    {
      retVal = operation_return_code::domain;
    }
    break;

  case operation::pow:
    *result = pow(leftValue, rightValue);
    break;

  case operation::inv:
    if (leftValue != 0)
    {
      *result = 1 / leftValue;
    }
    else
    {
      retVal = operation_return_code::divideByZero;
    }
    break;

  case operation::sin:
    switch (_angleMode)
    {
    case angle_mode::deg:
      *result = sin(leftValue * PI / 180.0);
      break;

    case angle_mode::rad:
      *result = sin(leftValue);
      break;
    }
    break;

  case operation::cos:
    switch (_angleMode)
    {
    case angle_mode::deg:
      *result = cos(leftValue * PI / 180.0);
      break;

    case angle_mode::rad:
      *result = cos(leftValue);
      break;
    }
    break;

  case operation::tan:
    switch (_angleMode)
    {
    case angle_mode::deg:
      if (abs(remainder(leftValue, 360.0)) == 90)
      {
        retVal = operation_return_code::domain;
      }
      else
      {
        *result = tan(leftValue * PI / 180.0);
      }
      break;

    case angle_mode::rad:
      if (cos(leftValue) != 0.0)
      {
        *result = tan(leftValue);
      }
      else
      {
        retVal = operation_return_code::domain;
      }
      break;
    }
    break;

  case operation::log:
    if (leftValue > 0)
    {
      *result = log10(leftValue);
    }
    else
    {
      retVal = operation_return_code::domain;
    }
    break;

  case operation::ln:
    if (leftValue > 0)
    {
      *result = log(leftValue);
    }
    else
    {
      retVal = operation_return_code::domain;
    }
    break;

  case operation::switchsign:
    *result = leftValue * (-1);
    break;

  case operation::factorial:
    if (leftValue > MAX_FACT)
    {
      retVal = operation_return_code::overflow;
    }
    else
    {
      absolute = abs(leftValue);
      if (absolute == floor(absolute) && (leftValue >= 0))
      {
        *result = 1;
        for (int i = 1; i <= absolute; i++)
        {
          *result *= i;
        }
      }
      else
      {
        retVal = operation_return_code::domain;
      }
    }
    break;

  default:
    retVal = operation_return_code::unknownoperation;
    break;
  }
  if (retVal == operation_return_code::success)
  {
    // check for result overflow if no other error was produced
    if ((*result > MAX_CALC_VALUE) || (*result < MIN_CALC_VALUE))
    {
      retVal = operation_return_code::overflow;
    }
  }
  // reset result value if an error courred
  if (retVal != operation_return_code::success)
  {
    *result = 0.0;
  }
  return (retVal);
}
