/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * MemCardFile.cpp: Memory Card file entry class.                          *
 *                                                                         *
 * Copyright (c) 2011 by David Korth.                                      *
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

/** MemCardFilePrivate **/

class MemCardFilePrivate
{
	public:
		MemCardFilePrivate(MemCardFile *q,
				MemCard *card, const int fileIdx,
				const card_dat *dat, const card_bat *bat);
	
	private:
		MemCardFile *const q;
		Q_DISABLE_COPY(MemCardFilePrivate);
	
	public:
		MemCard *const card;
		const int fileIdx;
		const card_dat *const dat;
		const card_bat *const bat;
		
		// Directory entry.
		// This points to an entry within dat.
		const card_direntry *m_direntry;
		
		// FAT entries.
		QVector<uint16_t> fat_entries;
		
		// File information. (Directory table.)
		QString gamecode;
		QString company;
		QString filename;
		QDateTime lastModified;	// Last modified time.
		
		// File information. (Comment, banner, icon)
		QString gameDesc;	// Game description.
		QString fileDesc;	// File description.
		
		// Images.
		bool imagesLoaded;
		QImage banner;
		QVector<QImage> icons;
		
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
		struct CI8_SHARED_data
		{
			int iconIdx;
			uint32_t iconAddr;
		};
		
		/**
		 * Load the banner and icon images..
		 */
		void loadImages(void);
};

MemCardFilePrivate::MemCardFilePrivate(MemCardFile *q,
		MemCard *card, const int fileIdx,
		const card_dat *dat, const card_bat *bat)
	: q(q)
	, card(card)
	, fileIdx(fileIdx)
	, dat(dat)
	, bat(bat)
	, imagesLoaded(false)
	, banner(NULL)
{
	// Load the directory table information.
	m_direntry = &dat->entries[fileIdx];
	
	// Load the FAT entries.
	fat_entries.clear();
	fat_entries.reserve(m_direntry->length);
	uint16_t last_block = m_direntry->block;
	if (last_block >= 5 && last_block != 0xFFFF)
	{
		fat_entries.append(last_block);
		for (int i = 1; i < m_direntry->length; i++)
		{
			last_block = bat->fat[last_block - 5];
			if (last_block == 0xFFFF || last_block < 5)
				break;
			fat_entries.append(last_block);
		}
	}
	
	// TODO: Convert Shift-JIS filenames to UTF-8.
	filename = QString::fromLatin1(m_direntry->filename, sizeof(m_direntry->filename));
	filename = filename.replace(QChar(0), QChar(L' '));
	
	gamecode = QString::fromLatin1(m_direntry->gamecode, sizeof(m_direntry->gamecode));
	company = QString::fromLatin1(m_direntry->company, sizeof(m_direntry->company));
	
	// TODO: GC memory card time uses local time.
	// QDateTime::setTime_t uses UTC. Add local time offset!
	lastModified.setTime_t(m_direntry->lastmodified + GC_UNIX_TIME_DIFF);
	
	// Get the block size.
	const int blockSize = card->blockSize();
	
	// Load the block with the comments.
	const int commentBlock = (m_direntry->commentaddr / blockSize);
	const int commentOffset = (m_direntry->commentaddr % blockSize);
	
	uint8_t *commentData = (uint8_t*)malloc(blockSize);
	card->readBlock(commentData, blockSize, fileBlockAddrToPhysBlockAddr(commentBlock));
	
	// Load the file comments. (64 bytes)
	// 0x00: Game description.
	// 0x20: File description.
	
	// Convert NULL characters to spaces to prevent confusion.
	uint8_t *commentPtr = &commentData[commentOffset];
	for (int i = 0; i < 64; i++)
	{
		if (commentPtr[i] == 0x00)
			commentPtr[i] = ' ';
	}
	
	// TODO: Convert Shift-JIS comments to UTF-8.
	gameDesc = QString::fromLatin1((char*)&commentPtr[0], 32).trimmed();
	fileDesc = QString::fromLatin1((char*)&commentPtr[32], 32).trimmed();
	
	// Free the comment data.
	free(commentData);
}


/**
 * Convert a file block number to a physical block number.
 * @param fileBlock File block number.
 * @return Physical block number, or negative on error.
 */
uint16_t MemCardFilePrivate::fileBlockAddrToPhysBlockAddr(uint16_t fileBlock)
{
	if ((int)fileBlock >= fat_entries.size())
		return -1;
	return fat_entries.at((int)fileBlock);
}


/**
 * Load file data.
 * @return QByteArray with file data, or empty QByteArray on error.
 */
QByteArray MemCardFilePrivate::loadFileData(void)
{
	const int blockSize = card->blockSize();
	if (m_direntry->length > card->sizeInBlocks())
		return QByteArray();
	
	QByteArray fileData;
	fileData.resize(m_direntry->length * blockSize);
	
	uint8_t *fileDataPtr = (uint8_t*)fileData.data();
	for (int i = 0; i < m_direntry->length; i++, fileDataPtr += blockSize)
	{
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
	// Images are being loaded.
	imagesLoaded = true;
	
	// Load the file.
	QByteArray fileData = loadFileData();
	if (fileData.isEmpty())
		return;
	
	// Decode the banner.
	uint32_t iconAddr = m_direntry->iconaddr;
	uint32_t imageSize = 0;
	banner = QImage();	// Clear the current banner.
	
	switch (m_direntry->bannerfmt & CARD_BANNER_MASK)
	{
		case CARD_BANNER_CI:
			// CI8 palette is right after the banner.
			// (256 entries in RGB5A3 format.)
			imageSize = (CARD_BANNER_W * CARD_BANNER_H * 1);
			if ((iconAddr + imageSize) > fileData.size())
				break;
			banner = GcImage::FromCI8(CARD_BANNER_W, CARD_BANNER_H,
					&fileData.constData()[iconAddr], imageSize,
					&fileData.constData()[iconAddr + imageSize], 0x200);
			iconAddr += imageSize + 0x200;
			break;
		
		case CARD_BANNER_RGB:
			imageSize = (CARD_BANNER_W * CARD_BANNER_H * 2);
			if ((iconAddr + imageSize) > fileData.size())
				break;
			banner = GcImage::FromRGB5A3(CARD_BANNER_W, CARD_BANNER_H,
					&fileData.constData()[iconAddr], imageSize);
			iconAddr += imageSize;
			break;
		
		default:
			break;
	}
	
	// Decode the icon(s).
	QVector<CI8_SHARED_data> lst_CI8_SHARED;
	icons.clear();
	
	uint16_t iconfmt = m_direntry->iconfmt;
	for (int i = 0; i < CARD_MAXICONS; i++)
	{
		if ((iconfmt & CARD_ICON_MASK) == CARD_ICON_CI_SHARED)
		{
			// CI8 palette is after *all* the icons.
			// (256 entries in RGB5A3 format.)
			// This is handled after the rest of the icons.
			CI8_SHARED_data data;
			data.iconIdx = i;
			data.iconAddr = iconAddr;
			lst_CI8_SHARED.append(data);
			
			// Add a NULL QImage as a placeholder.
			icons.append(QImage());
			
			// Next icon.
			imageSize = (CARD_ICON_W * CARD_ICON_H * 1);
			iconAddr += imageSize;
			iconfmt >>= 2;
			continue;
		}
		
		switch (iconfmt & CARD_ICON_MASK)
		{
			case CARD_ICON_CI_UNIQUE:
				// CI8 palette is right after the icon.
				// (256 entries in RGB5A3 format.)
				imageSize = (CARD_ICON_W * CARD_ICON_H * 1);
				if ((iconAddr + imageSize + 0x200) > fileData.size())
					break;
				icons.append(GcImage::FromCI8(CARD_ICON_W, CARD_ICON_H,
						&fileData.constData()[iconAddr], imageSize,
						&fileData.constData()[iconAddr + imageSize], 0x200));
				iconAddr += imageSize + 0x200;
				break;
			
			case CARD_BANNER_RGB:
				imageSize = (CARD_ICON_W * CARD_ICON_H * 2);
				if ((iconAddr + imageSize) > fileData.size())
					break;
				icons.append(GcImage::FromRGB5A3(CARD_ICON_W, CARD_ICON_H,
						&fileData.constData()[iconAddr], imageSize));
				iconAddr += imageSize;
				break;
			
			default:
				// No icon.
				// Add a NULL image as a placeholder.
				icons.append(QImage());
				break;
		}
		
		// Next icon.
		iconfmt >>= 2;
	}
	
	if (!lst_CI8_SHARED.isEmpty())
	{
		// Process CI8 SHARED icons.
		// TODO: Convert the palette once instead of every time?
		if ((iconAddr + 0x200) > fileData.size())
		{
			// Out of bounds.
			// Delete the NULL icons.
			for (int i = (icons.size() - 1); i >= 0; i--)
			{
				if (icons.at(i).isNull())
					icons.remove(i);
			}
		}
		else
		{
			// Process each icon.
			foreach (const CI8_SHARED_data& data, lst_CI8_SHARED)
			{
				icons[data.iconIdx] = GcImage::FromCI8(CARD_ICON_W, CARD_ICON_H,
							&fileData.constData()[data.iconAddr], imageSize,
							&fileData.constData()[iconAddr], 0x200);
			}
		}
	}
}


/** MemCardFile **/

MemCardFile::MemCardFile(MemCard *card, const int fileIdx,
			const card_dat *dat, const card_bat *bat)
	: QObject(card)
	, d(new MemCardFilePrivate(this, card, fileIdx, dat, bat))
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
 * Get the GC filename.
 * @return GC filename.
 */
QString MemCardFile::filename(void) const
	{ return d->filename; }

/**
 * Get the last modified time.
 * @return Last modified time.
 */
QDateTime MemCardFile::lastModified(void) const
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
	{ return d->m_direntry->permission; }

/**
 * Get the file permissions as a string.
 * @return File permission string.
 */
QString MemCardFile::permissionAsString(void) const
{
	char str[4];
	
	uint8_t permission = d->m_direntry->permission;
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
uint8_t MemCardFile::size(void) const
	{ return d->m_direntry->length; }

/**
 * Get the banner image.
 * @return Banner image.
 */
QImage MemCardFile::banner(void)
{
	if (!d->imagesLoaded)
		d->loadImages();
	return d->banner;
}

/**
 * Get the number of icons in the file.
 * @return Number of icons.
 */
int MemCardFile::numIcons(void) const
	{ return d->icons.size(); }

/**
 * Get an icon from the file.
 * @param idx Icon number.
 * @return Icon, or null QImage on error.
 */
QImage MemCardFile::icon(int idx)
{
	if (!d->imagesLoaded)
		d->loadImages();
	if (idx < 0 || idx >= d->icons.size())
		return QImage();
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
	
	return ((d->m_direntry->iconspeed >> (idx * 2)) & CARD_SPEED_MASK);
}

/**
 * Get the icon animation mode.
 * @return Icon animation mode.
 */
int MemCardFile::iconAnimMode(void) const
	{ return (d->m_direntry->bannerfmt & CARD_ANIM_MASK); }

/**
 * Ensure that the images are loaded.
 * Does nothing if images are already loaded.
 */
void MemCardFile::verifyImagesLoaded(void)
{
	if (d->card && !d->imagesLoaded)
		d->loadImages();
}

