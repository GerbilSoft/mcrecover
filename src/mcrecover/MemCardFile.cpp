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

#define NUM_ELEMENTS(x) (sizeof(x) / sizeof(x[0]))

/** MemCardFilePrivate **/

class MemCardFilePrivate
{
	public:
		MemCardFilePrivate(MemCardFile *q,
				const MemCard *card, const int fileIdx,
				const card_dat *dat, const card_bat *bat);
	
	private:
		MemCardFile *const q;
		Q_DISABLE_COPY(MemCardFilePrivate);
	
	public:
		const MemCard *const card;
		const int fileIdx;
		const card_dat *const dat;
		const card_bat *const bat;
};

MemCardFilePrivate::MemCardFilePrivate(MemCardFile *q,
		const MemCard *card, const int fileIdx,
		const card_dat *dat, const card_bat *bat)
	: q(q)
	, card(card)
	, fileIdx(fileIdx)
	, dat(dat)
	, bat(bat)
{
	// TODO: Load the file and extract the images.
}


/** MemCardFile **/

MemCardFile::MemCardFile(const MemCard *card, const int fileIdx,
			const card_dat *dat, const card_bat *bat)
	: d(new MemCardFilePrivate(this, card, fileIdx, dat, bat))
{ }

MemCardFile::~MemCardFile()
	{ delete d; }
