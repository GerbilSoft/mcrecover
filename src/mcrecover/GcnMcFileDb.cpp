/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * GcnMcFileDb.cpp: GCN Memory Card File Database class.                   *
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
#include <cstdio>
#include <cstring>
#include <cctype>

// GCN Memory Card File Definition class.
#include "GcnMcFileDef.hpp"

// Qt includes.
#include <QtCore/QFile>
#include <QtCore/QMap>
#include <QtCore/QTextCodec>
#include <QtCore/QVector>
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
		int load(QString filename);

		void parseXml_GcnMcFileDb(QXmlStreamReader &xml);
		GcnMcFileDef *parseXml_file(QXmlStreamReader &xml);
		QString parseXml_element(QXmlStreamReader &xml);
		void parseXml_file_search(QXmlStreamReader &xml, GcnMcFileDef *gcnMcFileDef);
		void parseXml_file_dirEntry(QXmlStreamReader &xml, GcnMcFileDef *gcnMcFileDef);
		pcre *compile_regexp(QString regexp);

		/**
		 * Error string.
		 * Set if an error occurs in load().
		 */
		QString errorString;

		// Text codecs.
		QTextCodec *const textCodecJP;
		QTextCodec *const textCodecUS;

		/**
		 * Get a comment from the GCN comment block, converted to UTF-8.
		 * @param buf Comment block.
		 * @param siz Size of comment block. (usually 32)
		 * @param textCodec QTextCodec. (If nullptr, use latin1.)
		 */
		static QByteArray GetGcnCommentUtf8(const char *buf, int siz, QTextCodec *textCodec);

		/**
		 * Check a GCN comment against a regexp.
		 * @param regexp Regexp.
		 * @param descUtf8 GCN comment, converted to UTF-8.
		 * @return Return value from pcre_exec(). (Positive indicates matches; negative indicates error.)
		 */
		static int CheckDescRegexp(pcre *regexp, const QByteArray &descUtf8);
};

GcnMcFileDbPrivate::GcnMcFileDbPrivate(GcnMcFileDb *q)
	: q(q)
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
	     iter != addr_file_defs.end(); iter++)
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
			GcnMcFileDef *gcnMcFileDef = parseXml_file(xml);
			if (gcnMcFileDef) {
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
		return NULL;
	}

	GcnMcFileDef *gcnMcFileDef = new GcnMcFileDef;
	QString regionStr;

	// Iterate over the properties.
	xml.readNext();
	while (!xml.hasError() &&
		!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == myTokenType)) {
		if (xml.tokenType() == QXmlStreamReader::StartElement) {
			// Check what this element is.
			if (xml.name() == QLatin1String("description")) {
				// File description.
				gcnMcFileDef->description = parseXml_element(xml);
			} else if (xml.name() == QLatin1String("gamecode")) {
				// Game code.
				gcnMcFileDef->gamecode = parseXml_element(xml);
			} else if (xml.name() == QLatin1String("company")) {
				// Company code.
				gcnMcFileDef->company = parseXml_element(xml);
			} else if (xml.name() == QLatin1String("regions")) {
				// Additional region codes.
				regionStr += parseXml_element(xml);
			} else if (xml.name() == QLatin1String("search")) {
				// Search definitions.
				parseXml_file_search(xml, gcnMcFileDef);
			} else if (xml.name() == QLatin1String("dirEntry")) {
				// Directory entry.
				parseXml_file_dirEntry(xml, gcnMcFileDef);
			}

			// TODO: File table information.
		}

		// Next token.
		xml.readNext();
	}

	// Determine the main region code from the game code.
	if (gcnMcFileDef->gamecode.length() == 4) {
		// Last character of the game code is the region code.
		QChar regionChr = gcnMcFileDef->gamecode.at(3);
		gcnMcFileDef->regions = RegionCharToBitfield(regionChr);
	} else {
		// TODO: Set an error flag and append a message.
		fprintf(stderr, "WARNING: Game code \"%s\" is invalid.\n",
			gcnMcFileDef->gamecode.toUtf8().constData());
		// Default to USA... (TODO: Maybe "all regions"?)
		gcnMcFileDef->regions = GcnMcFileDef::REGION_USA;
	}

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
				gcnMcFileDef->search.address = address_str.toUInt(NULL, 0);
			} else if (xml.name() == QLatin1String("gameDesc")) {
				// Game description. (regexp)
				gcnMcFileDef->search.gameDesc = parseXml_element(xml);
			} else if (xml.name() == QLatin1String("fileDesc")) {
				// File description. (regexp)
				gcnMcFileDef->search.fileDesc = parseXml_element(xml);
			}
		}

		// Next token.
		xml.readNext();
	}

	// Attempt to compile the regular expressions.
	// TODO: Display errors if compile_regexp() fails.

	// Game Description.
	if (gcnMcFileDef->search.gameDesc_regexp) {
		pcre_free(gcnMcFileDef->search.gameDesc_regexp);
		gcnMcFileDef->search.gameDesc_regexp = NULL;
	}
	gcnMcFileDef->search.gameDesc_regexp = compile_regexp(gcnMcFileDef->search.gameDesc);

	// File Description.
	if (gcnMcFileDef->search.fileDesc_regexp) {
		pcre_free(gcnMcFileDef->search.fileDesc_regexp);
		gcnMcFileDef->search.fileDesc_regexp = NULL;
	}
	gcnMcFileDef->search.fileDesc_regexp = compile_regexp(gcnMcFileDef->search.fileDesc);
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
				gcnMcFileDef->dirEntry.bannerFormat = (uint8_t)str.toUInt(NULL, 0);
			} else if (xml.name() == QLatin1String("iconAddress")) {
				// Icon address.
				str = parseXml_element(xml);
				gcnMcFileDef->dirEntry.iconAddress = str.toUInt(NULL, 0);
			} else if (xml.name() == QLatin1String("iconFormat")) {
				// Icon format.
				str = parseXml_element(xml);
				gcnMcFileDef->dirEntry.iconFormat = str.toUShort(NULL, 0);
			} else if (xml.name() == QLatin1String("iconSpeed")) {
				// Icon speed.
				str = parseXml_element(xml);
				gcnMcFileDef->dirEntry.iconSpeed = str.toUShort(NULL, 0);
			} else if (xml.name() == QLatin1String("permission")) {
				// Permission.
				str = parseXml_element(xml);
				gcnMcFileDef->dirEntry.permission = (uint8_t)str.toUInt(NULL, 0);
			} else if (xml.name() == QLatin1String("length")) {
				// Length, in blocks.
				str = parseXml_element(xml);
				gcnMcFileDef->dirEntry.length = str.toUShort(NULL, 0);
			} else if (xml.name() == QLatin1String("commentAddress")) {
				// Comment address.
				str = parseXml_element(xml);
				gcnMcFileDef->dirEntry.commentAddress = str.toUInt(NULL, 0);
			}
		}

		// Next token.
		xml.readNext();
	}
}


pcre *GcnMcFileDbPrivate::compile_regexp(QString regexp)
{
	if (regexp.isEmpty()) {
		// ERROR: Empty regexp is not allowed here.
		// TODO: Set an error flag and append a message.
		fprintf(stderr, "WARNING: regexp is empty\n");
		return NULL;
	}

	// Convert the regexp to UTF-8.
	QByteArray regexp_utf8 = regexp.toUtf8();

	// Attempt to compile the regexp.
	pcre *re;
	const char *error;
	int erroffset;
	re = pcre_compile(
		regexp_utf8.constData(),	// pattern
		PCRE_UTF8,		// options
		&error,			// error message
		&erroffset,		// error offset
		NULL);			// use default character tables
	if (!re) {
		// Regexp compilation failed.
		fprintf(stderr, "ERROR: Regexp compilation failed.\n- Regexp: %s\n- Error: %s\n- Offset: %d\n",
			regexp_utf8.constData(), error, erroffset);
	}

	return re;
}


/**
 * Get a comment from the GCN comment block, converted to UTF-8.
 * @param buf Comment block.
 * @param siz Size of comment block. (usually 32)
 * @param textCodec QTextCodec. (If nullptr, use latin1.)
 */
QByteArray GcnMcFileDbPrivate::GetGcnCommentUtf8(const char *buf, int siz, QTextCodec *textCodec)
{
	// Remove trialing NULL characters before converting to UTF-8.
	const char *p_nullChr = (const char*)memchr(buf, 0x00, siz);
	if (p_nullChr) {
		// Found a NULL character.
		if (p_nullChr == buf)
			return QByteArray();
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

	// Convert the comment to UTF-8.
	return comment.toUtf8();
}


/**
 * Check a GCN comment against a regexp.
 * @param regexp Regexp.
 * @param descUtf8 GCN comment, converted to UTF-8.
 * @return Return value from pcre_exec(). (Positive indicates matches; negative indicates error.)
 */
int GcnMcFileDbPrivate::CheckDescRegexp(pcre *regexp, const QByteArray &descUtf8)
{
	int ovector[60];
	int rc = pcre_exec(
		regexp,					// compiled regexp
		NULL,					// pattern not studied
		descUtf8.constData(),			// subject string
		descUtf8.size(),			// size of subject string
		0,					// start at offset 0 in the subject
		0,					// default options
		ovector,				// vector of integers for substring information
		sizeof(ovector)/sizeof(ovector[0]));	// number of elements in ovector

	// TODO: QVector with substring matches.
	return rc;
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
QString GcnMcFileDb::errorString(void) const
{
	return d->errorString;
}


/**
 * Check a GCN memory card block to see if it matches any search patterns.
 * @param buf		[in] GCN memory card block to check.
 * @param siz		[in] Size of buf. (Should be BLOCK_SIZE == 0x2000.)
 * @param dirEntry	[out] Constructed directory entry if a pattern matched.
 * @return 0 if a pattern was matched; non-zero if not.
 */
int GcnMcFileDb::checkBlock(const void *buf, int siz, card_direntry *dirEntry) const
{
	// TODO: Return a list of FAT entries.
	// (May require more info from MemCard, and might need to be in
	// another function somewhere else.)

	// Matching file definition.
	const GcnMcFileDef *matchFileDef = NULL;

	foreach (uint32_t address, d->addr_file_defs.keys()) {
		// Make sure this address is within the bounds of the buffer.
		// Game Description + File Description == 64 bytes. (0x40)
		const int maxAddress = (int)(address + 0x40);
		if (maxAddress < 0 || maxAddress > siz)
			continue;

		// Get the game description and file description.
		const char *commentData = ((const char*)buf + address);
		QByteArray gameDescUS = d->GetGcnCommentUtf8(commentData, 32, d->textCodecUS);
		QByteArray gameDescJP = d->GetGcnCommentUtf8(commentData, 32, d->textCodecJP);
		QByteArray fileDescUS = d->GetGcnCommentUtf8(commentData+32, 32, d->textCodecUS);
		QByteArray fileDescJP = d->GetGcnCommentUtf8(commentData+32, 32, d->textCodecJP);

		QVector<GcnMcFileDef*> *vec = d->addr_file_defs.value(address);
		foreach (const GcnMcFileDef *gcnMcFileDef, *vec) {
			bool gameDescMatch = false, fileDescMatch = false;
			int rc;

			// TODO: Save substring matches.

			// Check if the Game Description (US) matches.
			rc = d->CheckDescRegexp(
				gcnMcFileDef->search.gameDesc_regexp,
				gameDescUS);
			if (rc > 0) {
				gameDescMatch = true;
			} else {
				// Check if the Game Description (JP) matches.
				rc = d->CheckDescRegexp(
					gcnMcFileDef->search.gameDesc_regexp,
					gameDescJP);
				if (rc > 0)
					gameDescMatch = true;
			}

			// Check if the File Description (US) matches.
			rc = d->CheckDescRegexp(
				gcnMcFileDef->search.fileDesc_regexp,
				fileDescUS);
			if (rc > 0) {
				fileDescMatch = true;
			} else {
				// Check if the File Description (JP) matches.
				rc = d->CheckDescRegexp(
					gcnMcFileDef->search.fileDesc_regexp,
					fileDescJP);
				if (rc > 0)
					fileDescMatch = true;
			}

			if (gameDescMatch && fileDescMatch) {
				// Found a match.
				matchFileDef = gcnMcFileDef;
				break;
			}
		}

		if (matchFileDef)
			break;
	}

	if (!matchFileDef) {
		// No match.
		return -1;
	}

	// Construct the directory entry for this file.
	memset(dirEntry, 0x00, sizeof(*dirEntry));
	QByteArray ba;

	// Game code.
	ba = matchFileDef->gamecode.toLatin1();
	strncpy(dirEntry->gamecode, ba.constData(), sizeof(dirEntry->gamecode));

	// Company code.
	ba = matchFileDef->company.toLatin1();
	strncpy(dirEntry->company, ba.constData(), sizeof(dirEntry->company));

	// Clear the byte array before converting the filename.
	ba = QByteArray();

	// Filename.
	if (dirEntry->gamecode[3] == 'J' && d->textCodecJP) {
		// JP file. Convert to Shift-JIS.
		ba = d->textCodecJP->fromUnicode(matchFileDef->dirEntry.filename);
	} else if (d->textCodecUS) {
		// US/EU file. Convert to cp1252.
		ba = d->textCodecUS->fromUnicode(matchFileDef->dirEntry.filename);
	}

	if (ba.isEmpty()) {
		// QByteArray is empty. Conversion failed.
		// Convert to Latin1 instead.
		ba = matchFileDef->dirEntry.filename.toLatin1();
	}

	if (ba.length() > (int)sizeof(dirEntry->filename))
		ba.resize(sizeof(dirEntry->filename));
	strncpy(dirEntry->filename, ba.constData(), sizeof(dirEntry->filename));
	// TODO: Make sure the filename is null-terminated?

	// Values.
	/**
	 * TODO:
	 * - Construct a proper timestamp.
	 * - Use the actual starting block?
	 * - Block offsets for files with commentaddr >= 0x2000
	 * - Support for variable-length files?
	 */
	dirEntry->pad_00	= 0xFF;
	dirEntry->bannerfmt	= matchFileDef->dirEntry.bannerFormat;
	dirEntry->lastmodified	= 0;
	dirEntry->iconaddr	= matchFileDef->dirEntry.iconAddress;
	dirEntry->iconfmt	= matchFileDef->dirEntry.iconFormat;
	dirEntry->iconspeed	= matchFileDef->dirEntry.iconSpeed;
	dirEntry->permission	= matchFileDef->dirEntry.permission;
	dirEntry->copytimes	= 0;
	dirEntry->block		= 5;	// FIXME
	dirEntry->length	= matchFileDef->dirEntry.length;
	dirEntry->pad_01	= 0xFFFF;
	dirEntry->commentaddr	= matchFileDef->dirEntry.commentAddress;

	// Directory entry matched.
	return 0;
}
