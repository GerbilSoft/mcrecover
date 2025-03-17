/***************************************************************************
 * GameCube Memory Card Recovery Program [libmemcard]                      *
 * GcnCard.hpp: GameCube file entry class.                                 *
 *                                                                         *
 * Copyright (c) 2012-2025 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#pragma once

#include "File.hpp"
// TODO: Remove card.h from here.
#include "card.h"

// C++ includes
#include <vector>

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
	typedef File super;

	Q_PROPERTY(QString gameDesc READ gameDesc)
	Q_PROPERTY(QString fileDesc READ fileDesc)

public:
	/**
	 * Create a GcnFile for a GcnCard.
	 * This constructor is for valid files.
	 * @param card GcnCard (or GciCard)
	 * @param direntry Directory Entry pointer
	 * @param mc_bat Block table
	 */
	GcnFile(Card *card,
		const card_direntry *dirEntry,
		const card_bat *mc_bat);

	/**
	 * Create a GcnFile for a GcnCard.
	 *
	 * This constructor is for:
	 * - Lost files: fatEntries must be set to the FAT chain.
	 * - GCI files: fatEntries must be empty.
	 *
	 * @param card GcnCard (or GciCard)
	 * @param direntry Directory Entry pointer
	 * @param fatEntries FAT entries
	 */
	GcnFile(Card *card,
		const card_direntry *dirEntry,
		const std::vector<uint16_t> &fatEntries);

	protected:
		Q_DECLARE_PRIVATE(GcnFile)
	private:
		Q_DISABLE_COPY(GcnFile)

public:
	/**
	 * Get the game description.
	 * This is the first half of the file's description.
	 * @return Game description
	 */
	QString gameDesc(void) const;

	/**
	 * Get the file description.
	 * This is the second half of the file's description.
	 * @return File description
	 */
	QString fileDesc(void) const;

	/**
	 * Get the file's mode as a string.
	 * This is system-specific.
	 * @return File mode as a string
	 */
	QString modeAsString(void) const final;

	/** Lost File information **/

	/**
	 * Get the default export filename.
	 * @return Default export filename
	 */
	QString defaultExportFilename(void) const final;

	// TODO: Function to get format/extension of exported file.

	/**
	 * Export the file.
	 * @param filename Filename for the exported file
	 * @return 0 on success; non-zero on error.
	 * TODO: Error code constants.
	 */
	int exportToFile(const QString &filename) final;

	/**
	 * Export the file.
	 * @param qioDevice QIODevice to write the data to
	 * @return 0 on success; non-zero on error.
	 * TODO: Error code constants.
	 */
	int exportToFile(QIODevice *qioDevice) final;

	/**
	 * Get the directory entry.
	 * @return Directory entry
	 */
	const card_direntry *dirEntry(void) const;
};
