/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * GcnMcFileDb.cpp: GCN Memory Card File Database class.                   *
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

/**
 * References:
 * - http://www.developer.nokia.com/Community/Wiki/QXmlStreamReader_to_parse_XML_in_Qt
 */

#include "config.mcrecover.h"
#include "GcnMcFileDb.hpp"
#include "config/ConfigStore.hpp"

#include "GcnMcFileDef.hpp"
#include "VarReplace.hpp"
#include "libmemcard/TimeFuncs.hpp"

// GcnFile
#include "libmemcard/GcnFile.hpp"

// C includes.
#include <stdint.h>

// C includes. (C++ namespace)
#include <cassert>
#include <cctype>
#include <cstdio>
#include <cstring>

// Qt includes.
#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QMap>
#include <QtCore/QTextCodec>
#include <QtCore/QVector>
#include <QtCore/QXmlStreamReader>

class GcnMcFileDbPrivate
{
	public:
		explicit GcnMcFileDbPrivate(GcnMcFileDb *q);
		~GcnMcFileDbPrivate();

	private:
		GcnMcFileDb *const q_ptr;
		Q_DECLARE_PUBLIC(GcnMcFileDb)

		// Block size.
		static const uint32_t BLOCK_SIZE = 0x2000;
		static const uint32_t BLOCK_SIZE_MASK = (BLOCK_SIZE - 1);

	public:
		/**
		 * GCN memory card file definitions.
		 * - Key: Search address. (limited to BLOCK_SIZE-1)
		 * - Value: QVector<>* of GcnMcFileDef*.
		 */
		QMap<uint32_t, QVector<GcnMcFileDef*>*> addr_file_defs;

		/**
		 * Convert a region character to a GcnMcFileDef::regions_t bitfield value.
		 * @param regionChr Region character.
		 * @return region_t value, or 0 if unknown.
		 */
		static uint8_t RegionCharToBitfield(QChar regionChr);

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
		int load(const QString &filename);

		void parseXml_GcnMcFileDb(QXmlStreamReader &xml);
		GcnMcFileDef *parseXml_file(QXmlStreamReader &xml);
		QString parseXml_element(QXmlStreamReader &xml);
		void parseXml_file_search(QXmlStreamReader &xml, GcnMcFileDef *gcnMcFileDef);
		void parseXml_file_checksum(QXmlStreamReader &xml, GcnMcFileDef *gcnMcFileDef);
		void parseXml_file_dirEntry(QXmlStreamReader &xml, GcnMcFileDef *gcnMcFileDef);
		void parseXml_file_variables(QXmlStreamReader &xml, GcnMcFileDef *gcnMcFileDef);
		void parseXml_file_variable(QXmlStreamReader &xml, GcnMcFileDef *gcnMcFileDef);

		/**
		 * Error string.
		 * Set if an error occurs in load().
		 */
		QString errorString;

		// Text codecs.
		QTextCodec *const textCodecJP;
		QTextCodec *const textCodecUS;

		/**
		 * Get a comment from the GCN comment block, converted to UTF-16.
		 * @param buf Comment block.
		 * @param siz Size of comment block. (usually 32)
		 * @param textCodec QTextCodec. (If nullptr, use latin1.)
		 * @return GCN comment block, converted to UTF-16.
		 */
		static QString GetGcnCommentUtf16(const char *buf, int siz, QTextCodec *textCodec);

		/**
		 * Get a comment from the GCN comment block, converted to UTF-8.
		 * @param buf Comment block.
		 * @param siz Size of comment block. (usually 32)
		 * @param textCodec QTextCodec. (If nullptr, use latin1.)
		 * @return GCN comment block, converted to UTF-8.
		 */
		static QByteArray GetGcnCommentUtf8(const char *buf, int siz, QTextCodec *textCodec);

		/**
		 * Construct a GcnSearchData entry.
		 * @param matchFileDef	[in] File definition.
		 * @param vars		[in] Variables.
		 * @param qDateTime	[in] Timestamp.
		 * @return GcnSearchData entry.
		 */
		GcnSearchData constructSearchData(
			const GcnMcFileDef *matchFileDef,
			const QHash<QString, QString> &vars,
			const QDateTime &qDateTime) const;
};

GcnMcFileDbPrivate::GcnMcFileDbPrivate(GcnMcFileDb *q)
	: q_ptr(q)
	, textCodecJP(QTextCodec::codecForName("Shift-JIS"))
	, textCodecUS(QTextCodec::codecForName("Windows-1252"))
{ }

GcnMcFileDbPrivate::~GcnMcFileDbPrivate()
{
	clear();
}


/**
 * Convert a region character to a GcnMcFileDef::regions_t bitfield value.
 * @param regionChr Region character.
 * @return region_t value, or 0 if unknown.
 */
uint8_t GcnMcFileDbPrivate::RegionCharToBitfield(QChar regionChr)
{
	switch (regionChr.unicode()) {
		case 'J':	return GcnMcFileDef::REGION_JPN;
		case 'E':	return GcnMcFileDef::REGION_USA;
		case 'P':	return GcnMcFileDef::REGION_EUR;
		case 'K':	return GcnMcFileDef::REGION_KOR;
		default:
			break;
	}

	// Unknown region character.
	// TODO: Show an error message?
	return 0;
}


/**
 * Clear the GCN Memory Card File database.
 * This clears addr_file_defs.
 */
void GcnMcFileDbPrivate::clear(void)
{
	// Delete all GcnMcFileDefs.
	for (QMap<uint32_t, QVector<GcnMcFileDef*>*>::iterator iter = addr_file_defs.begin();
	     iter != addr_file_defs.end(); ++iter)
	{
		QVector<GcnMcFileDef*> *vec = *iter;
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
int GcnMcFileDbPrivate::load(const QString &filename)
{
	// Clear the loaded database.
	clear();

	// Attempt to open the specified database file.
	QFile file(filename);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		// Error opening the file.
		errorString = file.errorString();
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
			GcnMcFileDef *gcnMcFileDef = parseXml_file(xml);
			if (gcnMcFileDef && gcnMcFileDef->search.address > BLOCK_SIZE_MASK) {
				// FIXME: Support for files with search address above 0x1FFF.
				delete gcnMcFileDef;
			} else if (gcnMcFileDef) {
				// Add the file to the database.
				uint32_t address = gcnMcFileDef->search.address;
				address &= BLOCK_SIZE_MASK;	// search the specific block only
				QVector<GcnMcFileDef*>* vec = addr_file_defs.value(address);
				if (!vec) {
					// Create a new QVector.
					vec = new QVector<GcnMcFileDef*>();
					addr_file_defs.insert(address, vec);
				}
				vec->append(gcnMcFileDef);
			}
		} else {
			// Skip unreocgnized tokens.
			xml.readElementText(QXmlStreamReader::SkipChildElements);
		}

		// Next token.
		xml.readNext();
	}

	// Finished parsing the GcnMcFileDb element.
}


GcnMcFileDef *GcnMcFileDbPrivate::parseXml_file(QXmlStreamReader &xml)
{
	static const QString myTokenType = QLatin1String("file");

	// Check that this is actually a <file> element.
	if (xml.tokenType() != QXmlStreamReader::StartElement ||
	    xml.name() != myTokenType) {
		// Not a <file> element.
		return nullptr;
	}

	GcnMcFileDef *gcnMcFileDef = new GcnMcFileDef;
	QString regionStr;

	// TODO: Combine gamecode/company into ID6.

	// Iterate over the properties.
	xml.readNext();
	while (!xml.hasError() &&
		!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == myTokenType)) {
		if (xml.tokenType() == QXmlStreamReader::StartElement) {
			// Check what this element is.
			if (xml.name() == QLatin1String("gameName")) {
				// Game name.
				gcnMcFileDef->gameName = parseXml_element(xml);
			} else if (xml.name() == QLatin1String("fileInfo")) {
				// File information.
				gcnMcFileDef->gameName = parseXml_element(xml);
			} else if (xml.name() == QLatin1String("gamecode")) {
				// Game code.
				QString gamecode = parseXml_element(xml);
				gamecode.resize(sizeof(gcnMcFileDef->gamecode));
				memcpy(gcnMcFileDef->gamecode,
					gamecode.toLatin1().constData(),
					sizeof(gcnMcFileDef->gamecode));
			} else if (xml.name() == QLatin1String("company")) {
				// Company code.
				QString company = parseXml_element(xml);
				company.resize(sizeof(gcnMcFileDef->company));
				memcpy(gcnMcFileDef->company,
					company.toLatin1().constData(),
					sizeof(gcnMcFileDef->company));
			} else if (xml.name() == QLatin1String("regions")) {
				// Additional region codes.
				regionStr += parseXml_element(xml);
			} else if (xml.name() == QLatin1String("search")) {
				// Search definitions.
				parseXml_file_search(xml, gcnMcFileDef);
			} else if (xml.name() == QLatin1String("checksum")) {
				// Checksum definitions.
				parseXml_file_checksum(xml, gcnMcFileDef);
			} else if (xml.name() == QLatin1String("dirEntry")) {
				// Directory entry.
				parseXml_file_dirEntry(xml, gcnMcFileDef);
			} else if (xml.name() == QLatin1String("variables")) {
				// Variable modifiers.
				parseXml_file_variables(xml, gcnMcFileDef);
			} else {
				// Skip unreocgnized tokens.
				xml.readElementText(QXmlStreamReader::SkipChildElements);
			}
		}

		// Next token.
		xml.readNext();
	}

	// Determine the main region code from the game code.
	QChar regionChr((ushort)gcnMcFileDef->gamecode[3]);
	gcnMcFileDef->regions = RegionCharToBitfield(regionChr);

	// Parse additional region codes.
	for (int i = (regionStr.length() - 1); i >= 0; i--) {
		QChar regionChr = regionStr.at(i);
		gcnMcFileDef->regions |= RegionCharToBitfield(regionChr);
	}

	// Return the GcnMcFileDef.
	return gcnMcFileDef;
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


void GcnMcFileDbPrivate::parseXml_file_search(QXmlStreamReader &xml, GcnMcFileDef *gcnMcFileDef)
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
				gcnMcFileDef->search.address = address_str.toUInt(nullptr, 0);
			} else if (xml.name() == QLatin1String("gameDesc")) {
				// Game description. (regex)
				gcnMcFileDef->search.gameDesc = parseXml_element(xml);
			} else if (xml.name() == QLatin1String("fileDesc")) {
				// File description. (regex)
				gcnMcFileDef->search.fileDesc = parseXml_element(xml);
			} else {
				// Skip unreocgnized tokens.
				xml.readElementText(QXmlStreamReader::SkipChildElements);
			}
		}

		// Next token.
		xml.readNext();
	}

	// Set the regular expressions.
	gcnMcFileDef->search.gameDesc_regex.setPattern(gcnMcFileDef->search.gameDesc);
	gcnMcFileDef->search.fileDesc_regex.setPattern(gcnMcFileDef->search.fileDesc);
#if QT_VERSION >= QT_VERSION_CHECK(5,4,0)
	// TODO: If compiling with older Qt, set QRegularExpression::OptimizeOnFirstUsageOption.
	// This will allow optimization if used with newer Qt without recompiling.
	// QRegularExpression::PatternOption enum value 0x0080
	// QRegularExpression::setPatternOptions()
	gcnMcFileDef->search.gameDesc_regex.optimize();
	gcnMcFileDef->search.fileDesc_regex.optimize();
#endif /* QT_VERSION >= QT_VERSION_CHECK(5,4,0) */
}


void GcnMcFileDbPrivate::parseXml_file_checksum(QXmlStreamReader &xml, GcnMcFileDef *gcnMcFileDef)
{
	static const QString myTokenType = QLatin1String("checksum");

	// Check that this is actually a <checksum> element.
	if (xml.tokenType() != QXmlStreamReader::StartElement ||
	    xml.name() != myTokenType) {
		// Not a <checksum> element.
		return;
	}

	// Struct for the checksum definition.
	Checksum::ChecksumDef checksumDef;

	// Decode the algorithm later.
	QString algorithm;
	uint32_t poly = 0;

	// Multiple checksums with identical properties.
	static const int INSTANCES_DEFAULT = 1;
	int instances = INSTANCES_DEFAULT;
	static const uint32_t INCREMENT_DEFAULT = 0x2000;
	uint32_t increment = INCREMENT_DEFAULT;

	// Iterate over the <search> properties.
	xml.readNext();
	while (!xml.hasError() &&
		!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == myTokenType)) {
		if (xml.tokenType() == QXmlStreamReader::StartElement) {
			// Check what this element is.
			if (xml.name() == QLatin1String("algorithm")) {
				// Algorithm.
				QXmlStreamAttributes attributes = xml.attributes();

				// Polynomial attribute. (CRC-16, CRC-32)
				if (attributes.hasAttribute(QLatin1String("poly")))
					poly = attributes.value(QLatin1String("poly")).toString().toUInt(nullptr, 0);
				else
					poly = 0;

				algorithm = parseXml_element(xml).toLower();
			} else if (xml.name() == QLatin1String("address")) {
				// Checksum address.
				QString address_str = parseXml_element(xml);
				checksumDef.address = address_str.toUInt(nullptr, 0);
			} else if (xml.name() == QLatin1String("range")) {
				// Checksummed area.
				QXmlStreamAttributes attributes = xml.attributes();
				if (attributes.hasAttribute(QLatin1String("start")) &&
				    attributes.hasAttribute(QLatin1String("length"))) {
					// Required attributes are present.
					checksumDef.start =
						attributes.value(QLatin1String("start")).toString().toUInt(nullptr, 0);
					checksumDef.length =
						attributes.value(QLatin1String("length")).toString().toUInt(nullptr, 0);
				} else {
					// Attributes missing.
					// TODO: Show error message?
				}
			} else if (xml.name() == QLatin1String("multiple")) {
				// Multiple instances.
				QXmlStreamAttributes attributes = xml.attributes();

				if (attributes.hasAttribute(QLatin1String("instances"))) {
					// Number of instances.
					instances =
						attributes.value(QLatin1String("instances")).toString().toUInt(nullptr, 0);
				} else {
					// Instances not specified.
					instances = INSTANCES_DEFAULT;
				}

				if (attributes.hasAttribute(QLatin1String("increment"))) {
					// Byte increment between checksums.
					increment =
						attributes.value(QLatin1String("increment")).toString().toUInt(nullptr, 0);
				} else {
					// Increment not specified.
					increment = INCREMENT_DEFAULT;
				}
			} else {
				// Skip unreocgnized tokens.
				xml.readElementText(QXmlStreamReader::SkipChildElements);
			}
		}

		// Next token.
		xml.readNext();
	}

	if (checksumDef.length == 0) {
		// Invalid length.
		// TODO: Show an error message.
		return;
	}

	// Determine which checksum algorithm to use.
	checksumDef.algorithm = Checksum::ChkAlgorithmFromString(algorithm.toLatin1().constData());
	switch (checksumDef.algorithm) {
		case Checksum::CHKALG_CRC16:
			checksumDef.param =
				(poly != 0 ? (poly & 0xFFFF) : Checksum::CRC16_POLY_CCITT);
			break;

		case Checksum::CHKALG_CRC32:
			checksumDef.param =
				(poly != 0 ? poly : Checksum::CRC32_POLY_ZLIB);
			break;

		case Checksum::CHKALG_NONE:
			// Unknown algorithm.
			// TODO: Show an error message?
			return;

		default:
			// Other algorithm.
			// No parameter is required.
			checksumDef.param = 0;
			break;
	}

	// Clamp instances to an upper limit of 2043.
	// (Maximum number of blocks in a memory card.)
	if (instances > 2043)
		instances = 2043;

	// TODO: Make sure (instances * increment) doesn't go past the end of the card?
	for (; instances > 0; instances--) {
		// Add the checksum definition.
		gcnMcFileDef->checksumDefs.append(checksumDef);

		// Next instance.
		checksumDef.address += increment;
		checksumDef.start += increment;
	}
}


void GcnMcFileDbPrivate::parseXml_file_dirEntry(QXmlStreamReader &xml, GcnMcFileDef *gcnMcFileDef)
{
	static const QString myTokenType = QLatin1String("dirEntry");

	// Check that this is actually a <dirEntry> element.
	if (xml.tokenType() != QXmlStreamReader::StartElement ||
	    xml.name() != myTokenType) {
		// Not a <dirEntry> element.
		return;
	}

	// Iterate over the <dirEntry> properties.
	xml.readNext();
	QString str;	// temporary string
	while (!xml.hasError() &&
		!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == myTokenType)) {
		if (xml.tokenType() == QXmlStreamReader::StartElement) {
			// Check what this element is.
			if (xml.name() == QLatin1String("filename")) {
				// Filename.
				gcnMcFileDef->dirEntry.filename = parseXml_element(xml);
			} else if (xml.name() == QLatin1String("bannerFormat")) {
				// Banner format.
				str = parseXml_element(xml);
				gcnMcFileDef->dirEntry.bannerFormat = (uint8_t)str.toUInt(nullptr, 0);
			} else if (xml.name() == QLatin1String("iconAddress")) {
				// Icon address.
				str = parseXml_element(xml);
				gcnMcFileDef->dirEntry.iconAddress = str.toUInt(nullptr, 0);
			} else if (xml.name() == QLatin1String("iconFormat")) {
				// Icon format.
				str = parseXml_element(xml);
				gcnMcFileDef->dirEntry.iconFormat = str.toUShort(nullptr, 0);
			} else if (xml.name() == QLatin1String("iconSpeed")) {
				// Icon speed.
				str = parseXml_element(xml);
				gcnMcFileDef->dirEntry.iconSpeed = str.toUShort(nullptr, 0);
			} else if (xml.name() == QLatin1String("permission")) {
				// Permission.
				str = parseXml_element(xml);
				gcnMcFileDef->dirEntry.permission = (uint8_t)str.toUInt(nullptr, 0);
			} else if (xml.name() == QLatin1String("length")) {
				// Length, in blocks.
				str = parseXml_element(xml);
				gcnMcFileDef->dirEntry.length = str.toUShort(nullptr, 0);
			} else {
				// Skip unreocgnized tokens.
				xml.readElementText(QXmlStreamReader::SkipChildElements);
			}
		}

		// Next token.
		xml.readNext();
	}
}


void GcnMcFileDbPrivate::parseXml_file_variables(QXmlStreamReader &xml, GcnMcFileDef *gcnMcFileDef)
{
	static const QString myTokenType = QLatin1String("variables");

	// Check that this is actually a <variables> element.
	if (xml.tokenType() != QXmlStreamReader::StartElement ||
	    xml.name() != myTokenType) {
		// Not a <variables> element.
		return;
	}

	// Iterate over the <variables>.
	xml.readNext();
	while (!xml.hasError() &&
		!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == myTokenType)) {
		if (xml.tokenType() == QXmlStreamReader::StartElement) {
			// Check what this element is.
			if (xml.name() == QLatin1String("variable")) {
				// Variable definition.
				parseXml_file_variable(xml, gcnMcFileDef);
			} else {
				// Skip unreocgnized tokens.
				xml.readElementText(QXmlStreamReader::SkipChildElements);
			}
		}

		// Next token.
		xml.readNext();
	}
}


void GcnMcFileDbPrivate::parseXml_file_variable(QXmlStreamReader& xml, GcnMcFileDef* gcnMcFileDef)
{
	static const QString myTokenType = QLatin1String("variable");

	// Check that this is actually a <variable> element.
	if (xml.tokenType() != QXmlStreamReader::StartElement ||
	    xml.name() != myTokenType) {
		// Not a <variable> element.
		return;
	}

	// Get the variable ID.
	QString id;
	QXmlStreamAttributes attributes = xml.attributes();
	if (attributes.hasAttribute(QLatin1String("id")))
		id = attributes.value(QLatin1String("id")).toString();
	if (id.isEmpty()) {
		// No ID specified.
		// TODO: Show error message?
	}

	// Variable modiier definition.
	VarModifierDef varModifierDef;

	// Iterate over the <variable> properties.
	xml.readNext();
	QString str;	// temporary string
	while (!xml.hasError() &&
		!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == myTokenType)) {
		if (xml.tokenType() == QXmlStreamReader::StartElement) {
			// Check what this element is.
			if (xml.name() == QLatin1String("useAs")) {
				str = parseXml_element(xml).toLower();
				if (str == QLatin1String("year"))
					varModifierDef.useAs = VarModifierDef::USEAS_TS_YEAR;
				else if (str == QLatin1String("month"))
					varModifierDef.useAs = VarModifierDef::USEAS_TS_MONTH;
				else if (str == QLatin1String("day"))
					varModifierDef.useAs = VarModifierDef::USEAS_TS_DAY;
				else if (str == QLatin1String("hour"))
					varModifierDef.useAs = VarModifierDef::USEAS_TS_HOUR;
				else if (str == QLatin1String("minute"))
					varModifierDef.useAs = VarModifierDef::USEAS_TS_MINUTE;
				else if (str == QLatin1String("second"))
					varModifierDef.useAs = VarModifierDef::USEAS_TS_SECOND;
				else if (str == QLatin1String("ampm"))
					varModifierDef.useAs = VarModifierDef::USEAS_TS_AMPM;
				else //if (str == QLatin1String("filename"))
					varModifierDef.useAs = VarModifierDef::USEAS_FILENAME;
			} else if (xml.name() == QLatin1String("type")) {
				str = parseXml_element(xml).toLower();
				if (str == QLatin1String("number"))
					varModifierDef.varType = VarModifierDef::VARTYPE_NUMBER;
				else if (str == QLatin1String("char"))
					varModifierDef.varType = VarModifierDef::VARTYPE_CHAR;
				else //if (str == QLatin1String("string"))
					varModifierDef.varType = VarModifierDef::VARTYPE_STRING;
			} else if (xml.name() == QLatin1String("minWidth")) {
				// Minimum field width.
				str = parseXml_element(xml);
				varModifierDef.minWidth = (uint8_t)str.toUInt(nullptr, 0);
			} else if (xml.name() == QLatin1String("fillChar")) {
				// Fill character.
				// TODO: Show an error if the string is not exactly 1 character?
				// TODO: Show an error if the character is not ASCII?
				str = parseXml_element(xml);
				const QChar fillChar = (str.isEmpty() ? QChar(L' ') : str.at(0));
				varModifierDef.fillChar = fillChar.toLatin1();
			} else if (xml.name() == QLatin1String("align")) {
				// Field alignment.
				str = parseXml_element(xml).toLower();
				if (str == QLatin1String("left"))
					varModifierDef.fieldAlign = VarModifierDef::FIELDALIGN_LEFT;
				else //if (str == QLatin1String("right"))
					varModifierDef.fieldAlign = VarModifierDef::FIELDALIGN_RIGHT;
			} else if (xml.name() == QLatin1String("add")) {
				// Add value.
				// TODO: Show an error if not number or char?
				str = parseXml_element(xml).toLower();
				varModifierDef.addValue = str.toInt(nullptr, 0);
			} else {
				// Skip unreocgnized tokens.
				xml.readElementText(QXmlStreamReader::SkipChildElements);
			}
		}

		// Next token.
		xml.readNext();
	}

	// Add the variable modifier definition.
	// TODO: Show warning if this is a duplicate, or if the ID is invalid.
	if (!id.isEmpty())
		gcnMcFileDef->varModifiers.insert(id, varModifierDef);
}

/**
 * Get a comment from the GCN comment block, converted to UTF-16.
 * @param buf Comment block.
 * @param siz Size of comment block. (usually 32)
 * @param textCodec QTextCodec. (If nullptr, use latin1.)
 * @return GCN comment block, converted to UTF-16.
 */
QString GcnMcFileDbPrivate::GetGcnCommentUtf16(const char *buf, int siz, QTextCodec *textCodec)
{
	// Remove trailing NULL characters before converting to UTF-8.
	const char *p_nullChr = (const char*)memchr(buf, 0x00, siz);
	if (p_nullChr) {
		// Found a NULL character.
		if (p_nullChr == buf)
			return QString();
		siz = (p_nullChr - buf);
	}

	// Convert the comment to Unicode.
	// Trim the comment while we're at it.
	QString comment;
	if (!textCodec) {
		// No text codec was specified.
		// Default to Latin-1.
		comment = QString::fromLatin1(buf, siz).trimmed();
	} else {
		// Use the text codec.
		comment = textCodec->toUnicode(buf, siz).trimmed();
	}

	// Comment converted to UTF-16.
	return comment;
}

/**
 * Get a comment from the GCN comment block, converted to UTF-8.
 * @param buf Comment block.
 * @param siz Size of comment block. (usually 32)
 * @param textCodec QTextCodec. (If nullptr, use latin1.)
 * @return GCN comment block, converted to UTF-8.
 */
QByteArray GcnMcFileDbPrivate::GetGcnCommentUtf8(const char *buf, int siz, QTextCodec *textCodec)
{
	// This is now a wrapper around GetGcnCommentUtf16().
	return GetGcnCommentUtf16(buf, siz, textCodec).toUtf8();
}

/**
 * Construct a GcnSearchData entry.
 * @param matchFileDef	[in] File definition.
 * @param vars		[in] Variables.
 * @param qDateTime	[in] Timestamp.
 * @return GcnSearchData entry.
 */
GcnSearchData GcnMcFileDbPrivate::constructSearchData(
	const GcnMcFileDef *matchFileDef,
	const QHash<QString, QString> &vars,
	const QDateTime &qDateTime) const
{
	// TODO: Implicitly share GcnSearchData?
	GcnSearchData searchData;
	card_direntry *const dirEntry = &searchData.dirEntry;
	memset(dirEntry, 0x00, sizeof(*dirEntry));
	
	// Game and company codes.
	memcpy(dirEntry->gamecode, matchFileDef->gamecode, sizeof(dirEntry->gamecode));
	memcpy(dirEntry->company,  matchFileDef->company,  sizeof(dirEntry->company));

	// Convert the filename to the correct encoding.
	QByteArray ba;

	// Substitute variables in the filename.
	QString filename = VarReplace::Exec(matchFileDef->dirEntry.filename, vars);

	// Filename.
	// FIXME: Also for 'S' (used by SADX preview)?
	if (dirEntry->gamecode[3] == 'J' && textCodecJP) {
		// JP file. Convert to Shift-JIS.
		ba = textCodecJP->fromUnicode(filename);
	} else if (textCodecUS) {
		// US/EU file. Convert to cp1252.
		ba = textCodecUS->fromUnicode(filename);
	}

	if (ba.isEmpty()) {
		// QByteArray is empty. Conversion failed.
		// Convert to Latin1 instead.
		ba = filename.toLatin1();
	}

	if (ba.length() > (int)sizeof(dirEntry->filename))
		ba.resize(sizeof(dirEntry->filename));
	strncpy(dirEntry->filename, ba.constData(), sizeof(dirEntry->filename));
	// TODO: Make sure the filename is null-terminated?

	// Values.
	/**
	 * TODO:
	 * - Use the actual starting block?
	 * - Block offsets for files with commentaddr >= 0x2000
	 * - Support for variable-length files?
	 */
	dirEntry->pad_00	= 0xFF;
	dirEntry->bannerfmt	= matchFileDef->dirEntry.bannerFormat;
	dirEntry->lastmodified	= TimeFuncs::toGcnTimestamp(qDateTime);
	dirEntry->iconaddr	= matchFileDef->dirEntry.iconAddress;
	dirEntry->iconfmt	= matchFileDef->dirEntry.iconFormat;
	dirEntry->iconspeed	= matchFileDef->dirEntry.iconSpeed;
	dirEntry->permission	= matchFileDef->dirEntry.permission;
	dirEntry->copytimes	= 0;
	dirEntry->block		= 5;	// FIXME
	dirEntry->length	= matchFileDef->dirEntry.length;
	dirEntry->pad_01	= 0xFFFF;
	dirEntry->commentaddr	= matchFileDef->search.address;

	// Checksum data.
	searchData.checksumDefs = matchFileDef->checksumDefs;

	// Return the SearchData entry.
	return searchData;
}


/** GcnMcFileDb **/

GcnMcFileDb::GcnMcFileDb(QObject *parent)
	: super(parent)
	, d_ptr(new GcnMcFileDbPrivate(this))
{ }

GcnMcFileDb::~GcnMcFileDb()
{
	delete d_ptr;
}


/**
 * Load a GCN Memory Card File database.
 * @param filename Filename of the database file.
 * @return 0 on success; non-zero on error.
 */
int GcnMcFileDb::load(const QString &filename)
{
	Q_D(GcnMcFileDb);
	return d->load(filename);
}


/**
 * Get the error string.
 * This is set if load() fails.
 * @return Error string.
 */
QString GcnMcFileDb::errorString(void) const
{
	Q_D(const GcnMcFileDb);
	return d->errorString;
}


/**
 * Check a GCN memory card block to see if it matches any search patterns.
 * @param buf	[in] GCN memory card block to check.
 * @param siz	[in] Size of buf. (Should be BLOCK_SIZE == 0x2000.)
 * @return QVector of matches, or empty QVector if no matches were found.
 */
QVector<GcnSearchData> GcnMcFileDb::checkBlock(const void *buf, int siz) const
{
	// File entry matches.
	QVector<GcnSearchData> fileMatches;

	Q_D(const GcnMcFileDb);
	foreach (uint32_t address, d->addr_file_defs.keys()) {
		// Make sure this address is within the bounds of the buffer.
		// Game Description + File Description == 64 bytes. (0x40)
		const int maxAddress = (int)(address + 0x40);
		if (maxAddress < 0 || maxAddress > siz)
			continue;

		// Get the game description and file description.
		const char *const commentData = ((const char*)buf + address);
		const QString gameDescUS = d->GetGcnCommentUtf16(commentData, 32, d->textCodecUS);
		const QString gameDescJP = d->GetGcnCommentUtf16(commentData, 32, d->textCodecJP);
		const QString fileDescUS = d->GetGcnCommentUtf16(commentData+32, 32, d->textCodecUS);
		const QString fileDescJP = d->GetGcnCommentUtf16(commentData+32, 32, d->textCodecJP);

		QVector<GcnMcFileDef*> *vec = d->addr_file_defs.value(address);
		foreach (const GcnMcFileDef *gcnMcFileDef, *vec) {
			// Check if the Game Description (US) matches.
			QRegularExpressionMatch gameDescMatch =
				gcnMcFileDef->search.gameDesc_regex.match(gameDescUS);
			if (!gameDescMatch.hasMatch()) {
				// No match for US.
				// Check if the Game Description (JP) matches.
				gameDescMatch = gcnMcFileDef->search.gameDesc_regex.match(gameDescJP);
				if (!gameDescMatch.hasMatch()) {
					// No match for JP.
					continue;
				}
			}

			// Check if the File Description (US) matches.
			QRegularExpressionMatch fileDescMatch =
				gcnMcFileDef->search.fileDesc_regex.match(fileDescUS);
			if (!fileDescMatch.hasMatch()) {
				// No match for US.
				// Check if the Game Description (JP) matches.
				fileDescMatch = gcnMcFileDef->search.fileDesc_regex.match(fileDescJP);
				if (!fileDescMatch.hasMatch()) {
					// No match for JP.
					continue;
				}
			}

			// Found a match.
			// Attempt to apply variable modifiers.
			QDateTime qDateTime;
			QHash<QString, QString> vars = VarReplace::StringListsToHash(
				gameDescMatch.capturedTexts(), fileDescMatch.capturedTexts());
			int ret = VarReplace::ApplyModifiers(gcnMcFileDef->varModifiers, vars, &qDateTime);
			if (ret == 0) {
				// Variable modifiers applied successfully.
				// Construct a GcnSearchData struct for this file entry.
				fileMatches.append(d->constructSearchData(gcnMcFileDef, vars, qDateTime));
			}
		}
	}

	// Return the matched files.
	return fileMatches;
}


/**
 * Get a list of database files.
 * This function checks various paths for *.xml.
 * If two files with the same filename are found,
 * the one in the higher-precedence directory gets
 * higher precedence.
 * @return List of database files.
 */
QVector<QString> GcnMcFileDb::GetDbFilenames(void)
{
	QVector<QString> pathList;

#ifdef Q_OS_WIN
	// Win32: Search the program's /data/ and main directories.
	pathList.append(QCoreApplication::applicationDirPath() + QLatin1String("/data"));
	pathList.append(QCoreApplication::applicationDirPath());
#else /* !Q_OS_WIN */
	// Check if the program's directory is within the user's home directory.
	bool isPrgDirInHomeDir = false;
	QDir prgDir = QDir(QCoreApplication::applicationDirPath());
	QDir homeDir = QDir::home();

	do {
		if (prgDir == homeDir) {
			isPrgDirInHomeDir = true;
			break;
		}

		prgDir.cdUp();
	} while (!prgDir.isRoot());

	if (isPrgDirInHomeDir) {
		// Program is in the user's home directory.
		// This usually means they're working on it themselves.

		// Search the program's /data/ and main directories.
		pathList.append(QCoreApplication::applicationDirPath() + QLatin1String("/data"));
		pathList.append(QCoreApplication::applicationDirPath());
	}

	// Search the installed data directory.
	pathList.append(QString::fromUtf8(MCRECOVER_DATA_DIRECTORY));
#endif /* Q_OS_WIN */

	// Search the user's configuration directory.
	QDir configDir(ConfigStore::ConfigPath());
	if (configDir != QDir(QCoreApplication::applicationDirPath())) {
		pathList.append(configDir.absoluteFilePath(QLatin1String("data")));
		pathList.append(configDir.absolutePath());
	}

	// Name filters.
	static const char nameFilters_c[8][6] = {
		"*.xml", "*.xmL", "*.xMl", "*.xML",
		"*.Xml", "*.XmL", "*.XMl", "*.XML"
	};

	QStringList nameFilters;
	for (int i = 0; i < 8; i++)
		nameFilters << QLatin1String(nameFilters_c[i]);

	// Search the paths for XML files.
	QVector<QString> xmlFileList;
	xmlFileList.reserve(pathList.size());
	static const QDir::Filters filters = (QDir::Files | QDir::Readable);
#ifdef Q_OS_WIN
	static const QDir::SortFlags sortFlags = (QDir::Name | QDir::IgnoreCase);
#else /* !Q_OS_WIN */
	static const QDir::SortFlags sortFlags = (QDir::Name);
#endif /* Q_OS_WIN */

	foreach (QString path, pathList) {
		QDir dir(path);
		QFileInfoList files = dir.entryInfoList(nameFilters, filters, sortFlags);
		foreach (QFileInfo file, files) {
			xmlFileList.append(file.absoluteFilePath());
		}
	}

	return xmlFileList;
}

/**
 * Add checksum definitions to an open file.
 *
 * NOTE: The file must NOT have checksum definitions before calling
 * this function.
 *
 * @param file GcnFile
 * @return True if definitions were added by this class; false if not.
 */
bool GcnMcFileDb::addChecksumDefs(GcnFile *file) const
{
	assert(file->checksumStatus() == Checksum::CHKST_UNKNOWN);
	if (file->checksumStatus() != Checksum::CHKST_UNKNOWN) {
		// Checksum has already been obtained for this file.
		return true;
	}

	// TODO: Filename regex?

	// GCN file comments: "GameDesc\0FileDesc"
	// If no '\0' is present, this is an error.
	QStringList desc = file->description().split(QChar(L'\0'));
	if (desc.size() != 2) {
		// No '\0' is present.
		// Can't process this file.
		return false;
	}

	const QString &gameDesc = desc[0];
	const QString &fileDesc = desc[1];

	// TODO: QHash<> with the game ID?
	const QString gameID = file->gameID();
	Q_D(const GcnMcFileDb);
	foreach (QVector<GcnMcFileDef*>* vec, d->addr_file_defs) {
		foreach (GcnMcFileDef* gcnMcFileDef, *vec) {
			// Check if this file matches.
			if (gameID != QLatin1String(gcnMcFileDef->id6, sizeof(gcnMcFileDef->id6))) {
				// No match.
				continue;
			}

			// Make sure the GameDesc matches.
			QRegularExpressionMatch gameDescMatch =
				gcnMcFileDef->search.gameDesc_regex.match(gameDesc);
			if (!gameDescMatch.hasMatch()) {
				// Not a match.
				continue;
			}

			// Make sure the FileDesc matches.
			QRegularExpressionMatch fileDescMatch =
				gcnMcFileDef->search.fileDesc_regex.match(fileDesc);
			if (!fileDescMatch.hasMatch()) {
				// Not a match.
				continue;
			}

			// File matches.
			// Copy the checksum definitions.
			file->setChecksumDefs(gcnMcFileDef->checksumDefs);
			return true;
		}
	}

	// File information not found.
	return false;
}
