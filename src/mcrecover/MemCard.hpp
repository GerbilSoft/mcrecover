/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * MemCard.hpp: Memory Card reader class.                                  *
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

#ifndef __MCRECOVER_MEMCARD_HPP__
#define __MCRECOVER_MEMCARD_HPP__

// C includes.
#include <stdint.h>

// Qt includes.
#include <QtCore/QObject>

class MemCardPrivate;

class MemCard : public QObject
{
	Q_OBJECT
	
	public:
		MemCard(QObject *parent, const QString& filename);
		MemCard(const QString& filename);
		~MemCard();
	
	private:
		friend class MemCardPrivate;
		MemCardPrivate *const d;
		Q_DISABLE_COPY(MemCard);
	
	public:
		/**
		 * Check if the memory card is open.
		 * @return True if open; false if not.
		 */
		bool isOpen(void) const;
		
		/**
		 * Get the memory card filename.
		 * @return Memory card filename, or empty string if not open.
		 */
		QString filename(void) const;
		
		/**
		 * Get the size of the memory card, in blocks.
		 * @return Size of memory card, in blocks.
		 */
		int sizeInBlocks(void) const;
		
		/**
		 * Get the memory card block size, in bytes.
		 * @return Memory card block size, in bytes.
		 */
		int blockSize(void) const;
		
		/**
		 * Read a block.
		 * @param buf Buffer to read the block data into.
		 * @param siz Size of buffer.
		 * @param blockIdx Block index.
		 * @return Bytes read on success; negative on error.
		 */
		int readBlock(void *buf, size_t siz, uint16_t blockIdx);
		
		/**
		 * Get the memory card encoding.
		 * @return 0 for ANSI; 1 for SJIS; negative on error.
		 */
		int encoding(void) const;
};

#endif /* __MCRECOVER_MEMCARD_HPP__ */
