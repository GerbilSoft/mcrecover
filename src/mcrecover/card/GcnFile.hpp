/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * GcnCard.hpp: GameCube file entry class.                                 *
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

#ifndef __MCRECOVER_CARD_GCNFILE_HPP__
#define __MCRECOVER_CARD_GCNFILE_HPP__

#include "File.hpp"
// TODO: Remove card.h from here.
#include "card.h"

// Qt includes.
class QIODevice;

// Checksum algorithm class.
#include "Checksum.hpp"
// GcImage
class GcImage;
#include "GcImageWriter.hpp"

class GcnCard;

class GcnFilePrivate;
class GcnFile : public File
{
	Q_OBJECT

	Q_PROPERTY(QString gameDesc READ gameDesc)
	Q_PROPERTY(QString fileDesc READ fileDesc)

	public:
		/**
		 * Create a GcnFile for a GcnCard.
		 * This constructor is for valid files.
		 * @param card GcnCard.
		 * @param direntry Directory Entry pointer.
		 * @param mc_bat Block table.
		 */
		GcnFile(GcnCard *card,
			const card_direntry *dirEntry,
			const card_bat *mc_bat);

		/**
		 * Create a GcnFile for a GcnCard.
		 * This constructor is for lost files.
		 * @param card GcnCard.
		 * @param direntry Directory Entry pointer.
		 * @param fatEntries FAT entries.
		 */
		GcnFile(GcnCard *card,
			const card_direntry *dirEntry,
			const QVector<uint16_t> &fatEntries);

		virtual ~GcnFile();

	protected:
		Q_DECLARE_PRIVATE(GcnFile)
	private:
		Q_DISABLE_COPY(GcnFile)

	public:
		/**
		 * Get the game description.
		 * This is the first half of the file's description.
		 * @return Game description.
		 */
		QString gameDesc(void) const;

		/**
		 * Get the file description.
		 * This is the second half of the file's description.
		 * @return File description.
		 */
		QString fileDesc(void) const;

		/** TODO: Move encoding to File. **/

		/**
		 * Get the text encoding ID for this file.
		 * @return Text encoding ID.
		 */
		int encoding(void) const;

		/**
		 * Get the file's mode as a string.
		 * This is system-specific.
		 * @return File mode as a string.
		 */
		virtual QString modeAsString(void) const override;

		/** Lost File information **/

		/**
		 * Get the default export filename.
		 * @return Default export filename.
		 */
		virtual QString defaultExportFilename(void) const override;

		// TODO: Function to get format/extension of exported file.

		/**
		 * Export the file.
		 * @param filename Filename for the exported file.
		 * @return 0 on success; non-zero on error.
		 * TODO: Error code constants.
		 */
		virtual int exportToFile(const QString &filename) override;

		/**
		 * Export the file.
		 * @param qioDevice QIODevice to write the data to.
		 * @return 0 on success; non-zero on error.
		 * TODO: Error code constants.
		 */
		virtual int exportToFile(QIODevice *qioDevice) override;

		/**
		 * Get the directory entry.
		 * @return Directory entry.
		 */
		const card_direntry *dirEntry(void) const;

		// TODO: Move these down to File.

		/**
		 * Save the banner image.
		 * @param qioDevice QIODevice to write the banner image to.
		 * @return 0 on success; non-zero on error.
		 * TODO: Error code constants.
		 */
		virtual int saveBanner(QIODevice *qioDevice) const override;

		/**
		 * Save the icon.
		 * @param filenameNoExt Filename for the icon, sans extension.
		 * @param animImgf Animated image format for animated icons.
		 * @return 0 on success; non-zero on error.
		 * TODO: Error code constants.
		 */
		virtual int saveIcon(const QString &filenameNoExt,
			     GcImageWriter::AnimImageFormat animImgf) const override;
};

#endif /* __MCRECOVER_CARD_GCNFILE_HPP__ */
