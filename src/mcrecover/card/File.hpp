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

// TODO: "Generic" image writer?
#include "GcImageWriter.hpp"

// Checksums.
#include "Checksum.hpp"

// Qt includes.
#include <QtCore/QString>
#include <QtCore/QVector>
#include <QtCore/QIODevice>
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
	Q_PROPERTY(QString description READ description)
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

	/* TODO: Register Checksum metatypes?
	Q_PROPERTY(QVector<Checksum::ChecksumDef> checksumDefs READ checksumDefs WRITE setChecksumDefs)
	Q_PROPERTY(QVector<Checksum::ChecksumValue> checksumValues READ checksumValues WRITE setChecksumValues)
	Q_PROPERTY(Checksum::ChkAlgorithm checksumAlgorithm READ checksumAlgorithm)
	Q_PROPERTY(Checksum::ChkStatus checksumStatus READ checksumStatus)
	// TODO: checksumValuesFormatted?
	*/

	protected:
		/**
		 * Create a File for a Card.
		 * This Card object is NOT valid by itself, and must
		 * be subclassed by a system-specific class. The subclass
		 * constructor must then initialize the File, including
		 * fatEntries and other properties.
		 * @param d FilePrivate-derived private class.
		 * @param card Card object.
		 */
		File(FilePrivate *d, Card *card);
	public:
		virtual ~File();

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

		/**
		 * Load the file data.
		 * @return QByteArray with file data, or empty QByteArray on error.
		 */
		QByteArray loadFileData(void);

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
		 * Get the file's description.
		 * This is for UI purposes only.
		 * @return File's description.
		 */
		QString description(void) const;

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
		virtual QString modeAsString(void) const = 0;

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
		virtual int exportToFile(const QString &filename);

		/**
		 * Export the file.
		 * @param qioDevice QIODevice to write the data to.
		 * @return 0 on success; non-zero on error.
		 * TODO: Error code constants.
		 */
		virtual int exportToFile(QIODevice *qioDevice) = 0;

		/** Images **/

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

		/** Checksums **/

		/**
		 * Get the checksum definitions.
		 * @return Checksum definitions.
		 */
		QVector<Checksum::ChecksumDef> checksumDefs(void) const;

		/**
		 * Set the checksum definitions.
		 * @param cksumDefs Checksum definitions.
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
};

#endif /* __MCRECOVER_CARD_FILE_HPP__ */
