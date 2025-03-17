/***************************************************************************
 * GameCube Memory Card Recovery Program [libmemcard]                      *
 * File_p.hpp: Memory Card file entry. [base class] (PRIVATE)              *
 *                                                                         *
 * Copyright (c) 2012-2025 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#pragma once

#include "File.hpp"
class Card;
class GcImage;

#include "Checksum.hpp"

// C includes
#include <stdint.h>

// C++ includes
#include <vector>

class FilePrivate
{
public:
	/**
	 * Initialize the FilePrivate private class.
	 * @param q File object
	 * @param card Card this file belongs to
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

	// FAT entries
	// (TODO: Always 16-bit?)
	std::vector<uint16_t> fatEntries;

	// File information
	QString filename;	// Internal filename.
	// TODO: Add a QFlags indicating which fields are valid.
	QString gameID;		// Game ID, e.g. GALE01
	QDateTime mtime;	// Last Modified time.
	QString description;	// Description.
	uint32_t mode;		// Mode. (attributes, permissions)
	// Size is calculated using fatEntries.size().

	// GcImages. (internal use only)
	GcImage *gcBanner;
	QVector<GcImage*> gcIcons;
	// FIXME: Use system-independent values.
	// Currently uses GCN values.
	QVector<uint8_t> iconSpeed;
	uint8_t iconAnimMode;

	// QPixmap images
	QPixmap banner;
	QVector<QPixmap> icons;

	// Lost File information
	bool lostFile;

	/**
	 * Get the file size, in blocks.
	 * @return File size, in blocks
	 */
	int size(void) const;

	/**
	 * Convert a file block number to a physical block number.
	 * @param fileBlock File block number
	 * @return Physical block number, or negative on error.
	 */
	uint16_t fileBlockAddrToPhysBlockAddr(uint16_t fileBlock) const;

	/**
	 * Load the file data.
	 * @return QByteArray with file data, or empty QByteArray on error.
	 */
	QByteArray loadFileData(void);

	/**
	 * Read the specified range from the file.
	 * @param blockStart First block
	 * @param len Length, in blocks
	 * @return QByteArray with file data, or empty QByteArray on error.
	 */
	QByteArray readBlocks(uint16_t blockStart, int len);

	/**
	 * Strip invalid DOS characters from a filename.
	 * @param filename Filename
	 * @param replaceChar Replacement character
	 * @return Filename with invalid DOS characters replaced with replaceChar.
	 */
	static QString StripInvalidDosChars(const QString &filename, QChar replaceChar = QChar(L'_'));

	/**
	 * Attempt to decode text as Shift-JIS.
	 * If that fails, use cp1252.
	 * @param str Text data
	 * @param len Length of str
	 * @return Unicode QString
	 */
	static QString decodeText_SJISorCP1252(const char *str, int len);

	/** Images **/

	/**
	 * Load the banner and icon images.
	 */
	void loadImages(void);

	/**
	 * Load the banner image.
	 * @return GcImage containing the banner image, or nullptr on error.
	 */
	virtual GcImage *loadBannerImage(void) = 0;

	/**
	 * Load the icon images.
	 * @return QVector<GcImage*> containing the icon images, or empty QVector on error.
	 */
	virtual QVector<GcImage*> loadIconImages(void) = 0;

	/** Checksums **/

	// Checksum data
	std::vector<Checksum::ChecksumDef> checksumDefs;
	std::vector<Checksum::ChecksumValue> checksumValues;

	/**
	 * Calculate the file checksum.
	 */
	void calculateChecksum(void);
};
