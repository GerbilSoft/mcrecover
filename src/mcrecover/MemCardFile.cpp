/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * MemCardFile.cpp: Memory Card file entry class.                          *
 *                                                                         *
 * Copyright (c) 2012-2013 by David Korth.                                 *
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

#include "MemCardFile.hpp"
#include "card.h"
#include "byteswap.h"

// MemCard class.
#include "MemCard.hpp"

// GcImage class.
#include "GcImage.hpp"

// Qt includes.
#include <QtCore/QByteArray>
#include <QtCore/QTextCodec>
#include <QtCore/QFile>
#include <QtCore/QIODevice>

/** MemCardFilePrivate **/

class MemCardFilePrivate
{
	public:
		/**
		 * Initialize the MemCardFile private class.
		 * This constructor is for valid files.
		 * @param q MemCardFile.
		 * @param card MemCard.
		 * @param fileIdx File index in MemCard.
		 * @param dat Directory table.
		 * @param bat Block allocation table.
		 */
		MemCardFilePrivate(MemCardFile *q, 
				MemCard *card, const int fileIdx,
				const card_dat *dat, const card_bat *bat);

		/**
		 * Initialize the MemCardFile private class.
		 * This constructor is for "lost" files.
		 * @param q MemCardFile.
		 * @param card MemCard.
		 * @param dirEntry Constructed directory entry.
		 * @param fatEntries FAT entries.
		 */
		MemCardFilePrivate(MemCardFile *q,
				MemCard *card,
				const card_direntry *dirEntry,
				QVector<uint16_t> fatEntries);

		~MemCardFilePrivate();

	private:
		MemCardFile *const q;
		Q_DISABLE_COPY(MemCardFilePrivate);

		/**
		* Common initialization code.
		*/
		void init(void);

	public:
		MemCard *const card;

		// Card directory information.
		const int fileIdx;		// If -1, this is a lost file.
		const card_dat *const dat;	// If nullptr, this is a lost file.
		const card_bat *const bat;	// If nullptr, this is a lost file.

		/**
		 * Directory entry.
		 * This points to an entry within dat.
		 * NOTE: If this is a lost file, this was allocated by us,
		 * and needs to be freed in the destructor.
		 */
		const card_direntry *dirEntry;

		// FAT entries.
		QVector<uint16_t> fatEntries;

		// File information. (Directory table.)
		QString gamecode;
		QString company;
		QString filename;
		GcnDateTime lastModified;	// Last modified time.

		// File information. (Comment, banner, icon)
		QString gameDesc;	// Game description.
		QString fileDesc;	// File description.

		// Images.
		QPixmap banner;
		QVector<QPixmap> icons;

		/**
		 * Get the file size, in blocks.
		 * @return File size, in blocks.
		 */
		uint16_t size(void) const;

		/**
		 * Convert a file block number to a physical block number.
		 * @param fileBlock File block number.
		 * @return Physical block number, or negative on error.
		 */
		uint16_t fileBlockAddrToPhysBlockAddr(uint16_t fileBlock);

		/**
		 * Load file data.
		 * @return QByteArray with file data, or empty QByteArray on error.
		 */
		QByteArray loadFileData(void);

		// CI8 SHARED image struct.
		struct CI8_SHARED_data {
			int iconIdx;
			uint32_t iconAddr;
		};

		/**
		 * Load the banner and icon images.
		 */
		void loadImages(void);

		// Checksum data.
		QVector<Checksum::ChecksumDef> checksumDefs;
		QVector<Checksum::ChecksumValue> checksumValues;

		/**
		 * Calculate the file checksum.
		 */
		void calculateChecksum(void);

		/**
		 * Strip invalid DOS characters from a filename.
		 * @param filename Filename.
		 * @param replaceChar Replacement character.
		 * @return Filename with invalid DOS characters replaced with replaceChar.
		 */
		static QString StripInvalidDosChars(QString filename, QChar replaceChar = QChar(L'_'));
};


/**
 * Initialize the MemCardFile private class.
 * This constructor is for valid files.
 * @param q MemCardFile.
 * @param card MemCard.
 * @param fileIdx File index in MemCard.
 * @param dat Directory table.
 * @param bat Block allocation table.
 */
MemCardFilePrivate::MemCardFilePrivate(MemCardFile *q,
		MemCard *card, const int fileIdx,
		const card_dat *dat, const card_bat *bat)
	: q(q)
	, card(card)
	, fileIdx(fileIdx)
	, dat(dat)
	, bat(bat)
	, banner(QPixmap())
{
	// Load the directory table information.
	dirEntry = &dat->entries[fileIdx];

	// Clamp file length to the size of the memory card.
	// This shouldn't happen, but it's possible if either
	// the filesystem is heavily corrupted, or the file
	// isn't actually a GCN Memory Card image.
	int length = dirEntry->length;
	if (length > card->sizeInBlocksNoSys())
		length = card->sizeInBlocksNoSys();

	// Load the FAT entries.
	fatEntries.clear();
	fatEntries.reserve(length);
	uint16_t last_block = dirEntry->block;
	if (last_block >= 5 && last_block != 0xFFFF) {
		fatEntries.append(last_block);
		for (int i = length; i > 1; i--) {
			last_block = bat->fat[last_block - 5];
			if (last_block == 0xFFFF || last_block < 5)
				break;
			fatEntries.append(last_block);
		}
	}

	// Populate the rest of the fields.
	init();
}


/**
 * Initialize the MemCardFile private class.
 * This constructor is for "lost" files.
 * @param q MemCardFile.
 * @param card MemCard.
 * @param dirEntry Constructed directory entry.
 * @param fatEntries FAT entries.
 */
MemCardFilePrivate::MemCardFilePrivate(MemCardFile *q,
		MemCard *card, const card_direntry *dirEntry,
		QVector<uint16_t> fatEntries)
	: q(q)
	, card(card)
	, fileIdx(-1)
	, dat(NULL)
	, bat(NULL)
	, banner(QPixmap())
{
	// Take a copy of the constructed directory entry.
	card_direntry *dentry = (card_direntry*)malloc(sizeof(*dirEntry));
	memcpy(dentry, dirEntry, sizeof(*dentry));
	this->dirEntry = dentry;

	// Copy the FAT entries.
	this->fatEntries = fatEntries;

	// Populate the rest of the fields.
	init();
}


MemCardFilePrivate::~MemCardFilePrivate()
{
	if (fileIdx < 0) {
		// dirEntry was allocated by us.
		// Free it.
		free((void*)dirEntry);
	}
}


/**
 * Common initialization code.
 */
void MemCardFilePrivate::init(void)
{
	// TODO: Should filenames be converted from Shift-JIS, or always use Latin-1?
	filename = QString::fromLatin1(dirEntry->filename, sizeof(dirEntry->filename));
	int nullChr = filename.indexOf(QChar(L'\0'));
	if (nullChr >= 0)
		filename.resize(nullChr);

	// Game Code and Company are always Latin-1.
	gamecode = QString::fromLatin1(dirEntry->gamecode, sizeof(dirEntry->gamecode));
	company = QString::fromLatin1(dirEntry->company, sizeof(dirEntry->company));

	// Timestamp.Handle the timestamp as UTC.
	lastModified.setGcnTimestamp(dirEntry->lastmodified);

	// Get the block size.
	const int blockSize = card->blockSize();

	// Load the block with the comments.
	const int commentBlock = (dirEntry->commentaddr / blockSize);
	const int commentOffset = (dirEntry->commentaddr % blockSize);

	char *commentData = (char*)malloc(blockSize);
	card->readBlock(commentData, blockSize, fileBlockAddrToPhysBlockAddr(commentBlock));

	// Load the file comments. (64 bytes)
	// NOTE: These comments are supposed to be NULL-terminated.
	// 0x00: Game description.
	// 0x20: File description.
	QByteArray gameDescData(&commentData[commentOffset], 32);
	QByteArray fileDescData(&commentData[commentOffset+32], 32);
	free(commentData);

	// Remove trailing NULL characters before converting to UTF-8.
	nullChr = gameDescData.indexOf('\0');
	if (nullChr >= 0)
		gameDescData.resize(nullChr);
	nullChr = fileDescData.indexOf('\0');
	if (nullChr >= 0)
		fileDescData.resize(nullChr);

	// Convert the descriptions to UTF-8.
	// Trim the descriptions while we're at it.
	QTextCodec *textCodec = card->textCodec();
	if (!textCodec) {
		// No text codec was found.
		// Default to Latin-1.
		gameDesc = QString::fromLatin1(gameDescData.constData(), gameDescData.size()).trimmed();
		fileDesc = QString::fromLatin1(fileDescData.constData(), fileDescData.size()).trimmed();
	} else {
		// Use the text codec.
		gameDesc = textCodec->toUnicode(gameDescData.constData(), gameDescData.size()).trimmed();
		fileDesc = textCodec->toUnicode(fileDescData.constData(), fileDescData.size()).trimmed();
	}

	// Load the banner and icon images.
	loadImages();
}


/**
 * Get the file size, in blocks.
 * @return File size, in blocks.
 */
uint16_t MemCardFilePrivate::size(void) const
{
	// NOTE: We're using fatEntries.size() instead of
	// dirEntry.length, since the directory entry may
	// contain invalid data.
	return fatEntries.size();
}


/**
 * Convert a file block number to a physical block number.
 * @param fileBlock File block number.
 * @return Physical block number, or negative on error.
 */
uint16_t MemCardFilePrivate::fileBlockAddrToPhysBlockAddr(uint16_t fileBlock)
{
	if ((int)fileBlock >= fatEntries.size())
		return -1;
	return fatEntries.at((int)fileBlock);
}


/**
 * Load file data.
 * @return QByteArray with file data, or empty QByteArray on error.
 */
QByteArray MemCardFilePrivate::loadFileData(void)
{
	const int blockSize = card->blockSize();
	if (this->size() > card->sizeInBlocksNoSys()) {
		// File is larger than the card.
		// This shouldn't happen...
		return QByteArray();
	}

	QByteArray fileData;
	fileData.resize(this->size() * blockSize);

	uint8_t *fileDataPtr = (uint8_t*)fileData.data();
	for (int i = 0; i < this->size(); i++, fileDataPtr += blockSize) {
		const uint16_t physBlockAddr = fileBlockAddrToPhysBlockAddr(i);
		card->readBlock(fileDataPtr, blockSize, physBlockAddr);
	}
	return fileData;
}


/**
 * Load the banner and icon images.
 */
void MemCardFilePrivate::loadImages(void)
{
	// Load the file.
	QByteArray fileData = loadFileData();
	if (fileData.isEmpty())
		return;
	
	// Decode the banner.
	uint32_t iconAddr = dirEntry->iconaddr;
	uint32_t imageSize = 0;
	QImage bannerImg;
	
	switch (dirEntry->bannerfmt & CARD_BANNER_MASK) {
		case CARD_BANNER_CI:
			// CI8 palette is right after the banner.
			// (256 entries in RGB5A3 format.)
			imageSize = (CARD_BANNER_W * CARD_BANNER_H * 1);
			if ((int)(iconAddr + imageSize) > fileData.size())
				break;
			bannerImg = GcImage::FromCI8(CARD_BANNER_W, CARD_BANNER_H,
					&fileData.constData()[iconAddr], imageSize,
					&fileData.constData()[iconAddr + imageSize], 0x200);
			iconAddr += imageSize + 0x200;
			break;

		case CARD_BANNER_RGB:
			imageSize = (CARD_BANNER_W * CARD_BANNER_H * 2);
			if ((int)(iconAddr + imageSize) > fileData.size())
				break;
			bannerImg = GcImage::FromRGB5A3(CARD_BANNER_W, CARD_BANNER_H,
					&fileData.constData()[iconAddr], imageSize);
			iconAddr += imageSize;
			break;

		default:
			break;
	}

	if (!bannerImg.isNull()) {
		// Set the new banner image.
		banner = QPixmap::fromImage(bannerImg);
	} else {
		// No banner image.
		banner = QPixmap();
	}

	// Decode the icon(s).
	QVector<CI8_SHARED_data> lst_CI8_SHARED;
	icons.clear();

	uint16_t iconfmt = dirEntry->iconfmt;
	for (int i = 0; i < CARD_MAXICONS; i++) {
		if ((iconfmt & CARD_ICON_MASK) == CARD_ICON_CI_SHARED) {
			// CI8 palette is after *all* the icons.
			// (256 entries in RGB5A3 format.)
			// This is handled after the rest of the icons.
			CI8_SHARED_data data;
			data.iconIdx = i;
			data.iconAddr = iconAddr;
			lst_CI8_SHARED.append(data);

			// Add a NULL QPixmap as a placeholder.
			icons.append(QPixmap());

			// Next icon.
			imageSize = (CARD_ICON_W * CARD_ICON_H * 1);
			iconAddr += imageSize;
			iconfmt >>= 2;
			continue;
		}

		switch (iconfmt & CARD_ICON_MASK) {
			case CARD_ICON_CI_UNIQUE: {
				// CI8 palette is right after the icon.
				// (256 entries in RGB5A3 format.)
				imageSize = (CARD_ICON_W * CARD_ICON_H * 1);
				if ((int)(iconAddr + imageSize + 0x200) > fileData.size())
					break;
				QImage icon = GcImage::FromCI8(CARD_ICON_W, CARD_ICON_H,
						&fileData.constData()[iconAddr], imageSize,
						&fileData.constData()[iconAddr + imageSize], 0x200);
				icons.append(QPixmap::fromImage(icon));
				iconAddr += imageSize + 0x200;
				break;
			}

			case CARD_BANNER_RGB: {
				imageSize = (CARD_ICON_W * CARD_ICON_H * 2);
				if ((int)(iconAddr + imageSize) > fileData.size())
					break;
				QImage icon = GcImage::FromRGB5A3(CARD_ICON_W, CARD_ICON_H,
						&fileData.constData()[iconAddr], imageSize);
				icons.append(QPixmap::fromImage(icon));
				iconAddr += imageSize;
				break;
			}

			default:
				// No icon.
				// Add a NULL image as a placeholder.
				icons.append(QPixmap());
				break;
		}

		// Next icon.
		iconfmt >>= 2;
	}

	if (!lst_CI8_SHARED.isEmpty()) {
		// Process CI8 SHARED icons.
		// TODO: Convert the palette once instead of every time?
		if ((int)(iconAddr + 0x200) > fileData.size()) {
			// Out of bounds.
			// Delete the NULL icons.
			for (int i = (icons.size() - 1); i >= 0; i--) {
				if (icons.at(i).isNull())
					icons.remove(i);
			}
		} else {
			// Process each icon.
			foreach (const CI8_SHARED_data& data, lst_CI8_SHARED) {
				QImage icon = GcImage::FromCI8(CARD_ICON_W, CARD_ICON_H,
							&fileData.constData()[data.iconAddr], imageSize,
							&fileData.constData()[iconAddr], 0x200);
				icons[data.iconIdx] = QPixmap::fromImage(icon);
			}
		}
	}
}


/**
 * Calculate the file checksum.
 */
void MemCardFilePrivate::calculateChecksum(void)
{
	checksumValues.clear();

	if (checksumDefs.isEmpty()) {
		// No checksum definitions were set.
		return;
	}

	// Load the file data.
	QByteArray fileData = loadFileData();
	if (fileData.isEmpty()) {
		// File is empty.
		return;
	}

	// Pointer to fileData's internal data array.
	uint8_t *data = reinterpret_cast<uint8_t*>(fileData.data());

	// Process all of the checksum definitions.
	for (int i = 0; i < checksumDefs.count(); i++) {
		const Checksum::ChecksumDef &checksumDef = checksumDefs.at(i);

		if (checksumDef.algorithm == Checksum::CHKALG_NONE ||
		    checksumDef.algorithm >= Checksum::CHKALG_MAX ||
		    checksumDef.length == 0)
		{
			// No algorithm or invalid algorithm set,
			// or the checksum data has no length.
			continue;
		}

		// Make sure the checksum definition is in range.
		if (fileData.size() < (int)checksumDef.address ||
		    fileData.size() < (int)(checksumDef.start + checksumDef.length))
		{
			// File is too small...
			// TODO: Also check the size of the checksum itself.
			continue;
		}

		// Get the expected checksum.
		// NOTE: Assuming big-endian for all values.
		uint32_t expected = 0;
		Checksum::ChaoGardenChecksumData chaoChk_orig;

		switch (checksumDef.algorithm) {
			case Checksum::CHKALG_CRC16:
				expected = (data[checksumDef.address+0] << 8) |
					   (data[checksumDef.address+1]);
				break;

			case Checksum::CHKALG_CRC32:
			case Checksum::CHKALG_ADDINVDUAL16:
			case Checksum::CHKALG_ADDBYTES32:
				expected = (data[checksumDef.address+0] << 24) |
					   (data[checksumDef.address+1] << 16) |
					   (data[checksumDef.address+2] << 8) |
					   (data[checksumDef.address+3]);
				break;

			case Checksum::CHKALG_SONICCHAOGARDEN: {
				memcpy(&chaoChk_orig, &data[checksumDef.address], sizeof(chaoChk_orig));

				// Temporary working copy.
				Checksum::ChaoGardenChecksumData chaoChk = chaoChk_orig;
				expected = (chaoChk.checksum_3 << 24) |
					   (chaoChk.checksum_2 << 16) |
					   (chaoChk.checksum_1 << 8) |
					   (chaoChk.checksum_0);

				// Clear some fields that must be 0 when calculating the checksum.
				chaoChk.checksum_3 = 0;
				chaoChk.checksum_2 = 0;
				chaoChk.checksum_1 = 0;
				chaoChk.checksum_0 = 0;
				chaoChk.random_3 = 0;
				memcpy(&data[checksumDef.address], &chaoChk, sizeof(chaoChk));
				break;
			}

			case Checksum::CHKALG_NONE:
			default:
				// Unsupported algorithm.
				expected = 0;
				break;
		}

		// Calculate the checksum.
		const char *const start = (fileData.constData() + checksumDef.start);
		uint32_t actual = Checksum::Exec(checksumDef.algorithm,
				start, checksumDef.length, checksumDef.param);

		if (checksumDef.algorithm == Checksum::CHKALG_SONICCHAOGARDEN) {
			// Restore the Chao Garden checksum data.
			memcpy(&data[checksumDef.address], &chaoChk_orig, sizeof(chaoChk_orig));
		}

		// Save the checksums.
		Checksum::ChecksumValue checksumValue;
		checksumValue.expected = expected;
		checksumValue.actual = actual;
		checksumValues.append(checksumValue);
	}
}


/**
 * Strip invalid DOS characters from a filename.
 * @param filename Filename.
 * @param replaceChar Replacement character.
 * @return Filename with invalid DOS characters replaced with replaceChar.
 */
QString MemCardFilePrivate::StripInvalidDosChars(
				const QString filename,
				const QChar replaceChar)
{
	QString ret(filename);
	for (int i = (ret.size() - 1); i > 0; i--) {
		QCharRef chr = ret[i];

		// Reference: http://en.wikipedia.org/wiki/8.3_filename#Directory_table
		switch (chr.unicode()) {
			case '"':
			case '*':
			case '/':
			case ':':
			case '<':
			case '>':
			case '?':
			case '\\':
			case '[':
			case ']':
			case '|':
				// Invalid DOS character.
				// (Technically, '[' and ']' are legal on Win32,
				//  but we'll exclude them anyway.)
				chr = replaceChar;
				break;

			default:
				// Valid character.
				break;
		}
	}

	// Return the adjusted filename.
	return ret;
}


/** MemCardFile **/

/**
 * Create a MemCardFile for a MemCard.
 * This constructor is for valid files.
 * @param card MemCard.
 * @param fileIdx File index in MemCard.
 * @param dat Directory table.
 * @param bat Block allocation table.
 */
MemCardFile::MemCardFile(MemCard *card, const int fileIdx,
			const card_dat *dat, const card_bat *bat)
	: QObject(card)
	, d(new MemCardFilePrivate(this, card, fileIdx, dat, bat))
{ }

/**
 * Create a MemCardFile for a MemCard.
 * This constructor is for "lost" files.
 * @param card MemCard.
 * @param dirEntry Constructed directory entry.
 * @param fatEntries FAT entries.
 */
MemCardFile::MemCardFile(MemCard *card,
		const card_direntry *dirEntry,
		QVector<uint16_t> fatEntries)
	: QObject(card)
	, d(new MemCardFilePrivate(this, card, dirEntry, fatEntries))
{ }

MemCardFile::~MemCardFile()
	{ delete d; }


/**
 * Get the game code.
 * @return Game code.
 */
QString MemCardFile::gamecode(void) const
	{ return d->gamecode; }


/**
 * Get the company code.
 * @return Company code.
 */
QString MemCardFile::company(void) const
	{ return d->company; }


/**
 * Get the GCN filename.
 * @return GCN filename.
 */
QString MemCardFile::filename(void) const
	{ return d->filename; }

/**
 * Get the last modified time.
 * @return Last modified time.
 */
GcnDateTime MemCardFile::lastModified(void) const
	{ return d->lastModified; }

/**
 * Get the game description. ("Comments" field.)
 * @return Game description.
 */
QString MemCardFile::gameDesc(void) const
	{ return d->gameDesc; }

/**
 * Get the file description. ("Comments" field.)
 * @return File description.
 */
QString MemCardFile::fileDesc(void) const
	{ return d->fileDesc; }

/**
 * Get the file permissions.
 * @return File permissions.
 */
uint8_t MemCardFile::permission(void) const
	{ return d->dirEntry->permission; }

/**
 * Get the file permissions as a string.
 * @return File permission string.
 */
QString MemCardFile::permissionAsString(void) const
{
	char str[4];

	uint8_t permission = d->dirEntry->permission;
	str[0] = ((permission & CARD_ATTRIB_GLOBAL) ? 'G' : '-');
	str[1] = ((permission & CARD_ATTRIB_NOMOVE) ? 'M' : '-');
	str[2] = ((permission & CARD_ATTRIB_NOCOPY) ? 'C' : '-');
	str[3] = ((permission & CARD_ATTRIB_PUBLIC) ? 'P' : '-');

	return QString::fromLatin1(str, sizeof(str));
}

/**
 * Get the size, in blocks.
 * @return Size, in blocks.
 */
uint16_t MemCardFile::size(void) const
	{ return d->size(); }

/**
 * Get the banner image.
 * @return Banner image, or null QPixmap on error.
 */
QPixmap MemCardFile::banner(void) const
	{ return d->banner; }

/**
 * Get the number of icons in the file.
 * @return Number of icons.
 */
int MemCardFile::numIcons(void) const
	{ return d->icons.size(); }

/**
 * Get an icon from the file.
 * @param idx Icon number.
 * @return Icon, or null QPixmap on error.
 */
QPixmap MemCardFile::icon(int idx) const
{
	if (idx < 0 || idx >= d->icons.size())
		return QPixmap();
	return d->icons.at(idx);
}

/**
 * Get the delay for a given icon.
 * @param idx Icon number.
 * @return Icon delay.
 */
int MemCardFile::iconDelay(int idx) const
{
	if (idx < 0 || idx >= d->icons.size())
		return CARD_SPEED_END;
	return ((d->dirEntry->iconspeed >> (idx * 2)) & CARD_SPEED_MASK);
}

/**
 * Get the icon animation mode.
 * @return Icon animation mode.
 */
int MemCardFile::iconAnimMode(void) const
	{ return (d->dirEntry->bannerfmt & CARD_ANIM_MASK); }

/**
 * Is this a lost file?
 * @return True if lost; false if file is in the directory table.
 */
bool MemCardFile::isLostFile(void) const
	{ return (d->fileIdx < 0); }

/**
 * Get this file's FAT entries.
 * @return FAT entries.
 */
QVector<uint16_t> MemCardFile::fatEntries(void) const
	{ return d->fatEntries; }

/**
 * Get the checksum definitions.
 * @return Checksum definitions.
 */
QVector<Checksum::ChecksumDef> MemCardFile::checksumDefs(void) const
	{ return d->checksumDefs; }

/**
 * Set the checksum definitions.
 * @param checksumDefs Checksum definitions.
 */
void MemCardFile::setChecksumDefs(QVector<Checksum::ChecksumDef> checksumDefs)
{
	d->checksumDefs = checksumDefs;
	d->calculateChecksum();
}

/**
 * Get the checksum values.
 * @return Checksum values, or empty QVector if no checksum definitions were set.
 */
QVector<Checksum::ChecksumValue> MemCardFile::checksumValues(void) const
	{ return d->checksumValues; }

/**
 * Get the checksum algorithm.
 * NOTE: We're assuming each file only uses one algorithm...
 * @return Checksum algorithm.
 */
Checksum::ChkAlgorithm MemCardFile::checksumAlgorithm(void) const
{
	if (d->checksumDefs.isEmpty())
		return Checksum::CHKALG_NONE;
	return d->checksumDefs.at(0).algorithm;
}

/**
 * Get the checksum status.
 * @return Checksum status.
 */
Checksum::ChkStatus MemCardFile::checksumStatus(void) const
	{ return Checksum::ChecksumStatus(d->checksumValues); }

/**
 * Format checksum values as HTML for display purposes.
 * @return QVector containing one or two HTML strings.
 * - String 0 contains the actual checksums.
 * - String 1, if present, contains the expected checksums.
 */
QVector<QString> MemCardFile::checksumValuesFormatted(void) const
	{ return Checksum::ChecksumValuesFormatted(d->checksumValues); }


/**
 * Get the default GCI filename.
 * @return Default GCI filename.
 */
QString MemCardFile::defaultGciFilename(void) const
{
	/**
	 * Filename format:
	 * GALE01_SuperSmashBros0110290334_066000.gci
	 * gamecode_filename_startaddress.gci
	 */
	QString filename;
	filename.reserve(d->gamecode.size() + d->company.size() + 1 +
			 d->filename.size() + 1 + 6 + 4);
	filename += d->gamecode + d->company + QChar(L'_');
	filename += d->filename + QChar(L'_');

	// Convert the start address to hexadecimal.
	char start_address[16];
	snprintf(start_address, sizeof(start_address), "%06X",
		 d->dirEntry->block * d->card->blockSize());
	filename += QLatin1String(start_address);

	// Finish the filename.
	filename += QLatin1String(".gci");

	// Make sure no invalid DOS characters are present in the filename.
	return d->StripInvalidDosChars(filename);
}


/**
 * Save the file.
 * @param filename Filename for the GCI file.
 * @return 0 on success; non-zero on error.
 * TODO: Error code constants.
 */
int MemCardFile::saveGci(QString filename)
{
	QFile file(filename);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
		// Error opening the file.
		return -1;
	}

	// Write the GCI data.
	int ret = saveGci(&file);
	file.close();

	if (ret != 0) {
		// Error saving the GCI file.
		file.remove();
	}

	return ret;
}


/**
 * Save the file.
 * @param qioDevice QIODevice to write the GCI data to.
 * @return 0 on success; non-zero on error.
 * TODO: Error code constants.
 */
int MemCardFile::saveGci(QIODevice *qioDevice)
{
	// GCI header is the 64-byte directory entry.
	// NOTE: This must be byteswapped!
	card_direntry dirEntry = *d->dirEntry;
	dirEntry.lastmodified	= cpu_to_be32(dirEntry.lastmodified);
	dirEntry.iconaddr	= cpu_to_be32(dirEntry.iconaddr);
	dirEntry.iconfmt	= cpu_to_be16(dirEntry.iconfmt);
	dirEntry.iconspeed	= cpu_to_be16(dirEntry.iconspeed);
	dirEntry.block		= cpu_to_be16(dirEntry.block);
	dirEntry.length		= cpu_to_be16(dirEntry.length);
	dirEntry.commentaddr	= cpu_to_be32(dirEntry.commentaddr);

	// Write the directory entry.
	qint64 ret = qioDevice->write((char*)&dirEntry, (qint64)sizeof(dirEntry));
	if (ret != (qint64)sizeof(dirEntry)) {
		// Error saving the directory entry.
		return -2;
	}

	// Write the file data.
	const QByteArray fileData = d->loadFileData();
	ret = qioDevice->write(fileData);
	if (ret != (qint64)fileData.size()) {
		// Error saving the file data.
		return -3;
	}

	// Finished saving the file.
	return 0;
}
