/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * File.hpp: Memory Card file entry. [base class]                          *
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

#ifndef __MCRECOVER_CARD_FILE_HPP__
#define __MCRECOVER_CARD_FILE_HPP__

#include "Card.hpp"
#include "GcnDateTime.hpp"

// Qt includes.
#include <QtCore/QString>
#include <QtCore/QVector>
#include <QtGui/QPixmap>

class FilePrivate;
class File : public QObject
{
	Q_OBJECT

	// File information.
	Q_PROPERTY(QString filename READ filename)
	// TODO: Add a QFlags indicating which fields are valid.
	// TODO: Switch back to QDateTime?
	// Add an attribute indicating "no timezone"?
	Q_PROPERTY(QString gameID READ gameID)
	Q_PROPERTY(GcnDateTime mtime READ mtime)
	Q_PROPERTY(QString gameDesc READ gameDesc)
	Q_PROPERTY(QString fileDesc READ fileDesc)
	Q_PROPERTY(uint32_t mode READ mode)
	Q_PROPERTY(int size READ size)

	// Icon and banner.
	Q_PROPERTY(QPixmap banner READ banner)
	Q_PROPERTY(int iconCount READ iconCount)
	// TODO: Icon array?
	// TODO: Enum for animation mode.
	Q_PROPERTY(int iconAnimMode READ iconAnimMode)

	// TODO: Always 16-bit FAT?
	Q_PROPERTY(QVector<uint16_t> fatEntries READ fatEntries)

	// Lost File information.
	Q_PROPERTY(bool lostFile READ isLostFile)

	// Default export filename.
	// Should be customized per card type.
	Q_PROPERTY(QString defaultExportFilename READ defaultExportFilename STORED false)

	protected:
		/**
		 * Create a File for a Card.
		 * This Card object is NOT valid by itself, and must
		 * be subclassed by a system-specific class. The subclass
		 * constructor must then initialize the File, including
		 * fatEntries and other properties.
		 * @param card Card object.
		 */
		File(Card *card);
	public:
		~File();

	protected:
		FilePrivate *const d_ptr;
		Q_DECLARE_PRIVATE(File)
	private:
		Q_DISABLE_COPY(File)

	public:
		/** File information **/

		/**
		 * Get the internal filename.
		 * @return Filename.
		 */
		QString filename(void) const;

		/**
		 * Get this file's FAT entries.
		 * @return FAT entries.
		 */
		QVector<uint16_t> fatEntries(void) const;

		/** TODO: Add a QFlags indicating which fields are valid. **/

		/**
		 * Get the game ID.
		 * @return Game ID.
		 */
		QString gameID(void) const;

		/**
		 * Get the last modified time.
		 * @return Last modified time.
		 */
		GcnDateTime mtime(void) const;

		/**
		 * Get the game description.
		 * @return Game description.
		 */
		QString gameDesc(void) const;

		/**
		 * Get the file description.
		 * @return File description.
		 */
		QString fileDesc(void) const;

		/**
		 * Get the file's mode. (attributes, permissions)
		 * NOTE: Values may be system-specific.
		 * FIXME: Use system-independent values?
		 * @return File mode.
		 */
		uint32_t mode(void) const;

		/**
		 * Get the file's mode as a string.
		 * This is system-specific.
		 * @return File mode as a string.
		 */
		virtual uint32_t modeAsString(void) const = 0;

		/**
		 * Get the file size, in blocks.
		 * @return File size, in blocks.
		 */
		int size(void) const;

		/** Icon and banner **/

		/**
		 * Get the banner image.
		 * @return Banner image, or null QPixmap on error.
		 */
		QPixmap banner(void) const;

		/**
		 * Get the number of icons in the file.
		 * @return Number of icons.
		 */
		int iconCount(void) const;

		/**
		 * Get an icon from the file.
		 * @param idx Icon number.
		 * @return Icon, or null QPixmap on error.
		 */
		QPixmap icon(int idx) const;

		/**
		 * Get the delay for a given icon.
		 * FIXME: Use system-independent values.
		 * Currently uses GCN values.
		 * @param idx Icon number.
		 * @return Icon delay.
		 */
		int iconDelay(int idx) const;

		/**
		 * Get the icon animation mode.
		 * FIXME: Use system-independent values.
		 * Currently uses GCN values.
		 * @return Icon animation mode.
		 */
		int iconAnimMode(void) const;

		/** Lost File information **/

		/**
		 * Is this a lost file?
		 * @return True if lost; false if file is in the directory table.
		 */
		bool isLostFile(void) const;

		/** Export **/

		/**
		 * Get the default export filename.
		 * @return Default export filename.
		 */
		virtual QString defaultExportFilename(void) const = 0;

		/**
		 * Export the file.
		 * @param filename Filename for the exported file.
		 * @return 0 on success; non-zero on error.
		 * TODO: Error code constants.
		 */
		virtual int exportToFile(const QString &filename) = 0;

		// TODO: Export banner and icon.
};

#endif /* __MCRECOVER_CARD_FILE_HPP__ */
