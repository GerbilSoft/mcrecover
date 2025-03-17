/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * VarModifierDef.hpp: Variable modifier definition class.                 *
 *                                                                         *
 * Copyright (c) 2013-2025 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#pragma once

// C includes.
#include <stdint.h>

class VarModifierDef
{
public:
	enum class UseAs : uint8_t {
		/**
		 * Use as a filename component. [default]
		 * All variables are usable as a filename component,
		 * so this essentially disables other uses.
		 */
		Filename = 0,

		/** Timestamp components **/
		TS_Year,
		TS_Month,
		TS_Day,
		TS_Hour,
		TS_Minute,
		TS_Second,
		TS_AMPM,

		Max
	};
	// Use As [default is USEAS_FILENAME]
	UseAs useAs;

	enum class VarType : uint8_t {
		/**
		 * String: Use as-is. [default]
		 * "add" tag is not allowed.
		 */
		String = 0,

		/**
		 * Number: Interpret as a number.
		 * "add" tag can add/subtract numeric value.
		 */
		Number,

		/**
		 * Char: Right-most character is interpreted as ASCII.
		 * "add" tag can add/subtract ASCII value.
		 */
		Char,

		Max
	};
	// Variable type [default is VARTYPE_STRING]
	VarType varType;

	// Minimum field width [default is 0]
	uint8_t minWidth;

	/**
	 * Fill character [default is ' ']
	 * Must be ASCII: U+0000 - U+007F
	 */
	char fillChar;

	enum class FieldAlign : uint8_t {
		Right = 0,
		Left,

		Max
	};
	// Field alignment [default is FIELDALIGN_RIGHT]
	FieldAlign fieldAlign;

	/**
	 * Add value [default is 0]
	 * - VarType::Number: Adds the specified value to the number.
	 * - VarType::Char: Adds the specified value to the ASCII character value.
	 *                  (e.g. +1 turns 'A' into 'B')
	 */
	int addValue;

public:
	// Make sure all fields are initialized.
	VarModifierDef()
		: useAs(UseAs::Filename)
		, varType(VarType::String)
		, minWidth(0)
		, fillChar(' ')
		, fieldAlign(FieldAlign::Right)
		, addValue(0)
	{}
};
