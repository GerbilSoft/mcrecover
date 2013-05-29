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
				QByteArray gamedesc;	// UTF-8, regex
				QByteArray filedesc;	// UTF-8, regex
			} search;

			// Make sure all fields are initialized.
			gcn_file_def()
			{
				this->search.address = 0;
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
	foreach(uint16_t address, addr_file_defs.keys()) {
		QVector<gcn_file_def*> *vec = addr_file_defs.value(address);
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
		errorString = xml.errorString();
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

	// Skip the element name.
	xml.readNext();

	// Next token should be characters.
	if (xml.tokenType() != QXmlStreamReader::Characters)
		return QString();

	// Get the text.
	return xml.text().toString();
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
				// This must be converted to UTF-8 in order to
				// use the regex with libpcre.
				QString gamedesc_str = parseXml_element(xml);
				gcn_file->search.gamedesc = gamedesc_str.toUtf8();
			} else if (xml.name() == QLatin1String("filedesc")) {
				// File description. (regex)
				// This must be converted to UTF-8 in order to
				// use the regex with libpcre.
				QString filedesc_str = parseXml_element(xml);
				gcn_file->search.filedesc = filedesc_str.toUtf8();
			}
		}

		// Next token.
		xml.readNext();
	}
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
