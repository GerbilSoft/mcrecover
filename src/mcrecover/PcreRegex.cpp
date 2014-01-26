/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * PcreRegex.cpp: PCRE regular expression wrapper class.                   *
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

#include "PcreRegex.hpp"
#include "util/array_size.h"

// libpcre
#include <pcre.h>
// PCRE feature flags.
#ifndef PCRE_CONFIG_UTF8
#define PCRE_CONFIG_UTF8 0
#endif
#ifndef PCRE_CONFIG_UNICODE_PROPERTIES
#define PCRE_CONFIG_UNICODE_PROPERTIES 6
#endif
#ifndef PCRE_CONFIG_JIT
#define PCRE_CONFIG_JIT 9
#endif
#ifndef PCRE_CONFIG_UTF16
#define PCRE_CONFIG_UTF16 10
#endif
#ifndef PCRE_CONFIG_JITTARGET
#define PCRE_CONFIG_JITTARGET 11
#endif
#ifndef PCRE_CONFIG_UTF32
#define PCRE_CONFIG_UTF32 12
#endif

PcreRegex::PcreRegex()
	: m_regex(nullptr)
{ }

PcreRegex::PcreRegex(const QString &regex)
{
	// NOTE: If regex compilation fails, no error will be returned.
	setRegex(regex);
}

PcreRegex::~PcreRegex()
{
	// Free the compiled PCRE regex if one was allocated.
	if (m_regex)
		pcre_free(m_regex);
}

/**
 * Set the regular expression.
 * This function compiles the regular expression using pcre_compile().
 * @param regex		[in] Regular expression.
 * @param errOffset	[out, opt] Error offset if the regex compilation fails.
 * @return 0 on success; pcre_compile() error code on failure.
 */
int PcreRegex::setRegex(const QString &regex, int *errOffset)
{
	// Free the existing regex if one is set.
	if (m_regex)
		pcre_free(m_regex);

	// Don't allow empty regular expressions.
	if (regex.isEmpty()) {
		m_regex = nullptr;
		return -1; // NOTE: Not an actual pcre_compile2() error code...
	}

	// Convert the regex to UTF-8.
	QByteArray regex_utf8 = regex.toUtf8();

	// Attempt to compile the regex.
	const char *error;
	int erroffset;
	int errorcode;
	m_regex = pcre_compile2(
		regex_utf8.constData(),	// pattern
		PCRE_UTF8,		// options
		&errorcode,		// error code
		&error,			// error message
		&erroffset,		// error offset
		nullptr);		// use default character tables

	if (!m_regex) {
		// Regex compilation failed.
		if (errOffset)
			*errOffset = erroffset;
		return errorcode;
	}

	// Regex compiled successfully.
	return 0;
}

/**
 * Execute a regular expression.
 * @param subjectUtf8	[in] Subject to match against, encoded in UTF-8.
 * @param outVector	[out, opt] Output vector for substring matches.
 * @return Number of substring matches on success, or PCRE_ERROR_* value (negative number) on error.
 */
int PcreRegex::exec(const QByteArray &subjectUtf8, QVector<QString> *outVector) const
{
	if (!m_regex)
		return PCRE_ERROR_NOMATCH;

	// Output vector.
	// Supports up to 20 substring matches.
	static const int MAX_SUBSTRINGS = 20;
	int ovector[MAX_SUBSTRINGS*3];

	int rc = pcre_exec(
		m_regex,			// compiled regex
		nullptr,			// pattern not studied
		subjectUtf8.constData(),	// subject string
		subjectUtf8.size(),		// size of subject string
		0,				// start at offset 0 in the subject
		0,				// default options
		ovector,			// vector of integers for substring information
		ARRAY_SIZE(ovector));		// number of elements in ovector

	if (rc >= 0 && outVector) {
		// Substring matches found.
		// Store the substrings in the specified QVector.
		outVector->clear();

		// Substring count.
		// rc == 0: we ran out of space; use MAX_SUBSTRINGS.
		// rc > 0: rc is number of substrings.
		const int count = (rc > 0 ? rc : MAX_SUBSTRINGS);
		outVector->reserve(count);

		// Get the substrings.
		// TODO: Verify that count == number of substrings?
		const char **listptr;
		int rc_get = pcre_get_substring_list(
			subjectUtf8.constData(),
			ovector, count, &listptr);
		if (rc_get != 0) {
			// pcre_get_substring_list() failed.
			return rc_get;
		}

		// Convert the list of strings to QVector<QString>.
		for (const char **listiter = listptr; *listiter != nullptr; listiter++) {
			QString str = QString::fromUtf8(*listiter);
			outVector->append(str);
		}

		pcre_free(listptr);
	}

	return rc;
}

/** PCRE feature queries. **/

/**
 * Does PCRE support UTF-8?
 * @return True if UTF-8 is supported; false if not.
 */
bool PcreRegex::PCRE_has_UTF8(void)
{
	int ret, val;
	ret = pcre_config(PCRE_CONFIG_UTF8, &val);
	return (ret == 0 && val == 1);
}

/**
 * Does PCRE support Unicode character properties?
 * @return True if UCP is supported; false if not.
 */
bool PcreRegex::PCRE_has_UCP(void)
{
	int ret, val;
	ret = pcre_config(PCRE_CONFIG_UNICODE_PROPERTIES, &val);
	return (ret == 0 && val == 1);
}

/**
 * Does PCRE support just-in-time compilation?
 * @return True if JIT is supported; false if not.
 */
bool PcreRegex::PCRE_has_JIT(void)
{
	int ret, val;
	ret = pcre_config(PCRE_CONFIG_JIT, &val);
	return (ret == 0 && val == 1);
}

/**
 * Does PCRE support UTF-16?
 * @return True if UTF-16 is supported; false if not.
 */
bool PcreRegex::PCRE_has_UTF16(void)
{
	int ret, val;
	ret = pcre_config(PCRE_CONFIG_UTF16, &val);
	return (ret == 0 && val == 1);
}

/**
 * Does PCRE support UTF-32?
 * @return True if UTF-32 is supported; false if not.
 */
bool PcreRegex::PCRE_has_UTF32(void)
{
	int ret, val;
	ret = pcre_config(PCRE_CONFIG_UTF32, &val);
	return (ret == 0 && val == 1);
}
