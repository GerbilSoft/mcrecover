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
	const card_direntry *direntry = &dat->entries[fileIdx];
	
	// TODO: Convert Shift-JIS filenames to UTF-8.
	filename = QString::fromLatin1(direntry->filename, sizeof(direntry->filename));
	
	gamecode = QString::fromLatin1(direntry->gamecode, sizeof(direntry->gamecode));
	company = QString::fromLatin1(direntry->company, sizeof(direntry->company));
	
	// TODO: GC memory card time uses local time.
	// QDateTime::setTime_t uses UTC. Add local time offset!
	lastModified.setTime_t(direntry->lastmodified + GC_UNIX_TIME_DIFF);
	
	const int blockSize = card->blockSize();
	const uint16_t fileStartBlock = direntry->block;
	
	// Load the file comments. (64 bytes)
	// 0x00: Game description.
	// 0x20: File description.
	const uint16_t commentBlock = (uint16_t)(direntry->commentaddr / blockSize);
	const int commentOffset = (direntry->commentaddr % blockSize);
	
	// Read the block containing the file comments.
	uint8_t *tmpBlock = (uint8_t*)malloc(blockSize);
	card->readBlock(tmpBlock, blockSize, fileStartBlock + commentBlock);
	
	// TODO: Convert Shift-JIS comments to UTF-8.
	gameDesc = QString::fromLatin1((char*)&tmpBlock[commentOffset], 32).trimmed();
	fileDesc = QString::fromLatin1((char*)&tmpBlock[commentOffset+32], 32).trimmed();
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
