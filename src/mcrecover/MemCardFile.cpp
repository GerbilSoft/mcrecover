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
#include "util/byteswap.h"

// MemCard class.
#include "MemCard.hpp"

// GcImage class.
#include "GcImage.hpp"
#include "GcToolsQt.hpp"

// C++ includes.
#include <string>
#include <vector>
using std::string;
using std::vector;

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
				const QVector<uint16_t> &fatEntries);

		~MemCardFilePrivate();

	protected:
		MemCardFile *const q_ptr;
		Q_DECLARE_PUBLIC(MemCardFile)
	private:
		Q_DISABLE_COPY(MemCardFilePrivate)

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

		// GcImages.
		GcImage *gcBanner;
		QVector<GcImage*> gcIcons;

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

		/**
		 * Read the specified range from the file.
		 * @param blockStart First block.
		 * @param len Length, in blocks.
		 * @return QByteArray with file data, or empty QByteArray on error.
		 */
		QByteArray readBlocks(uint16_t blockStart, int len);

		// CI8 SHARED image struct.
		struct CI8_SHARED_data {
			int iconIdx;
			uint32_t iconAddr;
		};

		/**
		 * Determine if an address + length is within fileSize.
		 * @param address Address.
		 * @param length Length.
		 * @param fileSize File size.
		 * @return True if the address + length is within fileSize; false if not.
		 */
		static bool IsInFile(uint32_t address, uint32_t length, uint32_t fileSize);

		/**
		 * Load the banner image.
		 * @return GcImage containing the banner image, or nullptr on error.
		 */
		GcImage *loadBannerImage(void);

		/**
		 * Load the icon images.
		 * @return QVector<GcImage*> containing the icon images, or empty QVector on error.
		 */
		QVector<GcImage*> loadIconImages(void);

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
	: q_ptr(q)
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
		const QVector<uint16_t> &fatEntries)
	: q_ptr(q)
	, card(card)
	, fileIdx(-1)
	, dat(nullptr)
	, bat(nullptr)
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

	// Delete GcImages.
	delete gcBanner;
	qDeleteAll(gcIcons);
	gcIcons.clear();
}

/**
 * Common initialization code.
 */
void MemCardFilePrivate::init(void)
{
	// Game Code and Company are always Latin-1.
	gamecode = QString::fromLatin1(dirEntry->gamecode, sizeof(dirEntry->gamecode));
	company = QString::fromLatin1(dirEntry->company, sizeof(dirEntry->company));

	// Get the appropriate QTextCodec for this file.
	// TODO: If 0, default to card codec?
	const char region = (gamecode.size() >= 4
				? gamecode.at(3).toLatin1()
				: 0);
	QTextCodec *textCodec = card->textCodec(region);

	// Remove trailing NULL characters before converting to UTF-8.
	QByteArray filenameData(dirEntry->filename, sizeof(dirEntry->filename));
	int nullChr = filenameData.indexOf('\0');
	if (nullChr >= 0)
		filenameData.resize(nullChr);

	// Convert the filename to UTF-8.
	if (!textCodec) {
		// No text codec was found.
		// Default to Latin-1.
		filename = QString::fromLatin1(filenameData.constData(), filenameData.size());
	} else {
		// Use the text codec.
		filename = textCodec->toUnicode(filenameData.constData(), filenameData.size());
	}

	// Timestamp.
	lastModified.setGcnTimestamp(dirEntry->lastmodified);

	// Get the block size.
	const int blockSize = card->blockSize();

	// Load the block containing the comments.
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
 * Read the specified range from the file.
 * @param blockStart First block.
 * @param len Length, in blocks.
 * @return QByteArray with file data, or empty QByteArray on error.
 */
QByteArray MemCardFilePrivate::readBlocks(uint16_t blockStart, int len)
{
	// Check if the starting block is valid.
	if (blockStart >= this->size()) {
		// Starting block is larger than the filesize.
		return QByteArray();
	}

	// Check if the length is valid.
	uint16_t blockEnd = blockStart + len;
	if (blockEnd > this->size()) {
		// Reading too much data.
		// Truncate it to the available data.
		blockEnd = this->size();
		len = (this->size() - blockStart);
	}

	const int blockSize = card->blockSize();
	QByteArray blockData;
	blockData.resize(len * blockSize);

	uint8_t *blockDataPtr = (uint8_t*)blockData.data();
	for (int i = 0; i < len; i++, blockDataPtr += blockSize) {
		const uint16_t physBlockAddr = fileBlockAddrToPhysBlockAddr(i);
		card->readBlock(blockDataPtr, blockSize, physBlockAddr);
	}
	return blockData;
}

/**
 * Determine if an address + length is within fileSize.
 * @param address Address.
 * @param length Length.
 * @param fileSize File size.
 * @return True if the address + length is within fileSize; false if not.
 */
inline bool MemCardFilePrivate::IsInFile(uint32_t address, uint32_t length, uint32_t fileSize)
{
	if (address > fileSize || length > fileSize)
		return false;

	uint64_t addrPlusLen = ((uint64_t)address + (uint64_t)length);
	return !(addrPlusLen > (uint64_t)fileSize);
}

/**
 * Load the banner image.
 * @return GcImage* containing the banner image, or nullptr on error.
 */
GcImage *MemCardFilePrivate::loadBannerImage(void)
{
	// Determine the banner length.
	uint32_t imgSize = 0;
	switch (dirEntry->bannerfmt & CARD_BANNER_MASK)
	{
		case CARD_BANNER_CI:
			imgSize = (CARD_BANNER_W * CARD_BANNER_H * 1);
			break;
		case CARD_BANNER_RGB:
			imgSize = (CARD_BANNER_W * CARD_BANNER_H * 2);
			break;
		default:
			// No banner.
			return nullptr;
	}

	// Load the banner.
	uint32_t imgAddr = dirEntry->iconaddr;
	const int blockSize = card->blockSize();
	const uint16_t blockStart = (imgAddr / blockSize);
	QByteArray imgData = readBlocks(blockStart, 1);
	if (imgData.size() != blockSize)
		return nullptr;
	imgAddr &= 0x1FFF;

	GcImage *gcBannerImg = nullptr;
	switch (dirEntry->bannerfmt & CARD_BANNER_MASK) {
		case CARD_BANNER_CI:
			// CI8 palette is right after the banner.
			// (256 entries in RGB5A3 format.)
			gcBannerImg = GcImage::fromCI8(CARD_BANNER_W, CARD_BANNER_H,
					(const uint8_t*)&imgData.constData()[imgAddr], imgSize,
					(const uint16_t*)&imgData.constData()[imgAddr + imgSize], 0x200);
			break;

		case CARD_BANNER_RGB:
			gcBannerImg = GcImage::fromRGB5A3(CARD_BANNER_W, CARD_BANNER_H,
					(const uint16_t*)&imgData.constData()[imgAddr], imgSize);
			break;

		default:
			break;
	}

	return gcBannerImg;
}

/**
 * Load the icon images.
 * @return QVector<GcImage*> containing the icon images, or empty QVector on error.
 */
QVector<GcImage*> MemCardFilePrivate::loadIconImages(void)
{
	// Calculate the first icon address.
	uint32_t imgAddr = dirEntry->iconaddr;
	switch (dirEntry->bannerfmt & CARD_BANNER_MASK)
	{
		case CARD_BANNER_CI:
			imgAddr += (CARD_BANNER_W * CARD_BANNER_H * 1);
			imgAddr += 0x200; // palette
			break;
		case CARD_BANNER_RGB:
			imgAddr += (CARD_BANNER_W * CARD_BANNER_H * 2);
			break;
		default:
			// No banner.
			break;
	}

	// Calculate the icon sizes.
	int iconSizes[8];
	int iconLenTotal = 0;
	bool isShared = false;
	uint16_t iconfmt = dirEntry->iconfmt;
	for (int i = 0; i < CARD_MAXICONS; i++, iconfmt >>= 2) {
		switch (iconfmt & CARD_ICON_MASK) {
			case CARD_ICON_CI_SHARED:
				iconSizes[i] = (CARD_ICON_W * CARD_ICON_H * 1);
				iconLenTotal += iconSizes[i];
				isShared = true;
				break;
			case CARD_ICON_CI_UNIQUE:
				iconSizes[i] = (CARD_ICON_W * CARD_ICON_H * 1);
				iconLenTotal += iconSizes[i] + 0x200;
				break;
			case CARD_BANNER_RGB:
				iconSizes[i] = (CARD_ICON_W * CARD_ICON_H * 2);
				iconLenTotal += iconSizes[i];
				break;
		}
	}

	if (isShared) {
		// CARD_ICON_CI_SHARED has a palette stored
		// after all of the icons.
		iconLenTotal += 0x200;
	}

	// Load the icon data.
	const int blockSize = card->blockSize();
	const uint16_t blockStart = (imgAddr / blockSize);
	const uint16_t blockEnd = ((imgAddr + iconLenTotal) / blockSize);
	const int blockLen = ((blockEnd - blockStart) + 1);
	QByteArray imgData = readBlocks(blockStart, blockLen);
	if (imgData.size() != (blockLen * blockSize))
		return QVector<GcImage*>();
	imgAddr &= 0x1FFF;

	// Decode the icon(s).
	QVector<CI8_SHARED_data> lst_CI8_SHARED;
	QVector<GcImage*> gcImages;

	iconfmt = dirEntry->iconfmt;
	for (int i = 0; i < CARD_MAXICONS; i++, iconfmt >>= 2) {
		switch (iconfmt & CARD_ICON_MASK) {
			case CARD_ICON_CI_SHARED: {
				// CI8 palette is after *all* the icons.
				// (256 entries in RGB5A3 format.)
				// This is handled after the rest of the icons.
				CI8_SHARED_data data;
				data.iconIdx = i;
				data.iconAddr = imgAddr;
				lst_CI8_SHARED.append(data);

				// Add a nullptr as a placeholder.
				gcImages.append(nullptr);

				// Next icon.
				imgAddr += (CARD_ICON_W * CARD_ICON_H * 1);
				continue;
			}

			case CARD_ICON_CI_UNIQUE: {
				// CI8 palette is right after the icon.
				// (256 entries in RGB5A3 format.)
				const int imageSize = (CARD_ICON_W * CARD_ICON_H * 1);
				GcImage *gcIcon = GcImage::fromCI8(CARD_ICON_W, CARD_ICON_H,
						(const uint8_t*)&imgData.constData()[imgAddr], imageSize,
						(const uint16_t*)&imgData.constData()[imgAddr + imageSize], 0x200);
				gcImages.append(gcIcon);
				imgAddr += imageSize + 0x200;
				break;
			}

			case CARD_BANNER_RGB: {
				const int imageSize = (CARD_ICON_W * CARD_ICON_H * 2);
				GcImage *gcIcon = GcImage::fromRGB5A3(CARD_ICON_W, CARD_ICON_H,
						(const uint16_t*)&imgData.constData()[imgAddr], imageSize);
				gcImages.append(gcIcon);
				imgAddr += imageSize;
				break;
			}

			default:
				// No icon.
				// Add a nullptr as a placeholder.
				gcImages.append(nullptr);
				break;
		}
	}

	if (!lst_CI8_SHARED.isEmpty()) {
		// Process CI8 SHARED icons.
		// TODO: Convert the palette once instead of every time?
		const int imageSize = (CARD_ICON_W * CARD_ICON_H * 1);
		foreach (const CI8_SHARED_data& data, lst_CI8_SHARED) {
			GcImage *gcIcon = GcImage::fromCI8(CARD_ICON_W, CARD_ICON_H,
						(const uint8_t*)&imgData.constData()[data.iconAddr], imageSize,
						(const uint16_t*)&imgData.constData()[imgAddr], 0x200);
			gcImages[data.iconIdx] = gcIcon;
		}
	}

	// Shrink the QVector.
	int iconCount = gcImages.size();
	for (; iconCount > 0; iconCount--) {
		if (gcImages.at(iconCount-1))
			break;
	}
	if (iconCount > 0)
		gcImages.resize(iconCount);
	else
		gcImages.clear();

	return gcImages;
}

/**
 * Load the banner and icon images.
 */
void MemCardFilePrivate::loadImages(void)
{
	// Load the banner.
	this->gcBanner = loadBannerImage();
	if (gcBanner) {
		// Set the new banner image.
		QImage qBanner = gcImageToQImage(gcBanner);
		if (!qBanner.isNull())
			banner = QPixmap::fromImage(qBanner);
		else
			banner = QPixmap();
	} else {
		// No banner image.
		banner = QPixmap();
	}

	// Load the icons.
	this->gcIcons = loadIconImages();
	icons.clear();
	icons.reserve(gcIcons.size());
	foreach (GcImage *gcIcon, gcIcons) {
		if (gcIcon) {
			QImage qIcon = gcImageToQImage(gcIcon);
			if (!qIcon.isNull())
				icons.append(QPixmap::fromImage(qIcon));
			else
				icons.append(QPixmap());
		} else {
			// No icon image.
			icons.append(QPixmap());
		}
	}
}

/**
 * Calculate the file checksum.
 */
void MemCardFilePrivate::calculateChecksum(void)
{
	checksumValues.clear();

	if (checksumDefs.empty()) {
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
	for (int i = 0; i < (int)checksumDefs.size(); i++) {
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
		checksumValues.push_back(checksumValue);
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
	, d_ptr(new MemCardFilePrivate(this, card, fileIdx, dat, bat))
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
		const QVector<uint16_t> &fatEntries)
	: QObject(card)
	, d_ptr(new MemCardFilePrivate(this, card, dirEntry, fatEntries))
{ }

MemCardFile::~MemCardFile()
{
	Q_D(MemCardFile);
	delete d;
}

/**
 * Get the game code.
 * @return Game code.
 */
QString MemCardFile::gamecode(void) const
{
	Q_D(const MemCardFile);
	return d->gamecode;
}

/**
 * Get the company code.
 * @return Company code.
 */
QString MemCardFile::company(void) const
{
	Q_D(const MemCardFile);
	return d->company;
}

/**
 * Get the GCN filename.
 * @return GCN filename.
 */
QString MemCardFile::filename(void) const
{
	Q_D(const MemCardFile);
	return d->filename;
}

/**
 * Get the last modified time.
 * @return Last modified time.
 */
GcnDateTime MemCardFile::lastModified(void) const
{
	Q_D(const MemCardFile);
	return d->lastModified;
}

/**
 * Get the game description. ("Comments" field.)
 * @return Game description.
 */
QString MemCardFile::gameDesc(void) const
{
	Q_D(const MemCardFile);
	return d->gameDesc;
}

/**
 * Get the file description. ("Comments" field.)
 * @return File description.
 */
QString MemCardFile::fileDesc(void) const
{
	Q_D(const MemCardFile);
	return d->fileDesc;
}

/**
 * Get the file permissions.
 * @return File permissions.
 */
uint8_t MemCardFile::permission(void) const
{
	Q_D(const MemCardFile);
	return d->dirEntry->permission;
}

/**
 * Get the file permissions as a string.
 * @return File permission string.
 */
QString MemCardFile::permissionAsString(void) const
{
	Q_D(const MemCardFile);
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
{
	Q_D(const MemCardFile);
	return d->size();
}

/**
 * Get the banner image.
 * @return Banner image, or null QPixmap on error.
 */
QPixmap MemCardFile::banner(void) const
{
	Q_D(const MemCardFile);
	return d->banner;
}

/**
 * Get the number of icons in the file.
 * @return Number of icons.
 */
int MemCardFile::numIcons(void) const
{
	Q_D(const MemCardFile);
	return d->icons.size();
}

/**
 * Get an icon from the file.
 * @param idx Icon number.
 * @return Icon, or null QPixmap on error.
 */
QPixmap MemCardFile::icon(int idx) const
{
	Q_D(const MemCardFile);
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
	Q_D(const MemCardFile);
	if (idx < 0 || idx >= d->icons.size())
		return CARD_SPEED_END;
	return ((d->dirEntry->iconspeed >> (idx * 2)) & CARD_SPEED_MASK);
}

/**
 * Get the icon animation mode.
 * @return Icon animation mode.
 */
int MemCardFile::iconAnimMode(void) const
{
	Q_D(const MemCardFile);
	return (d->dirEntry->bannerfmt & CARD_ANIM_MASK);
}

/**
 * Is this a lost file?
 * @return True if lost; false if file is in the directory table.
 */
bool MemCardFile::isLostFile(void) const
{
	Q_D(const MemCardFile);
	return (d->fileIdx < 0);
}

/**
 * Get this file's FAT entries.
 * @return FAT entries.
 */
QVector<uint16_t> MemCardFile::fatEntries(void) const
{
	Q_D(const MemCardFile);
	return d->fatEntries;
}

/**
 * Get the checksum definitions.
 * @return Checksum definitions.
 */
QVector<Checksum::ChecksumDef> MemCardFile::checksumDefs(void) const
{
	Q_D(const MemCardFile);
	return d->checksumDefs;
}

/**
 * Set the checksum definitions.
 * @param checksumDefs Checksum definitions.
 */
void MemCardFile::setChecksumDefs(const QVector<Checksum::ChecksumDef> &checksumDefs)
{
	Q_D(MemCardFile);
	d->checksumDefs = checksumDefs;
	d->calculateChecksum();
}

/**
 * Get the checksum values.
 * @return Checksum values, or empty QVector if no checksum definitions were set.
 */
QVector<Checksum::ChecksumValue> MemCardFile::checksumValues(void) const
{
	Q_D(const MemCardFile);
	return d->checksumValues;
}

/**
 * Get the checksum algorithm.
 * NOTE: We're assuming each file only uses one algorithm...
 * @return Checksum algorithm.
 */
Checksum::ChkAlgorithm MemCardFile::checksumAlgorithm(void) const
{
	Q_D(const MemCardFile);
	if (d->checksumDefs.isEmpty())
		return Checksum::CHKALG_NONE;
	return d->checksumDefs.at(0).algorithm;
}

/**
 * Get the checksum status.
 * @return Checksum status.
 */
Checksum::ChkStatus MemCardFile::checksumStatus(void) const
{
	Q_D(const MemCardFile);
	return Checksum::ChecksumStatus(d->checksumValues.toStdVector());
}

/**
 * Format checksum values as HTML for display purposes.
 * @return QVector containing one or two HTML strings.
 * - String 0 contains the actual checksums.
 * - String 1, if present, contains the expected checksums.
 */
QVector<QString> MemCardFile::checksumValuesFormatted(void) const
{
	Q_D(const MemCardFile);
	vector<string> vs = Checksum::ChecksumValuesFormatted(d->checksumValues.toStdVector());
	QVector<QString> ret;
	ret.reserve((int)vs.size());
	for (int i = 0; i < (int)vs.size(); i++) {
		ret.append(QString::fromStdString(vs[i]));
	}
	return ret;
}

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
	Q_D(const MemCardFile);

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
int MemCardFile::saveGci(const QString &filename)
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
	Q_D(MemCardFile);

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

/**
 * Get the directory entry.
 * @return Directory entry.
 */
const card_direntry *MemCardFile::dirEntry(void) const
{
	Q_D(const MemCardFile);
	return d->dirEntry;
}

/**
 * Save the banner image.
 * @param filenameNoExt Filename for the GCI file, sans extension.
 * @return 0 on success; non-zero on error.
 * TODO: Error code constants.
 */
int MemCardFile::saveBanner(const QString &filenameNoExt) const
{
	Q_D(const MemCardFile);
	if (!d->gcBanner)
		return -EINVAL;

	// Append the correct extension.
	QString filename = filenameNoExt;
	const char *ext = GcImageWriter::extForImageFormat(GcImageWriter::IMGF_PNG);
	if (ext)
		filename += QChar(L'.') + QLatin1String(ext);

	QFile file(filename);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
		// Error opening the file.
		// TODO: Convert QFileError to a POSIX error code.
		return -EIO;
	}

	// Write the banner image.
	int ret = saveBanner(&file);
	file.close();

	if (ret != 0) {
		// Error saving the banner image.
		file.remove();
	}

	return ret;
}

/**
 * Save the banner image.
 * @param qioDevice QIODevice to write the banner image to.
 * @return 0 on success; non-zero on error.
 * TODO: Error code constants.
 */
int MemCardFile::saveBanner(QIODevice *qioDevice) const
{
	Q_D(const MemCardFile);
	if (!d->gcBanner)
		return -EINVAL;

	GcImageWriter gcImageWriter;
	int ret = gcImageWriter.write(d->gcBanner, GcImageWriter::IMGF_PNG);
	if (!ret) {
		const vector<uint8_t> *pngData = gcImageWriter.memBuffer();
		ret = qioDevice->write(reinterpret_cast<const char*>(pngData->data()), pngData->size());
		if (ret != (qint64)pngData->size())
			return -EIO;
		ret = 0;
	}

	// Saved the banner image.
	return ret;
}

/**
 * Save the icon.
 * @param filenameNoExt Filename for the icon, sans extension.
 * @param animImgf Animated image format to use for animated icons.
 * @return 0 on success; non-zero on error.
 * TODO: Error code constants.
 */
int MemCardFile::saveIcon(const QString &filenameNoExt,
	GcImageWriter::AnimImageFormat animImgf) const
{
	Q_D(const MemCardFile);
	if (d->gcIcons.isEmpty())
		return -EINVAL;

	// Append the correct extension.
	const char *ext;
	if (d->gcIcons.size() > 1) {
		// Animated icon.
		ext = GcImageWriter::extForAnimImageFormat(animImgf);
	} else {
		// Static icon.
		ext = GcImageWriter::extForImageFormat(GcImageWriter::IMGF_PNG);
	}

	// NOTE: Due to PNG_FPF saving multiple files, we can't simply
	// call a version of saveIcon() that takes a QIODevice.
	GcImageWriter gcImageWriter;
	int ret;
	if (d->gcIcons.size() > 1) {
		// Animated icon.
		vector<const GcImage*> gcImages;
		const int maxIcons = (d->gcIcons.size() * 2 - 2);
		gcImages.reserve(maxIcons);
		gcImages.resize(d->gcIcons.size());
		for (int i = 0; i < d->gcIcons.size(); i++)
			gcImages[i] = d->gcIcons[i];

		// Icon speed.
		vector<int> gcIconDelays;
		gcIconDelays.reserve(maxIcons);
		gcIconDelays.resize(d->gcIcons.size());
		for (int i = 0; i < d->gcIcons.size(); i++)
			gcIconDelays[i] = iconDelay(i);

		if (iconAnimMode() == CARD_ANIM_BOUNCE) {
			// BOUNCE animation.
			int src = (gcImages.size() - 2);
			int dest = gcImages.size();
			gcImages.resize(maxIcons);
			gcIconDelays.resize(maxIcons);
			for (; src >= 1; src--, dest++) {
				gcImages[dest] = gcImages[src];
				gcIconDelays[dest] = gcIconDelays[src];
			}
		}

		ret = gcImageWriter.write(&gcImages, &gcIconDelays, animImgf);
	} else {
		// Static icon.
		ret = gcImageWriter.write(d->gcIcons.at(0), GcImageWriter::IMGF_PNG);
	}

	if (ret != 0) {
		// Error writing the icon.
		return ret;
	}

	// Icon written successfully.
	// Save it to a file.
	for (int i = 0; i < gcImageWriter.numFiles(); i++) {
		QString filename = filenameNoExt;
		if (gcImageWriter.numFiles() > 1) {
			// Multiple files.
			// Append the file number.
			char tmp[8];
			snprintf(tmp, sizeof(tmp), "%02d", i+1);
			filename += QChar(L'.') + QLatin1String(tmp);
		}

		// Append the file extension.
		if (ext)
			filename += QChar(L'.') + QLatin1String(ext);

		QFile file(filename);
		if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
			// Error opening the file.
			// TODO: Convert QFileError to a POSIX error code.
			// TODO: Delete previous files?
			return -EIO;
		}

		const vector<uint8_t> *pngData = gcImageWriter.memBuffer(i);
		ret = file.write(reinterpret_cast<const char*>(pngData->data()), pngData->size());
		file.close();

		if (ret != (qint64)pngData->size()) {
			// Error saving the icon.
			file.remove();
			return -EIO;
		}

		ret = 0;
	}

	return ret;
}
