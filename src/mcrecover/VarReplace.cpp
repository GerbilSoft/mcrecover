/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * VarReplace.cpp: Variable replacement functions.                         *
 *                                                                         *
 * Copyright (c) 2013-2018 by David Korth.                                 *
 *                                                                         *
 * This program is free software; you can redistribute it and/or modify it *
 * under the terms of the GNU General Public License as published by the   *
 * Free Software Foundation; either version 2 of the License, or (at your  *
 * option) any later version.                                              *
 *                                                                         *
 * This program is distributed in the hope that it will be useful, but     *
 * WITHOUT ANY WARRANTY; without even the implied warranty of              *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU General Public License for more details.                            *
 *                                                                         *
 * You should have received a copy of the GNU General Public License       *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.   *
 ***************************************************************************/

#include "VarReplace.hpp"

class VarReplacePrivate
{
	private:
		VarReplacePrivate();
		~VarReplacePrivate();

	private:
		Q_DISABLE_COPY(VarReplacePrivate);

	public:
		static inline bool isValidVarNameChr(QChar chr)
		{
			return (chr.isLetterOrNumber() || chr == QChar(L'_'));
		}

		static inline bool isValidVarName(const QString &varName)
		{
			for (int i = (varName.size() - 1); i >= 0; i--) {
				if (!isValidVarNameChr(varName.at(i)))
					return false;
			}
			return true;
		}
};

/** VarReplace **/

/**
 * Replace variables in a given string.
 * @param str String to replace variables in.
 * @param vars QHash containing variables for replacement.
 *
 * QHash format:
 * - key: variable name
 * - value: variable value
 *
 * @return str with replaced variables.
 */
QString VarReplace::Exec(const QString &str, const QHash<QString, QString> &vars)
{
	// Variable format: $VAR, ${VAR}, $(VAR)
	QString workStr;
	workStr.reserve(str.size() * 3 / 2);

	// Valid variable name characters: [a-zA-Z_]
	bool inVar = false;	// True if we're currently processing a variable.
	QString varName;	// Current variable name.
	QChar varDelimStart;	// Variable delimiter, start char. (If 0, any non-variable character works.)
	QChar varDelimEnd;	// Variable delimiter, end char. (If 0, any non-variable character works.)

	// TODO: Add escape characters, e.g. "\$" and "\\".
	for (int i = 0; i < str.size(); i++) {
		QChar chr = str.at(i);
		if (!inVar) {
			// Not currently in a variable.
			// Check for a dollar sign.
			if (chr == QChar(L'$')) {
				// Found a dollar sign. Start processing the variable.
				inVar = true;
				varName.clear();
				varName.reserve(8);
				if (i + 1 >= str.length()) {
					// Not enough characters remaining.
					// TODO: Print error message?
					workStr += chr;
					break;
				}

				// Check if a delimiter is being used.
				QChar nextChr = str.at(i + 1);
				switch (nextChr.unicode()) {
					case L'(':
						// Parentheses-style delimiters.
						varDelimStart = QChar(L'(');
						varDelimEnd = QChar(L')');
						i++;
						break;
					case L'{':
						// Braces-style delimiters.
						varDelimStart = QChar(L'{');
						varDelimEnd = QChar(L'}');
						i++;
						break;
					default:
						// No delimiter is being used.
						varDelimStart = QChar();
						varDelimEnd = QChar();
						break;
				}
			} else {
				// Not a dollar sign. Add the character to the string.
				workStr += chr;
			}
		} else {
			// Currently in a variable.
			bool isVarFinished = false;
			bool isVarInvalid = false;
			QChar chr_extra; // Extra character if we have no delimiters.

			if (!varDelimEnd.isNull()) {
				// Variable delimiter is specified.
				// Check if we've reached it.
				if (chr == varDelimEnd) {
					// We've reached the variable delimiter.
					isVarFinished = true;
				} else {
					// Not the delimiter.
					varName += chr;
					if (i + 1 >= str.size()) {
						// End of string. This is an error.
						// TODO: Print error message?
						varDelimEnd = QChar();
						isVarInvalid = true;
					}
				}
			} else {
				// Variable delimiter is not specified.
				// Check if the character is a valid variable name character.
				if (VarReplacePrivate::isValidVarNameChr(chr)) {
					// Character is valid.
					varName += chr;
				} else {
					// Not a variable name character.
					chr_extra = chr;
					isVarFinished = true;
				}

				if (i + 1 >= str.length()) {
					// End of string, so end of variable name.
					isVarFinished = true;
				}
			}

			if (isVarFinished) {
				// Variable name is finished.
				if (varName.isEmpty()) {
					// Empty variable name.
					// TODO: Print a warning message.
					isVarInvalid = true;
				} else if (!VarReplacePrivate::isValidVarName(varName)) {
					// Variable name is invalid.
					// TODO: Print a warning message.
					isVarInvalid = true;
				} else {
					// Check if the variable is in the QHash.
					// NOTE: This requires two lookups.
					// Just get the value instead of checking contains()?
					if (!vars.contains(varName)) {
						// Variable is not in the QHash.
						// TODO: Print a warning message?
						isVarInvalid = true;
					} else {
						// Variable is in the QHash.
						workStr += vars.value(varName);
					}
				}

				// Clear the "in-var" state.
				inVar = false;
			}

			if (isVarInvalid) {
				// Variable is invalid.
				// Append the original variable name.
				workStr += QChar(L'$');
				if (!varDelimStart.isNull())
					workStr += varDelimStart;
				workStr += varName;
				if (!varDelimEnd.isNull())
					workStr += varDelimEnd;

				// Clear the "in-var" state.
				inVar = false;
			}

			// Append the extra character if we didn't have delimiters.
			if (!chr_extra.isNull())
				workStr += chr_extra;
		}
	}

	// Return the processed string.
	return workStr;
}

/**
 * Combine QStringLists of GameDesc variables and FileDesc variables into a QHash.
 * @param gameDescVars GameDesc variables.
 * @param fileDescVars FileDesc variables.
 *
 * NOTE: The first variable in each match ($G0, $F0) is
 * the full match from PCRE. This usually won't be used,
 * but is included in the variable hash anyway.
 *
 * @return QHash containing the variables. (key == ID)
 */
QHash<QString, QString> VarReplace::StringListsToHash(
	const QStringList &gameDescVars,
	const QStringList &fileDescVars)
{
	QHash<QString, QString> vars;
	QString varName;
	varName.reserve(4);

	for (int i = gameDescVars.size()-1; i >= 0; i--) {
		varName = QChar(L'G') + QString::number(i);
		vars.insert(varName, gameDescVars.at(i));
	}
	for (int i = fileDescVars.size()-1; i >= 0; i--) {
		varName = QChar(L'F') + QString::number(i);
		vars.insert(varName, fileDescVars.at(i));
	}

	return vars;
}

/**
 * Parse a string as an integer.
 * This function handles fullwidth numbers.
 * @param str String.
 * @return Integer.
 */
int VarReplace::strToInt(const QString &str)
{
	// TODO: Qt should have a way to do this itself...

	// Fullwidth/Halfwidth to Standard table.
	// Index is FW; value is standard.
	// NOTE: Characters with '0' are not supported here.
	static const uint16_t fwhwToStd[256] = {
		   0, L'!', L'"', L'#', L'$', L'%', L'&', L'\'',
		L'(', L')', L'*', L'+', L',', L'-', L'.', L'/',
		L'0', L'1', L'2', L'3', L'4', L'5', L'6', L'7',
		L'8', L'9', L':', L';', L'<', L'=', L'>', L'?',
		L'@', L'A', L'B', L'C', L'D', L'E', L'F', L'G',
		L'H', L'I', L'J', L'K', L'L', L'M', L'N', L'O',
		L'P', L'Q', L'R', L'S', L'T', L'U', L'V', L'W',
		L'X', L'Y', L'Z', L'[', L'\\', L']', L'^', L'_',
		L'`', L'a', L'b', L'c', L'd', L'e', L'f', L'g',
		L'h', L'i', L'j', L'k', L'l', L'm', L'n', L'o',
		L'p', L'q', L'r', L's', L't', L'u', L'v', L'w',
		L'x', L'y', L'z', L'{', L'|', L'}', L'~', 0x2985,
		0x2986, 0x3002, 0x300C, 0x300D, 0x3001, 0x30FB, 0x30F2, 0x30A1,
		0x30A3, 0x30A5, 0x30A7, 0x30A9, 0x30E3, 0x30E5, 0x30E7, 0x30C3,
		0x30FC, 0x30A2, 0x30A4, 0x30A6, 0x30A8, 0x30AA, 0x30AB, 0x30AD,
		0x30AF, 0x30B1, 0x30B3, 0x30B5, 0x30B7, 0x30B9, 0x30BB, 0x30BD,
		0x30BF, 0x30C1, 0x30C4, 0x30C6, 0x30C8, 0x30CA, 0x30CB, 0x30CC,
		0x30CD, 0x30CE, 0x30CF, 0x30D2, 0x30D5, 0x30D8, 0x30DB, 0x30DE,
		0x30DF, 0x30E0, 0x30E1, 0x30E2, 0x30E4, 0x30E6, 0x30E8, 0x30E9,
		0x30EA, 0x30EB, 0x30EC, 0x30ED, 0x30EF, 0x30F3, 0x3099, 0x309A,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0x00A2, 0x00A3, 0x00AC, 0x00AF, 0x00A6, 0x00A5, 0x20A9, 0,
		0x2502, 0x2190, 0x2191, 0x2192, 0x2193, 0x25A0, 0x25CB, 0,

		// U+FFF0-U+FFFF - not assigned
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	};

	// Convert the string from fullwidth first.
	QString std_str(str);
	for (int i = 0; i < std_str.size(); i++) {
		uint16_t chr = std_str[i].unicode();
		uint16_t old_chr = chr;
		if ((chr & 0xFF00) == 0xFF00) {
			// Fullwidth/halfwidth.
			chr = fwhwToStd[chr & 0xFF];
			std_str[i] = QChar(chr);
		}
	}

	// Convert to integer.
	return std_str.toInt(nullptr, 10);
}

/**
 * Apply variable modifiers to a QHash containing variables.
 * @param varModifierDefs	[in] Variable modifier definitions.
 * @param vars			[in, out] Variables to modify.
 * @param qDateTime		[out, opt] If specified, QDateTime for the timestamp.
 * @return 0 on success; non-zero if any modifiers failed.
 */
int VarReplace::ApplyModifiers(const QHash<QString, VarModifierDef> &varModifierDefs,
			       QHash<QString, QString> &vars,
			       QDateTime *qDateTime)
{
	// Timestamp construction.
	int year = -1, month = -1, day = -1;
	int hour = -1, minute = -1, second = -1;
	int ampm = -1;

	// TODO: Verify that all variables to be modified
	// were present in vars.

	QList<QString> varIDs = vars.keys();
	foreach (const QString &id, varIDs) {
		if (!varModifierDefs.contains(id))
			continue;

		QString var = vars.value(id);
		const VarModifierDef &varModifierDef = varModifierDefs[id];

		// Always convert the string to num and char,
		// in case it's needed for e.g. useAs==month.
		int num = strToInt(var);
		num += varModifierDef.addValue;
		char chr = 0;
		if (var.size() == 1) {
			chr = var.at(0).toLatin1();
			chr += varModifierDef.addValue;
		}

		// Apply the modifier.
		switch (varModifierDef.varType) {
			default:
			case VarModifierDef::VARTYPE_STRING:
				// Parse as a string.
				// Nothing special needs to be done here...
				break;

			case VarModifierDef::VARTYPE_NUMBER:
				// Parse as a number. (Base 10)
				var = QString::number(num, 10);
				break;

			case VarModifierDef::VARTYPE_CHAR:
				// Parse as an ASCII character.
				if (var.size() != 1)
					return -2;
				var = QChar::fromLatin1(chr);
				break;
		}

		// Pad the variable with fillChar, if necessary.
		if (var.size() < varModifierDef.minWidth) {
			var.reserve(varModifierDef.minWidth);
			QChar fillChar = QChar::fromLatin1(varModifierDef.fillChar);
			if (varModifierDef.fieldAlign == VarModifierDef::FIELDALIGN_LEFT) {
				while (var.size() < varModifierDef.minWidth)
					var.append(fillChar);
			} else /*if (variableDef.fieldAlign == VarModifierDef::FIELDALIGN_RIGHT)*/ {
				while (var.size() < varModifierDef.minWidth)
					var.prepend(fillChar);
			}
		}

		// Check if this variable should be used in the QDateTime.
		switch (varModifierDef.useAs) {
			default:
			case VarModifierDef::USEAS_FILENAME:
				// Not a QDateTime component.
				break;

			case VarModifierDef::USEAS_TS_YEAR:
				if (num >= 0 && num <= 99) {
					// 2-digit year.
					year = num + 2000;
				} else if (num >= 2000 && num <= 9999) {
					// 4-digit year.
					year = num;
				} else {
					// Invalid year.
					return -3;
				}
				break;

			case VarModifierDef::USEAS_TS_MONTH: {
				if (num >= 1 && num <= 12)
					month = num;
				else
					return -4;
				break;
			}

			case VarModifierDef::USEAS_TS_DAY:
				if (num >= 1 && num <= 31)
					day = num;
				else
					return -5;
				break;

			case VarModifierDef::USEAS_TS_HOUR:
				if (num >= 0 && num <= 23)
					hour = num;
				else
					return -6;
				break;

			case VarModifierDef::USEAS_TS_MINUTE:
				if (num >= 0 && num <= 59)
					minute = num;
				else
					return -7;
				break;

			case VarModifierDef::USEAS_TS_SECOND:
				if (num >= 0 && num <= 59)
					second = num;
				else
					return -8;
				break;

			case VarModifierDef::USEAS_TS_AMPM:
				// TODO: Implement this once I encounter
				// a save file that actually uses it.
				break;
		}

		// Update the variable in the hash.
		vars.insert(id, var);
	}

	if (qDateTime) {
		// Set the QDateTime to the current time for now.
		const QDateTime currentDateTime(QDateTime::currentDateTime());
		*qDateTime = currentDateTime;

		// Adjust the date.
		QDate date = qDateTime->date();
		const bool isDateSet = (year != -1 || month != -1 || day != -1);
		if (year == -1) {
			year = date.year();
		}
		if (month == -1) {
			month = date.month();
		}
		if (day == -1) {
			day = date.day();
		}
		date.setDate(year, month, day);
		qDateTime->setDate(date);

		// Adjust the time.
		QTime time = qDateTime->time();
		const bool isTimeSet = (hour != -1 || minute != -1);
		if (isDateSet && !isTimeSet) {
			// Date was set by the file, but time wasn't.
			// Assume default of 12:00 AM.
			time.setHMS(0, 0, 0);
		} else {
			if (hour == -1) {
				hour = time.hour();
			}
			if (minute == -1) {
				minute = time.minute();
			}
			if (second == -1) {
				second = 0;	// Don't bother using the current second.
			}
			if (ampm != -1) {
				hour %= 12;
				hour += ampm;
			}
			time.setHMS(hour, minute, second);
		}
		qDateTime->setTime(time);

		// If the QDateTime is more than one day
		// in the future, adjust its years value.
		// (One-day variance is allowed due to timezone differences.)
		const QDateTime tomorrow = QDateTime::fromMSecsSinceEpoch(
			currentDateTime.toMSecsSinceEpoch() + (86400*1000), Qt::UTC);

		if (*qDateTime > tomorrow) {
			QDate adjDate = qDateTime->date();
			int curYear = currentDateTime.date().year();
			// NOTE: Minimum year of 2000 for GCN,
			// but Dreamcast was released in 1998.
			if (curYear > 1995) {
				// Update the QDateTime.
				qDateTime->setDate(adjDate.addYears(-1));
			}
		}
	}

	// Variables modified successfully.
	return 0;
}
