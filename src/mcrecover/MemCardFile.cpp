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

#define NUM_ELEMENTS(x) ((int)(sizeof(x) / sizeof(x[0])))

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
		uint32_t lastModified;	// Last modified time. (UNIX timestamp)
		
		// File information. (Comment, banner, icon)
		QString gameDesc;	// Game description.
		QString fileDesc;	// File description.
		
		// Images.
		QImage banner;
		QImage icon[CARD_MAXICONS];
		uint8_t iconSpeed;	// TODO: Animation.
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
	lastModified = (direntry->lastmodified + GC_UNIX_TIME_DIFF);
	
	const int blockSize = card->blockSize();
	
	// Load the file comments. (64 bytes)
	// 0x00: Game description.
	// 0x20: File description.
	int commentBlock = (direntry->commentaddr / blockSize);
	int commentOffset = (direntry->commentaddr % blockSize);
	
	// Read the block containing the file comments.
	uint8_t *tmpBlock = (uint8_t*)malloc(blockSize);
	card->readBlock(tmpBlock, blockSize, commentBlock);
	
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
