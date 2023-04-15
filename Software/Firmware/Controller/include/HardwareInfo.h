// HardwareInfo.h

// Provides information about the hardware

// Copyright (C) 2023 highvoltglow
// Licensed under the MIT License

#pragma once

enum class display_type
{
  undefined,
  in12,
  in16,
  in17,
  b5870
};

//==========================================
// set here your display type
#define DISPLAY_TYPE display_type::undefined
//==========================================

// generates compile time error if display type not set
static_assert(DISPLAY_TYPE == display_type::in12 ||
                  DISPLAY_TYPE == display_type::in16 ||
                  DISPLAY_TYPE == display_type::in17 ||
                  DISPLAY_TYPE == display_type::b5870,
              "Invalid display type!");
