/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * File_p.hpp: Memory Card file entry. [base class] (PRIVATE)              *
 *                                                                         *
 * Copyright (c) 2012-2015 by David Korth.                                 *
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

#ifndef __MCRECOVER_CARD_FILE_P_HPP__
#define __MCRECOVER_CARD_FILE_P_HPP__

#include "File.hpp"
class Card;

// C includes.
#include <stdint.h>

class FilePrivate
{
	public:
		/**
		 * Initialize the FilePrivate private class.
		 * @param q File object.
		 * @param card Card this file belongs to.
		 */
		FilePrivate(File *q, Card *card);
		virtual ~FilePrivate();

	protected:
		File *const q_ptr;
		Q_DECLARE_PUBLIC(File)
	private:
		Q_DISABLE_COPY(FilePrivate)

	public:
		Card *const card;

		// FAT entries.
		// (TODO: Always 16-bit?)
		QVector<uint16_t> fatEntries;

		// File information.
		QString filename;	// Internal filename.
		// TODO: Add a QFlags indicating which fields are valid.
		QString gameID;		// Game ID, e.g. GALE01
		GcnDateTime mtime;	// Last Modified time.
		QString gameDesc;	// Game description.
		QString fileDesc;	// File description.
		uint32_t mode;		// Mode. (attributes, permissions)
		// Size is calculated using fatEntries.size().

		// Images.
		QPixmap banner;
		QVector<QPixmap> icons;
		// FIXME: Use system-independent values.
		// Currently uses GCN values.
		QVector<uint8_t> iconSpeed;
		uint8_t iconAnimMode;

		// Lost File information.
		bool lostFile;

		/**
		 * Get the file size, in blocks.
		 * @return File size, in blocks.
		 */
		int size(void) const;

		/**
		 * Convert a file block number to a physical block number.
		 * @param fileBlock File block number.
		 * @return Physical block number, or negative on error.
		 */
		uint16_t fileBlockAddrToPhysBlockAddr(uint16_t fileBlock) const;

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

		/**
		 * Strip invalid DOS characters from a filename.
		 * @param filename Filename.
		 * @param replaceChar Replacement character.
		 * @return Filename with invalid DOS characters replaced with replaceChar.
		 */
		static QString StripInvalidDosChars(const QString &filename, QChar replaceChar = QChar(L'_'));
};

#endif /* __MCRECOVER_CARD_FILE_P_HPP__ */