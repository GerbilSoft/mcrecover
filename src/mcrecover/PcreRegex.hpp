/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * PcreRegex.hpp: PCRE regular expression wrapper class.                   *
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

#ifndef __MCRECOVER_PCREREGEX_HPP__
#define __MCRECOVER_PCREREGEX_HPP__

// libpcre
struct real_pcre;
typedef struct real_pcre pcre;

// Qt includes.
#include <QtCore/QByteArray>
#include <QtCore/QString>
#include <QtCore/QVector>

class PcreRegex
{
	public:
		PcreRegex();
		PcreRegex(const QString &regex);
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

		/**
		 * Execute a regular expression.
		 * @param subjectUtf8	[in] Subject to match against, encoded in UTF-8.
		 * @param outVector	[out, opt] Output vector for substring matches.
		 * @return Number of substring matches on success, or PCRE_ERROR_* value (negative number) on error.
		 */
		int exec(const QByteArray &subjectUtf8, QVector<QString> *outVector = nullptr) const;

	protected:
		// Compiled PCRE regular expression.
		pcre *m_regex;

	public:
		/** PCRE feature queries. **/

		/**
		 * Does PCRE support UTF-8?
		 * @return True if UTF is supported; false if not.
		 */
		static bool PCRE_has_UTF8(void);

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

		/**
		 * Does PCRE support UTF-16?
		 * @return True if UTF-16 is supported; false if not.
		 */
		static bool PCRE_has_UTF16(void);

		/**
		 * Does PCRE support UTF-32?
		 * @return True if UTF-32 is supported; false if not.
		 */
		static bool PCRE_has_UTF32(void);
};

#endif /* __MCRECOVER_PCREREGEX_HPP__ */
