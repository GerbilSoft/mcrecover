/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * MemCardFile.hpp: Memory Card file entry class.                          *
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

#ifndef __MCRECOVER_MEMCARDFILE_HPP__
#define __MCRECOVER_MEMCARDFILE_HPP__

#include "card.h"

// C++ includes.
#include <vector>

// Qt includes.
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QVector>
#include <QtGui/QPixmap>
class QIODevice;

// GcnDateTime: QDateTime wrapper.
#include "GcnDateTime.hpp"

// Checksum algorithm class.
#include "Checksum.hpp"

// GcImage
class GcImage;
#include "GcImageWriter.hpp"

class GcnCard;

class MemCardFilePrivate;

class MemCardFile : public QObject
{
	Q_OBJECT

	Q_PROPERTY(QString id6 READ id6)
	Q_PROPERTY(QString id4 READ id4)
	Q_PROPERTY(QString id3 READ id3)
	Q_PROPERTY(QString company READ company)
	Q_PROPERTY(QString filename READ filename)
	Q_PROPERTY(GcnDateTime lastModified READ lastModified)
	Q_PROPERTY(QString gameDesc READ gameDesc)
	Q_PROPERTY(QString fileDesc READ fileDesc)
	Q_PROPERTY(uint8_t permission READ permission)
	Q_PROPERTY(uint16_t size READ size)
	Q_PROPERTY(QPixmap banner READ banner)
	Q_PROPERTY(int numIcons READ numIcons)
	// TODO: Icon array?
	Q_PROPERTY(int iconAnimMode READ iconAnimMode)
	Q_PROPERTY(bool lostFile READ isLostFile)
	Q_PROPERTY(QVector<uint16_t> fatEntries READ fatEntries)
	/* TODO: Register Checksum metatypes?
	Q_PROPERTY(QVector<Checksum::ChecksumDef> checksumDefs READ checksumDefs WRITE setChecksumDefs)
	Q_PROPERTY(QVector<Checksum::ChecksumValue> checksumValues READ checksumValues WRITE setChecksumValues)
	Q_PROPERTY(Checksum::ChkAlgorithm checksumAlgorithm READ checksumAlgorithm)
	Q_PROPERTY(Checksum::ChkStatus checksumStatus READ checksumStatus)
	// TODO: checksumValuesFormatted?
	*/
	Q_PROPERTY(QString defaultGciFilename READ defaultGciFilename STORED false)

	public:
		/**
		 * Create a MemCardFile for a GcnCard.
		 * This constructor is for valid files.
		 * @param card MemCard.
		 * @param fileIdx File index in GcnCard.
		 * @param dat Directory table.
		 * @param bat Block allocation table.
		 */
		MemCardFile(GcnCard *card, const int fileIdx,
				const card_dat *dat, const card_bat *bat);

		/**
		 * Create a MemCardFile for a GcnCard.
		 * This constructor is for "lost" files.
		 * @param card GcnCard.
		 * @param dirEntry Constructed directory entry.
		 * @param fatEntries FAT entries.
		 */
		MemCardFile(GcnCard *card,
				const card_direntry *dirEntry,
				const QVector<uint16_t> &fatEntries);

		~MemCardFile();
	
	protected:
		MemCardFilePrivate *const d_ptr;
		Q_DECLARE_PRIVATE(MemCardFile)
	private:
		Q_DISABLE_COPY(MemCardFile)

	public:
		/**
		 * Get the 6-character game ID, e.g. GALE01.
		 * @return 6-character game ID.
		 */
		QString id6(void) const;

		/**
		 * Get the 4-character game ID, e.g. GALE.
		 * @return 4-character game ID.
		 */
		QString id4(void) const;

		/**
		 * Get the 3-character game ID, e.g. GAL.
		 * @return 3-character game ID.
		 */
		QString id3(void) const;

		/**
		 * Get the company code, e.g. 01.
		 * @return Company code.
		 */
		QString company(void) const;

		/**
		 * Get the internal filename.
		 * @return Internal filename.
		 */
		QString filename(void) const;

		/**
		 * Get the last modified time.
		 * @return Last modified time.
		 */
		GcnDateTime lastModified(void) const;

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
		 * Get the file size, in blocks.
		 * @return File size, in blocks.
		 */
		uint16_t size(void) const;

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

		/**
		 * Get the checksum definitions.
		 * @return Checksum definitions.
		 */
		QVector<Checksum::ChecksumDef> checksumDefs(void) const;

		/**
		 * Set the checksum definitions.
		 * @param checksumDefs Checksum definitions.
		 */
		void setChecksumDefs(const QVector<Checksum::ChecksumDef> &checksumDefs);

		/**
		 * Get the checksum values.
		 * @return Checksum values, or empty QVector if no checksum definitions were set.
		 */
		QVector<Checksum::ChecksumValue> checksumValues(void) const;

		/**
		 * Get the checksum algorithm.
		 * NOTE: We're assuming each file only uses one algorithm...
		 * @return Checksum algorithm.
		 */
		Checksum::ChkAlgorithm checksumAlgorithm(void) const;

		/**
		 * Get the checksum status.
		 * @return Checksum status.
		 */
		Checksum::ChkStatus checksumStatus(void) const;

		/**
		 * Format checksum values as HTML for display purposes.
		 * @return QVector containing one or two HTML strings.
		 * - String 0 contains the actual checksums.
		 * - String 1, if present, contains the expected checksums.
		 */
		QVector<QString> checksumValuesFormatted(void) const;

		/**
		 * Get the default GCI filename.
		 * @return Default GCI filename.
		 */
		QString defaultGciFilename(void) const;

		/**
		 * Get the text encoding ID for this file.
		 * @return Text encoding ID.
		 */
		int encoding(void) const;

		/**
		 * Save the file in GCI format.
		 * @param filename Filename for the GCI file.
		 * @return 0 on success; non-zero on error.
		 * TODO: Error code constants.
		 */
		int saveGci(const QString &filename);

		/**
		 * Save the file in GCI format.
		 * @param qioDevice QIODevice to write the GCI data to.
		 * @return 0 on success; non-zero on error.
		 * TODO: Error code constants.
		 */
		int saveGci(QIODevice *qioDevice);

		/**
		 * Get the directory entry.
		 * @return Directory entry.
		 */
		const card_direntry *dirEntry(void) const;

		/**
		 * Save the banner image.
		 * @param filenameNoExt Filename for the banner image, sans extension.
		 * @return 0 on success; non-zero on error.
		 * TODO: Error code constants.
		 */
		int saveBanner(const QString &filenameNoExt) const;

		/**
		 * Save the banner image.
		 * @param qioDevice QIODevice to write the banner image to.
		 * @return 0 on success; non-zero on error.
		 * TODO: Error code constants.
		 */
		int saveBanner(QIODevice *qioDevice) const;

		/**
		 * Save the icon.
		 * @param filenameNoExt Filename for the icon, sans extension.
		 * @param animImgf Animated image format for animated icons.
		 * @return 0 on success; non-zero on error.
		 * TODO: Error code constants.
		 */
		int saveIcon(const QString &filenameNoExt,
			     GcImageWriter::AnimImageFormat animImgf) const;
};

#endif /* __MCRECOVER_MEMCARDFILE_HPP__ */