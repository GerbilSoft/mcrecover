/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * PcreRegex.hpp: PCRE regular expression wrapper class.                   *
 *                                                                         *
 * Copyright (c) 2013-2016 by David Korth.                                 *
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

#ifndef __MCRECOVER_PCREREGEX_HPP__
#define __MCRECOVER_PCREREGEX_HPP__

// FIXME: Split config, or do we even want to include this here?
// Maybe we should just allow automatic conversion even though
// it reduces performance?
#include "config.mcrecover.h"

// Qt includes.
#include <QtCore/QByteArray>
#include <QtCore/QString>
#include <QtCore/QVector>

class PcreRegex
{
	public:
		PcreRegex();
		explicit PcreRegex(const QString &regex);
		~PcreRegex();

	private:
		Q_DISABLE_COPY(PcreRegex);

	public:
		/**
		 * Set the regular expression.
		 * This function compiles the regular expression using pcre_compile().
		 * @param regex		[in] Regular expression.
		 * @param errOffset	[out, opt] Error offset if the regex compilation fails.
		 * @return 0 on success; pcre_compile() error on failure.
		 */
		int setRegex(const QString &regex, int *errOffset = nullptr);

		/**
		 * Is a regular expression set?
		 * @return True if set; false if not.
		 */
		bool isRegexSet(void) const
			{ return (m_regex != nullptr); }

#if !defined(HAVE_PCRE16)
		/**
		 * Execute a regular expression.
		 * @param subjectUtf8	[in] Subject to match against, encoded in UTF-8.
		 * @param outVector	[out, opt] Output vector for substring matches.
		 * @return Number of substring matches on success, or PCRE_ERROR_* value (negative number) on error.
		 */
		int exec(const QByteArray &subjectUtf8, QVector<QString> *outVector = nullptr) const;
#endif /* !HAVE_PCRE16 */

#if defined(HAVE_PCRE16)
		/**
		 * Execute a regular expression.
		 * @param subjectUtf16	[in] Subject to match against, encoded in UTF-16.
		 * @param outVector	[out, opt] Output vector for substring matches.
		 * @return Number of substring matches on success, or PCRE_ERROR_* value (negative number) on error.
		 */
		int exec(const QString &subjectUtf16, QVector<QString> *outVector = nullptr) const;
#endif /* HAVE_PCRE16 */

	protected:
		// Compiled PCRE regular expression.
		// NOTE: void* - may be either pcre* or pcre16*.
		void *m_regex;

	public:
		/** PCRE feature queries. **/

		/**
		 * Does PCRE support Unicode?
		 * (UTF-8 for pcre; UTF-16 for pcre16)
		 * @return True if Unicode is supported; false if not.
		 */
		static bool PCRE_has_Unicode(void);

		/**
		 * Does PCRE support Unicode character properties?
		 * @return True if UCP is supported; false if not.
		 */
		static bool PCRE_has_UCP(void);

		/**
		 * Does PCRE support just-in-time compilation?
		 * @return True if JIT is supported; false if not.
		 */
		static bool PCRE_has_JIT(void);
};

#endif /* __MCRECOVER_PCREREGEX_HPP__ */
