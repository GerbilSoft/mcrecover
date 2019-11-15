/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * VarModifierDef.hpp: Variable modifier definition class.                 *
 *                                                                         *
 * Copyright (c) 2013 by David Korth.                                      *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#ifndef __MCRECOVER_VARIABLEDEF_HPP__
#define __MCRECOVER_VARIABLEDEF_HPP__

// C includes.
#include <stdint.h>

class VarModifierDef
{
	public:
		enum UseAs_t {
			/**
			 * Use as a filename component. [default]
			 * All variables are usable as a filename component,
			 * so this essentially disables other uses.
			 */
			USEAS_FILENAME	= 0,

			/** Timestamp components. **/

			USEAS_TS_YEAR,
			USEAS_TS_MONTH,
			USEAS_TS_DAY,
			USEAS_TS_HOUR,
			USEAS_TS_MINUTE,
			USEAS_TS_SECOND,
			USEAS_TS_AMPM,

			USEAS_TS_MAX
		};

		// Use As. [default is USEAS_FILENAME]
		uint8_t useAs;

		enum VarType_t {
			/**
			 * String: Use as-is. [default]
			 * "add" tag is not allowed.
			 */
			VARTYPE_STRING	= 0,

			/**
			 * Number: Interpret as a number.
			 * "add" tag can add/subtract numeric value.
			 */
			VARTYPE_NUMBER,

			/**
			 * Char: Right-most character is interpreted as ASCII.
			 * "add" tag can add/subtract ASCII value.
			 */
			VARTYPE_CHAR,

			VARTYPE_MAX
		};

		// Variable type. [default is VARTYPE_STRING]
		uint8_t varType;

		// Minimum field width. [default is 0]
		uint8_t minWidth;

		/**
		 * Fill character. [default is ' ']
		 * Must be ASCII: U+0000 - U+007F
		 */
		char fillChar;

		enum FieldAlign_t {
			FIELDALIGN_RIGHT = 0,
			FIELDALIGN_LEFT,

			FIELDALIGN_MAX
		};

		// Field alignment. [default is FIELDALIGN_RIGHT]
		uint8_t fieldAlign;

		/**
		 * Add value. [default is 0]
		 * - VARTYPE_NUMBER: Adds the specified value to the number.
		 * - VARTYPE_CHAR: Adds the specified value to the ASCII character value.
		 *                 (e.g. +1 turns 'A' into 'B')
		 */
		int addValue;

	public:
		// Make sure all fields are initialized.
		VarModifierDef()
		{
			useAs = USEAS_FILENAME;
			varType = VARTYPE_STRING;
			minWidth = 0;
			fillChar = ' ';
			fieldAlign = FIELDALIGN_RIGHT;
			addValue = 0;
		}
};

#endif /* __MCRECOVER_VARIABLEDEF_HPP__ */
