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

// Qt includes.
#include <QtCore/QString>
#include <QtGui/QImage>

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
		// TODO: These seem to cause memory errors...
		//QImage banner;
		//QImage icon[CARD_MAXICONS];
		//uint8_t iconSpeed;	// TODO: Animation.
		
		/**
		 * Convert a file block number to a physical block number.
		 * @param fileBlock File block number.
		 * @return Physical block number, or negative on error.
		 */
		uint16_t fileBlockToPhysBlock(uint16_t fileBlock);
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
	fat_entries.append(last_block);
	for (int i = 1; i < m_direntry->length; i++)
	{
		last_block = bat->fat[last_block - 5];
		if (last_block == 0xFFFF)
			break;
		fat_entries.append(last_block);
	}
	
	// TODO: Convert Shift-JIS filenames to UTF-8.
	filename = QString::fromLatin1(m_direntry->filename, sizeof(m_direntry->filename));
	filename = filename.replace(QChar(0), QChar(L' '));
	
	gamecode = QString::fromLatin1(m_direntry->gamecode, sizeof(m_direntry->gamecode));
	company = QString::fromLatin1(m_direntry->company, sizeof(m_direntry->company));
	
	// TODO: GC memory card time uses local time.
	// QDateTime::setTime_t uses UTC. Add local time offset!
	lastModified.setTime_t(m_direntry->lastmodified + GC_UNIX_TIME_DIFF);
	
	const int blockSize = card->blockSize();
	
	// Load the file comments. (64 bytes)
	// 0x00: Game description.
	// 0x20: File description.
	const uint16_t commentBlock = fileBlockToPhysBlock((m_direntry->commentaddr / blockSize));
	const int commentOffset = (m_direntry->commentaddr % blockSize);
	
	// Read the block containing the file comments.
	uint8_t *tmpBlock = (uint8_t*)malloc(blockSize);
	card->readBlock(tmpBlock, blockSize, commentBlock);
	
	// Convert NULL characters to spaces to prevent confusion.
	for (int i = commentOffset; i < (commentOffset+64); i++)
	{
		if (tmpBlock[i] == 0x00)
			tmpBlock[i] = ' ';
	}
	
	// TODO: Convert Shift-JIS comments to UTF-8.
	gameDesc = QString::fromLatin1((char*)&tmpBlock[commentOffset], 32).trimmed();
	fileDesc = QString::fromLatin1((char*)&tmpBlock[commentOffset+32], 32).trimmed();
}


/**
 * Convert a file block number to a physical block number.
 * @param fileBlock File block number.
 * @return Physical block number, or negative on error.
 */
uint16_t MemCardFilePrivate::fileBlockToPhysBlock(uint16_t fileBlock)
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
 * Get the size, in blocks.
 * @return Size, in blocks.
 */
uint8_t MemCardFile::size(void) const
	{ return d->m_direntry->length; }
