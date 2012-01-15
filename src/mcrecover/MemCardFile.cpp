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
		uint8_t reserved_00[128];	// FIXME: QImage is screwing up...
		QImage banner;
		uint8_t reserved_01[128];	// FIXME: QImage is screwing up...
		QVector<QImage> icons;
		uint8_t reserved_02[128];	// FIXME: QImage is screwing up...
		
		uint8_t iconSpeed;	// TODO: Animation.
		
		/**
		 * Convert a file block number to a physical block number.
		 * @param fileBlock File block number.
		 * @return Physical block number, or negative on error.
		 */
		uint16_t fileBlockAddrToPhysBlockAddr(uint16_t fileBlock);
};

MemCardFilePrivate::MemCardFilePrivate(MemCardFile *q,
		MemCard *card, const int fileIdx,
		const card_dat *dat, const card_bat *bat)
	: q(q)
	, card(card)
	, fileIdx(fileIdx)
	, dat(dat)
	, bat(bat)
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
	
	// Load the file data.
	// TODO: Split this into another function?
	uint8_t *fileData = (uint8_t*)malloc(m_direntry->length * blockSize);
	uint8_t *filePtr = fileData;
	for (int i = 0; i < m_direntry->length; i++, filePtr += blockSize)
	{
		const uint16_t physBlockAddr = fileBlockAddrToPhysBlockAddr(i);
		card->readBlock(filePtr, blockSize, physBlockAddr);
	}
	
	// Load the file comments. (64 bytes)
	// 0x00: Game description.
	// 0x20: File description.
	
	// Convert NULL characters to spaces to prevent confusion.
	const uint32_t commentAddr = m_direntry->commentaddr;
	for (filePtr = &fileData[commentAddr];
	     filePtr < (&fileData[commentAddr] + 64); filePtr++)
	{
		if (*filePtr == 0x00)
			*filePtr = ' ';
	}
	
	// TODO: Convert Shift-JIS comments to UTF-8.
	gameDesc = QString::fromLatin1((char*)&fileData[commentAddr], 32).trimmed();
	fileDesc = QString::fromLatin1((char*)&fileData[commentAddr+32], 32).trimmed();
	
	// Decode the banner.
	uint32_t iconAddr = m_direntry->iconaddr;
	uint32_t bannerSize = 0;
	switch (m_direntry->bannerfmt & CARD_BANNER_MASK)
	{
		case CARD_BANNER_CI:
			// CI8 palette is right after the banner.
			// (256 entries in RGB5A1 format.)
			bannerSize = (CARD_BANNER_W * CARD_BANNER_H * 1) + 0x200;
			banner = GcImage::FromCI8(CARD_BANNER_W, CARD_BANNER_H,
					&fileData[iconAddr], bannerSize);
			iconAddr += bannerSize;
			break;
		
		case CARD_BANNER_RGB:
			bannerSize = (CARD_BANNER_W * CARD_BANNER_H * 2);
			banner = GcImage::FromRGB5A3(CARD_BANNER_W, CARD_BANNER_H,
					&fileData[iconAddr], bannerSize);
			iconAddr += bannerSize;
			break;
		
		default:
			break;
	}
	
	// Free the file data.
	free(fileData);
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
QImage MemCardFile::banner(void) const
	{ return d->banner; }
