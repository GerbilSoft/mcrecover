/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * VarReplace.hpp: Variable replacement functions.                         *
 *                                                                         *
 * Copyright (c) 2013-2018 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#ifndef __MCRECOVER_VARREPLACE_HPP__
#define __MCRECOVER_VARREPLACE_HPP__

// MemCard Recover includes.
#include "VarModifierDef.hpp"

// Qt includes.
#include <QtCore/qglobal.h>
#include <QtCore/QDateTime>
#include <QtCore/QHash>
#include <QtCore/QString>
#include <QtCore/QRegularExpression>

class VarReplace
{
	private:
		VarReplace();
		~VarReplace();

	private:
		Q_DISABLE_COPY(VarReplace);

	public:
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
		static QString Exec(const QString &str, const QHash<QString, QString> &vars);

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
		static QHash<QString, QString> StringListsToHash(
			const QStringList &gameDescVars,
			const QStringList &fileDescVars);

		/**
		 * Parse a string as an integer.
		 * This function handles fullwidth numbers.
		 * @param str String.
		 * @return Integer.
		 */
		static int strToInt(const QString &str);

		/**
		* Apply variable modifiers to a QHash containing variables.
		* @param varModifierDefs	[in] Variable modifier definitions.
		* @param vars			[in, out] Variables to modify.
		* @param qDateTime		[out, opt] If specified, QDateTime for the timestamp.
		* @return 0 on success; non-zero if any modifiers failed.
		*/
		static int ApplyModifiers(const QHash<QString, VarModifierDef> &varModifierDefs,
					  QHash<QString, QString> &vars,
					  QDateTime *qDateTime);
};

#endif /* __MCRECOVER_VARREPLACE_HPP__ */
