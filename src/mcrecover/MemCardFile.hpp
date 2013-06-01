/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * MemCardFile.hpp: Memory Card file entry class.                          *
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

#ifndef __MCRECOVER_MEMCARDFILE_HPP__
#define __MCRECOVER_MEMCARDFILE_HPP__

#include "card.h"

// Qt includes.
#include <QtCore/QDateTime>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QVector>
#include <QtGui/QPixmap>

// MemCard class.
class MemCard;

class MemCardFilePrivate;

class MemCardFile : public QObject
{
	Q_OBJECT
	
	public:
		/**
		 * Create a MemCardFile for a MemCard.
		 * This constructor is for valid files.
		 * @param card MemCard.
		 * @param fileIdx File index in MemCard.
		 * @param dat Directory table.
		 * @param bat Block allocation table.
		 */
		MemCardFile(MemCard *card, const int fileIdx,
				const card_dat *dat, const card_bat *bat);

		/**
		 * Create a MemCardFile for a MemCard.
		 * This constructor is for "lost" files.
		 * @param card MemCard.
		 * @param dirEntry Constructed directory entry.
		 * @param fatEntries FAT entries.
		 */
		MemCardFile(MemCard *card,
				const card_direntry *dirEntry,
				QVector<uint16_t> fatEntries);

		~MemCardFile();
	
	private:
		friend class MemCardFilePrivate;
		MemCardFilePrivate *const d;
		Q_DISABLE_COPY(MemCardFile);

	public:
		/**
		 * Get the game code.
		 * @return Game code.
		 */
		QString gamecode(void) const;

		/**
		 * Get the company code.
		 * @return Company code.
		 */
		QString company(void) const;

		/**
		 * Get the GCN filename.
		 * @return GCN filename.
		 */
		QString filename(void) const;

		/**
		 * Get the last modified time.
		 * @return Last modified time.
		 */
		QDateTime lastModified(void) const;

		/**
		 * Get the game description. ("Comments" field.)
		 * @return Game description.
		 */
		QString gameDesc(void) const;

		/**
		 * Get the file description. ("Comments" field.)
		 * @return File description.
		 */
		QString fileDesc(void) const;

		/**
		 * Get the file permissions.
		 * @return File permissions.
		 */
		uint8_t permission(void) const;

		/**
		 * Get the file permissions as a string.
		 * @return File permission string.
		 */
		QString permissionAsString(void) const;

		/**
		 * Get the size, in blocks.
		 * @return Size, in blocks.
		 */
		uint8_t size(void) const;

		/**
		 * Get the banner image.
		 * @return Banner image, or null QPixmap on error.
		 */
		QPixmap banner(void) const;

		/**
		 * Get the number of icons in the file.
		 * @return Number of icons.
		 */
		int numIcons(void) const;

		/**
		 * Get an icon from the file.
		 * @param idx Icon number.
		 * @return Icon, or null QPixmap on error.
		 */
		QPixmap icon(int idx) const;

		/**
		 * Get the delay for a given icon.
		 * @param idx Icon number.
		 * @return Icon delay.
		 */
		int iconDelay(int idx) const;

		/**
		 * Get the icon animation mode.
		 * @return Icon animation mode.
		 */
		int iconAnimMode(void) const;

		/**
		 * Is this a lost file?
		 * @return True if lost; false if file is in the directory table.
		 */
		bool isLostFile(void) const;

		/**
		 * Get this file's FAT entries.
		 * @return FAT entries.
		 */
		QVector<uint16_t> fatEntries(void) const;
};

#endif /* __MCRECOVER_MEMCARDFILE_HPP__ */
