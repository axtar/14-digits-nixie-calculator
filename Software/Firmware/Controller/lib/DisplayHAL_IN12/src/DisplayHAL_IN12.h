// displayHAL_IN12.h

// implements the hardware abstraction layer interface
// for the IN-12 board (socket version)

// Copyright (C) 2023 highvoltglow
// Licensed under the MIT License

#pragma once

#include <Arduino.h>
#include <displayHAL.h>

#define IN12_DIGITCOUNT 14
#define IN12_DECIMALPOINTCOUNT 14
#define IN12_LEDCOUNT 15

class DisplayHAL_IN12 : public DisplayHAL
{
public:
	DisplayHAL_IN12()
	{
		initTranslationTable();
	}

	virtual ~DisplayHAL_IN12()
	{
		free(_translationTable);
	}

	uint8_t getDigitCount()
	{
		return (IN12_DIGITCOUNT);
	}

	uint8_t getDecimalPointCount()
	{
		return (IN12_DECIMALPOINTCOUNT);
	}

	uint8_t getLEDCount()
	{
		return (IN12_LEDCOUNT);
	}

	bool hasPlusSign()
	{
		return (true);
	}

	bool hasMenuSign()
	{
		return (true);
	}

	bool hasLedPerDigit()
	{
		return (true);
	}

	led_type getLedType()
	{
		return (led_type::smd);
	}

	// returns information about what is connected to a specific shift register output
	register_type getRegisterInfo(uint8_t registerNumber, uint8_t *digit, uint8_t *number)
	{
		*digit = 0;
		*number = 0;

		register_type regType;

		if ((registerNumber < 1) || (registerNumber > REGISTER_COUNT))
		{
			regType = register_type::unknown;
		}
		else
		{
			regType = _translationTable[registerNumber - 1].rt;
			*digit = _translationTable[registerNumber - 1].digit;
			*number = _translationTable[registerNumber - 1].number;
		}
		return (regType);
	}

private:
	// translation table
	// Nixie   Digit
	// -----   -----
	// N1  --> 14
	// N2  --> 0
	// N3  --> 1
	// N4  --> 2
	// N5  --> 3
	// N6  --> 4
	// N7  --> 5
	// N8  --> 6
	// N9  --> 7
	// N10 --> 8
	// N11 --> 9
	// N12 --> 10
	// N13 --> 11
	// N14 --> 12
	// N15 --> 13

	void initTranslationTable()
	{
		_translationTable = (TRANSLATION_TABLE_ENTRY *)malloc(sizeof(TRANSLATION_TABLE_ENTRY) * REGISTER_COUNT);
		_translationTable[34] = {register_type::number, 0, 0};
		_translationTable[33] = {register_type::number, 0, 1};
		_translationTable[32] = {register_type::number, 0, 2};
		_translationTable[25] = {register_type::number, 0, 3};
		_translationTable[24] = {register_type::number, 0, 4};
		_translationTable[23] = {register_type::number, 0, 5};
		_translationTable[22] = {register_type::number, 0, 6};
		_translationTable[21] = {register_type::number, 0, 7};
		_translationTable[20] = {register_type::number, 0, 8};
		_translationTable[35] = {register_type::number, 0, 9};
		_translationTable[36] = {register_type::decimal_point, 0, 0};
		_translationTable[39] = {register_type::number, 1, 0};
		_translationTable[38] = {register_type::number, 1, 1};
		_translationTable[37] = {register_type::number, 1, 2};
		_translationTable[19] = {register_type::number, 1, 3};
		_translationTable[18] = {register_type::number, 1, 4};
		_translationTable[17] = {register_type::number, 1, 5};
		_translationTable[16] = {register_type::number, 1, 6};
		_translationTable[15] = {register_type::number, 1, 7};
		_translationTable[14] = {register_type::number, 1, 8};
		_translationTable[40] = {register_type::number, 1, 9};
		_translationTable[41] = {register_type::decimal_point, 1, 0};
		_translationTable[44] = {register_type::number, 2, 0};
		_translationTable[43] = {register_type::number, 2, 1};
		_translationTable[42] = {register_type::number, 2, 2};
		_translationTable[13] = {register_type::number, 2, 3};
		_translationTable[12] = {register_type::number, 2, 4};
		_translationTable[11] = {register_type::number, 2, 5};
		_translationTable[10] = {register_type::number, 2, 6};
		_translationTable[9] = {register_type::number, 2, 7};
		_translationTable[8] = {register_type::number, 2, 8};
		_translationTable[45] = {register_type::number, 2, 9};
		_translationTable[46] = {register_type::decimal_point, 2, 0};
		_translationTable[49] = {register_type::number, 3, 0};
		_translationTable[48] = {register_type::number, 3, 1};
		_translationTable[47] = {register_type::number, 3, 2};
		_translationTable[7] = {register_type::number, 3, 3};
		_translationTable[6] = {register_type::number, 3, 4};
		_translationTable[5] = {register_type::number, 3, 5};
		_translationTable[4] = {register_type::number, 3, 6};
		_translationTable[3] = {register_type::number, 3, 7};
		_translationTable[2] = {register_type::number, 3, 8};
		_translationTable[50] = {register_type::number, 3, 9};
		_translationTable[51] = {register_type::decimal_point, 3, 0};
		_translationTable[54] = {register_type::number, 4, 0};
		_translationTable[53] = {register_type::number, 4, 1};
		_translationTable[52] = {register_type::number, 4, 2};
		_translationTable[1] = {register_type::number, 4, 3};
		_translationTable[0] = {register_type::number, 4, 4};
		_translationTable[95] = {register_type::number, 4, 5};
		_translationTable[94] = {register_type::number, 4, 6};
		_translationTable[93] = {register_type::number, 4, 7};
		_translationTable[92] = {register_type::number, 4, 8};
		_translationTable[55] = {register_type::number, 4, 9};
		_translationTable[56] = {register_type::decimal_point, 4, 0};
		_translationTable[59] = {register_type::number, 5, 0};
		_translationTable[58] = {register_type::number, 5, 1};
		_translationTable[57] = {register_type::number, 5, 2};
		_translationTable[91] = {register_type::number, 5, 3};
		_translationTable[90] = {register_type::number, 5, 4};
		_translationTable[89] = {register_type::number, 5, 5};
		_translationTable[88] = {register_type::number, 5, 6};
		_translationTable[87] = {register_type::number, 5, 7};
		_translationTable[86] = {register_type::number, 5, 8};
		_translationTable[60] = {register_type::number, 5, 9};
		_translationTable[61] = {register_type::decimal_point, 5, 0};
		_translationTable[96] = {register_type::number, 6, 0};
		_translationTable[63] = {register_type::number, 6, 1};
		_translationTable[62] = {register_type::number, 6, 2};
		_translationTable[81] = {register_type::number, 6, 3};
		_translationTable[82] = {register_type::number, 6, 4};
		_translationTable[83] = {register_type::number, 6, 5};
		_translationTable[84] = {register_type::number, 6, 6};
		_translationTable[85] = {register_type::number, 6, 7};
		_translationTable[80] = {register_type::number, 6, 8};
		_translationTable[97] = {register_type::number, 6, 9};
		_translationTable[98] = {register_type::decimal_point, 6, 0};
		_translationTable[101] = {register_type::number, 7, 0};
		_translationTable[100] = {register_type::number, 7, 1};
		_translationTable[99] = {register_type::number, 7, 2};
		_translationTable[79] = {register_type::number, 7, 3};
		_translationTable[78] = {register_type::number, 7, 4};
		_translationTable[77] = {register_type::number, 7, 5};
		_translationTable[76] = {register_type::number, 7, 6};
		_translationTable[75] = {register_type::number, 7, 7};
		_translationTable[74] = {register_type::number, 7, 8};
		_translationTable[102] = {register_type::number, 7, 9};
		_translationTable[103] = {register_type::decimal_point, 7, 0};
		_translationTable[106] = {register_type::number, 8, 0};
		_translationTable[105] = {register_type::number, 8, 1};
		_translationTable[104] = {register_type::number, 8, 2};
		_translationTable[73] = {register_type::number, 8, 3};
		_translationTable[72] = {register_type::number, 8, 4};
		_translationTable[71] = {register_type::number, 8, 5};
		_translationTable[70] = {register_type::number, 8, 6};
		_translationTable[69] = {register_type::number, 8, 7};
		_translationTable[68] = {register_type::number, 8, 8};
		_translationTable[107] = {register_type::number, 8, 9};
		_translationTable[108] = {register_type::decimal_point, 8, 0};
		_translationTable[111] = {register_type::number, 9, 0};
		_translationTable[110] = {register_type::number, 9, 1};
		_translationTable[109] = {register_type::number, 9, 2};
		_translationTable[67] = {register_type::number, 9, 3};
		_translationTable[66] = {register_type::number, 9, 4};
		_translationTable[65] = {register_type::number, 9, 5};
		_translationTable[64] = {register_type::number, 9, 6};
		_translationTable[159] = {register_type::number, 9, 7};
		_translationTable[158] = {register_type::number, 9, 8};
		_translationTable[112] = {register_type::number, 9, 9};
		_translationTable[113] = {register_type::decimal_point, 9, 0};
		_translationTable[116] = {register_type::number, 10, 0};
		_translationTable[115] = {register_type::number, 10, 1};
		_translationTable[114] = {register_type::number, 10, 2};
		_translationTable[157] = {register_type::number, 10, 3};
		_translationTable[156] = {register_type::number, 10, 4};
		_translationTable[155] = {register_type::number, 10, 5};
		_translationTable[154] = {register_type::number, 10, 6};
		_translationTable[153] = {register_type::number, 10, 7};
		_translationTable[152] = {register_type::number, 10, 8};
		_translationTable[117] = {register_type::number, 10, 9};
		_translationTable[118] = {register_type::decimal_point, 10, 0};
		_translationTable[121] = {register_type::number, 11, 0};
		_translationTable[120] = {register_type::number, 11, 1};
		_translationTable[119] = {register_type::number, 11, 2};
		_translationTable[151] = {register_type::number, 11, 3};
		_translationTable[150] = {register_type::number, 11, 4};
		_translationTable[149] = {register_type::number, 11, 5};
		_translationTable[148] = {register_type::number, 11, 6};
		_translationTable[147] = {register_type::number, 11, 7};
		_translationTable[146] = {register_type::number, 11, 8};
		_translationTable[122] = {register_type::number, 11, 9};
		_translationTable[123] = {register_type::decimal_point, 11, 0};
		_translationTable[126] = {register_type::number, 12, 0};
		_translationTable[125] = {register_type::number, 12, 1};
		_translationTable[124] = {register_type::number, 12, 2};
		_translationTable[145] = {register_type::number, 12, 3};
		_translationTable[144] = {register_type::number, 12, 4};
		_translationTable[143] = {register_type::number, 12, 5};
		_translationTable[142] = {register_type::number, 12, 6};
		_translationTable[141] = {register_type::number, 12, 7};
		_translationTable[140] = {register_type::number, 12, 8};
		_translationTable[127] = {register_type::number, 12, 9};
		_translationTable[128] = {register_type::decimal_point, 12, 0};
		_translationTable[131] = {register_type::number, 13, 0};
		_translationTable[130] = {register_type::number, 13, 1};
		_translationTable[129] = {register_type::number, 13, 2};
		_translationTable[139] = {register_type::number, 13, 3};
		_translationTable[138] = {register_type::number, 13, 4};
		_translationTable[137] = {register_type::number, 13, 5};
		_translationTable[136] = {register_type::number, 13, 6};
		_translationTable[135] = {register_type::number, 13, 7};
		_translationTable[134] = {register_type::number, 13, 8};
		_translationTable[132] = {register_type::number, 13, 9};
		_translationTable[133] = {register_type::decimal_point, 13, 0};
		_translationTable[27] = {register_type::minus_sign, 14, 0}; // -
		_translationTable[28] = {register_type::plus_sign, 14, 0};	// +
		_translationTable[29] = {register_type::menu_sign, 14, 0};	// M
		_translationTable[31] = {register_type::not_used, 14, 0};		// %
		_translationTable[30] = {register_type::not_used, 14, 0};		// N
		_translationTable[26] = {register_type::not_used, 14, 0};		// .
	}

private:
	TRANSLATION_TABLE_ENTRY *_translationTable;
};
