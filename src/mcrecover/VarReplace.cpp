/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * VarReplace.cpp: Variable replacement functions.                         *
 *                                                                         *
 * Copyright (c) 2013 by David Korth.                                      *
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
 * You should have received a copy of the GNU General Public License along *
 * with this program; if not, write to the Free Software Foundation, Inc., *
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.           *
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
			{ return (chr.isLetterOrNumber() || chr == QChar(L'_')); }

		static inline bool isValidVarName(QString varName)
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
QString VarReplace::Exec(const QString str, const QHash<QString, QString> vars)
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
					case L')':
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
 * Combine vectors of GameDesc variables and FileDesc variables into a QHash.
 * @param gameDescVars GameDesc variables.
 * @param fileDescVars FileDesc variables.
 *
 * NOTE: The first variable in each QVector ($G0, $F0) is
 * the full match from PCRE. This usually won't be used,
 * but is included in the variable hash anyway.
 *
 * @return QHash containing the variables. (key == ID)
 */
QHash<QString, QString> VarReplace::VecsToHash(
			const QVector<QString> gameDescVars,
			const QVector<QString> fileDescVars)
{
	QHash<QString, QString> vars;
	QString varName;
	varName.reserve(4);

	for (int i = 0; i < gameDescVars.size(); i++) {
		varName = QChar(L'G') + QString::number(i);
		vars.insert(varName, gameDescVars.at(i));
	}
	for (int i = 0; i < fileDescVars.size(); i++) {
		varName = QChar(L'F') + QString::number(i);
		vars.insert(varName, fileDescVars.at(i));
	}

	return vars;
}

/**
 * Apply variable modifiers to a QHash containing variables.
 * @param varModifierDefs	[in] Variable modifier definitions.
 * @param vars			[in, out] Variables to modify.
 * @param gcnDateTime		[out, opt] If specified, GcnDateTime for the timestamp.
 * @return 0 on success; non-zero if any modifiers failed.
 */
int VarReplace::ApplyModifiers(const QHash<QString, VarModifierDef> varModifierDefs,
			       QHash<QString, QString> &vars,
			       GcnDateTime *gcnDateTime)
{
	// Timestamp construction.
	int year = -1, month = -1, day = -1;
	int hour = -1, minute = -1, second = -1;
	int ampm = -1;

	// TODO: Verify that all variables to be modified
	// were present in vars.

	QList<QString> varIDs = vars.keys();
	foreach (QString id, varIDs) {
		if (!varModifierDefs.contains(id))
			continue;

		QString var = vars.value(id);
		const VarModifierDef &varModifierDef = varModifierDefs[id];

		// Apply the modifier.
		switch (varModifierDef.varType) {
			default:
			case VarModifierDef::VARTYPE_STRING:
				// Parse as a string.
				// Nothing special needs to be done here...
				break;

			case VarModifierDef::VARTYPE_NUMBER: {
				// Parse as a number. (Base 10)
				// TODO: Add support for other bases?
				int num = var.toInt(NULL, 10);
				num += varModifierDef.addValue;
				var = QString::number(num, 10);
				break;
			}

			case VarModifierDef::VARTYPE_CHAR: {
				// Parse as an ASCII character.
				if (var.size() != 1)
					return -2;
				char chr = var.at(0).toLatin1();
				chr += varModifierDef.addValue;
				var = QChar::fromLatin1(chr);
				break;
			}
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

		// Check if this variable should be used in the GcnDateTime.
		int num = var.toInt(NULL, 10);
		switch (varModifierDef.useAs) {
			default:
			case VarModifierDef::USEAS_FILENAME:
				// Not a GcnDateTime component.
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

			case VarModifierDef::USEAS_TS_MONTH:
				if (num >= 1 && num <= 12)
					month = num;
				else
					return -4;
				break;

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

	if (gcnDateTime) {
		// Set the GcnDateTime to the current time for now.
		*gcnDateTime = QDateTime::currentDateTime();

		// Adjust the date.
		QDate date = gcnDateTime->date();
		if (year == -1)
			year = date.year();
		if (month == -1)
			month = date.month();
		if (day == -1)
			day = date.day();
		date.setDate(year, month, day);
		gcnDateTime->setDate(date);

		// Adjust the time.
		QTime time = gcnDateTime->time();
		if (hour == -1)
			hour = time.hour();
		if (minute == -1)
			minute = time.minute();
		if (second == -1)
			second = 0;	// Don't bother using the current second.
		if (ampm != -1) {
			hour %= 12;
			hour += ampm;
		}
		time.setHMS(hour, minute, second);
		gcnDateTime->setTime(time);
	}

	// Variables modified successfully.
	return 0;
}
