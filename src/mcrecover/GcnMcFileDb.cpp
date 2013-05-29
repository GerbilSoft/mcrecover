/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * GcnMcFileDb.cpp: GCN Memory CardFile Database class.                    *
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

/**
 * References:
 * - http://www.developer.nokia.com/Community/Wiki/QXmlStreamReader_to_parse_XML_in_Qt
 */

#include "GcnMcFileDb.hpp"

// C includes.
#include <stdint.h>

// Qt includes.
#include <QtCore/QVector>
#include <QtCore/QMap>
#include <QtCore/QFile>
#include <QtXml/QXmlStreamReader>

// libpcre
#include <pcre.h>

class GcnMcFileDbPrivate
{
	public:
		GcnMcFileDbPrivate(GcnMcFileDb *q);
		~GcnMcFileDbPrivate();

	private:
		GcnMcFileDb *const q;
		Q_DISABLE_COPY(GcnMcFileDbPrivate);

	public:
		struct gcn_file_def {
			QString description;
			QString gamecode;
			QString company;

			struct {
				uint32_t address;
				QString gamedesc;	// regex
				QString filedesc;	// regex

				// compiled regexes
				pcre *gamedesc_regex;
				pcre *filedesc_regex;
			} search;

			// Make sure all fields are initialized.
			gcn_file_def()
			{
				this->search.address = 0;
				this->search.gamedesc_regex = NULL;
				this->search.filedesc_regex = NULL;
			}
		};

		/**
		 * GCN memory card file definitions.
		 * - Key: Search address.
		 * - Value: QVector<> of gcn_file_defs.
		 */
		QMap<uint32_t, QVector<gcn_file_def*>* > addr_file_defs;

		/**
		 * Clear the GCN Memory Card File database.
		 * This clears addr_file_defs.
		 */
		void clear(void);

		/**
		 * Load a GCN Memory Card File database.
		 * @param filename Filename of the database file.
		 * @return 0 on success; non-zero on error. (Check errorString()!)
		 */
		int load(QString filename);

		void parseXml_GcnMcFileDb(QXmlStreamReader &xml);
		gcn_file_def *parseXml_file(QXmlStreamReader &xml);
		QString parseXml_element(QXmlStreamReader &xml);
		void parseXml_file_search(QXmlStreamReader &xml, gcn_file_def *gcn_file);
		pcre *compile_regex(QString regex);

		/**
		 * Error string.
		 * Set if an error occurs in load().
		 */
		QString errorString;
};

GcnMcFileDbPrivate::GcnMcFileDbPrivate(GcnMcFileDb *q)
	: q(q)
{ }

GcnMcFileDbPrivate::~GcnMcFileDbPrivate()
{
	clear();
}


/**
 * Clear the GCN Memory Card File database.
 * This clears addr_file_defs.
 */
void GcnMcFileDbPrivate::clear(void)
{
	// Delete all gcn_file_defs.
	foreach (uint16_t address, addr_file_defs.keys()) {
		QVector<gcn_file_def*> *vec = addr_file_defs.value(address);
		foreach (gcn_file_def* gcn_file, *vec) {
			// Make sure the regexes are freed.
			if (gcn_file->search.gamedesc_regex)
				pcre_free(gcn_file->search.gamedesc_regex);
			if (gcn_file->search.filedesc_regex)
				pcre_free(gcn_file->search.filedesc_regex);
		}
		qDeleteAll(*vec);
		delete vec;
	}

	addr_file_defs.clear();
}


/**
 * Load a GCN Memory Card File Database.
 * @param filename Filename of the database file.
 * @return 0 on success; non-zero on error.
 */
int GcnMcFileDbPrivate::load(QString filename)
{
	// Clear the loaded database.
	clear();

	// Attempt to open the specified database file.
	QFile file(filename);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		// Error opening the file.
		// TODO: Show an error message; return a useful error code.
		return -1;
	}

	QXmlStreamReader xml(&file);
	while (!xml.atEnd() && !xml.hasError()) {
		// Read the next element.
		QXmlStreamReader::TokenType token = xml.readNext();
		switch (token) {
			case QXmlStreamReader::StartDocument:
				break;

			case QXmlStreamReader::StartElement:
				// Start of element.
				if (xml.name() == QLatin1String("GcnMcFileDb")) {
					// Start of GcnMcFileDb.
					parseXml_GcnMcFileDb(xml);
				}
				break;

			default:
				break;
		}
	}

	if (xml.hasError()) {
		// XML parse error occurred.
		errorString = xml.errorString() +
			QLatin1String(" (line ") + QString::number(xml.lineNumber()) +
			QLatin1String(", column ") + QString::number(xml.columnNumber()) +
			QChar(L')');
		return -2;
	}

	// Database parsed successfully.
	errorString = QString();
	return 0;
}


void GcnMcFileDbPrivate::parseXml_GcnMcFileDb(QXmlStreamReader &xml)
{
	static const QString myTokenType = QLatin1String("GcnMcFileDb");

	// Check that this is actually a <GcnMcFileDb> element.
	if (xml.tokenType() != QXmlStreamReader::StartElement ||
	    xml.name() != myTokenType) {
		// Not a <GcnMcFileDb> element.
		return;
	}

	// GcnMcFileDb contains <file> elements.
	// Iterate over them until we reach </GcnMcFileDb>.
	xml.readNext();
	while (!xml.hasError() &&
		!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == myTokenType)) {
		if (xml.tokenType() == QXmlStreamReader::StartElement &&
		    xml.name() == QLatin1String("file")) {
			// Found a <file> element.
			gcn_file_def *gcn_file = parseXml_file(xml);
			if (gcn_file) {
				// Add the file to the database.
				const uint32_t address = gcn_file->search.address;
				QVector<gcn_file_def*>* vec = addr_file_defs.value(address);
				if (!vec) {
					// Create a new QVector.
					vec = new QVector<gcn_file_def*>();
					addr_file_defs.insert(address, vec);
				}
				vec->append(gcn_file);
			}
		}

		// Next token.
		xml.readNext();
	}

	// Finished parsing the GcnMcFileDb element.
}


GcnMcFileDbPrivate::gcn_file_def *GcnMcFileDbPrivate::parseXml_file(QXmlStreamReader &xml)
{
	static const QString myTokenType = QLatin1String("file");

	// Check that this is actually a <file> element.
	if (xml.tokenType() != QXmlStreamReader::StartElement ||
	    xml.name() != myTokenType) {
		// Not a <file> element.
		return NULL;
	}

	gcn_file_def *gcn_file = new gcn_file_def;

	// Iterate over the properties.
	xml.readNext();
	while (!xml.hasError() &&
		!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == myTokenType)) {
		if (xml.tokenType() == QXmlStreamReader::StartElement) {
			// Check what this element is.
			if (xml.name() == QLatin1String("description")) {
				// File description.
				gcn_file->description = parseXml_element(xml);
			} else if (xml.name() == QLatin1String("gamecode")) {
				// Game code.
				gcn_file->gamecode = parseXml_element(xml);
			} else if (xml.name() == QLatin1String("company")) {
				// Company code.
				gcn_file->company = parseXml_element(xml);
			} else if (xml.name() == QLatin1String("search")) {
				// Search definitions.
				parseXml_file_search(xml, gcn_file);
			}

			// TODO: File table information.
		}

		// Next token.
		xml.readNext();
	}

	// Return the gcn_file_def.
	return gcn_file;
}


QString GcnMcFileDbPrivate::parseXml_element(QXmlStreamReader &xml)
{
	// Get the element text data.
	// This needs to be a start element.
	if (xml.tokenType() != QXmlStreamReader::StartElement)
		return QString();

	// Read the element's text.
	return xml.readElementText();
}


void GcnMcFileDbPrivate::parseXml_file_search(QXmlStreamReader &xml, gcn_file_def *gcn_file)
{
	static const QString myTokenType = QLatin1String("search");

	// Check that this is actually a <search> element.
	if (xml.tokenType() != QXmlStreamReader::StartElement ||
	    xml.name() != myTokenType) {
		// Not a <search> element.
		return;
	}

	// Iterate over the <search> properties.
	xml.readNext();
	while (!xml.hasError() &&
		!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == myTokenType)) {
		if (xml.tokenType() == QXmlStreamReader::StartElement) {
			// Check what this element is.
			if (xml.name() == QLatin1String("address")) {
				// Search address.
				QString address_str = parseXml_element(xml);
				gcn_file->search.address = address_str.toUInt(NULL, 0);
			} else if (xml.name() == QLatin1String("gamedesc")) {
				// Game description. (regex)
				gcn_file->search.gamedesc = parseXml_element(xml);
			} else if (xml.name() == QLatin1String("filedesc")) {
				// File description. (regex)
				gcn_file->search.filedesc = parseXml_element(xml);
			}
		}

		// Next token.
		xml.readNext();
	}

	// Attempt to compile the regular expressions.

	// Game Description.
	if (gcn_file->search.gamedesc_regex) {
		pcre_free(gcn_file->search.gamedesc_regex);
		gcn_file->search.gamedesc_regex = NULL;
	}
	gcn_file->search.gamedesc_regex = compile_regex(gcn_file->search.gamedesc);

	// File Description.
	if (gcn_file->search.filedesc_regex) {
		pcre_free(gcn_file->search.filedesc_regex);
		gcn_file->search.filedesc_regex = NULL;
	}
	gcn_file->search.filedesc_regex = compile_regex(gcn_file->search.filedesc);
}


pcre *GcnMcFileDbPrivate::compile_regex(QString regex)
{
	if (regex.isEmpty()) {
		// ERROR: Empty regex is not allowed here.
		// TODO: Set an error flag and append a message.
		fprintf(stderr, "WARNING: regex is empty\n");
		return NULL;
	}

	// Convert the regex to UTF-8.
	QByteArray regex_utf8 = regex.toUtf8();

	// Attempt to compile the regex.
	pcre *re;
	const char *error;
	int erroffset;
	re = pcre_compile(
		regex_utf8.constData(),	// pattern
		PCRE_UTF8,		// options
		&error,			// error message
		&erroffset,		// error offset
		NULL);			// use default character tables
	if (!re) {
		// Regex compilation failed.
		fprintf(stderr, "ERROR: Regex compilation failed.\n- Regex: %s\n- Error: %s\n- Offset: %d\n",
			regex_utf8.constData(), error, erroffset);
	}

	return re;
}


/** GcnMcFileDb **/

GcnMcFileDb::GcnMcFileDb(QObject *parent)
	: QObject(parent)
	, d(new GcnMcFileDbPrivate(this))
{ }

GcnMcFileDb::~GcnMcFileDb()
{
	delete d;
}


/**
 * Load a GCN Memory Card File database.
 * @param filename Filename of the database file.
 * @return 0 on success; non-zero on error.
 */
int GcnMcFileDb::load(QString filename)
{
	return d->load(filename);
}


/**
 * Get the error string.
 * This is set if load() fails.
 * @return Error string.
 */
QString GcnMcFileDb::errorString(void)
{
	return d->errorString;
}
