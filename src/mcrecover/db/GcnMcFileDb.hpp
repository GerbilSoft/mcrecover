/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * GcnMcFileDb.hpp: GCN Memory Card File Database class.                   *
 *                                                                         *
 * Copyright (c) 2013-2025 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#pragma once

// Card definitions.
#include "card.h"

// Checksum algorithm class.
#include "Checksum.hpp"

// Search data.
#include "GcnSearchData.hpp"

// Qt includes.
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QVector>

class GcnFile;

class GcnMcFileDbPrivate;
class GcnMcFileDb : public QObject
{
	Q_OBJECT
	typedef QObject super;

public:
	explicit GcnMcFileDb(QObject *parent = 0);
	virtual ~GcnMcFileDb();

protected:
	GcnMcFileDbPrivate *const d_ptr;
	Q_DECLARE_PRIVATE(GcnMcFileDb)
private:
	Q_DISABLE_COPY(GcnMcFileDb)

public:
	/**
	 * Load a GCN Memory Card File database.
	 * @param filename Filename of the database file
	 * @return 0 on success; non-zero on error.
	 */
	int load(const QString &filename);

	/**
	 * Get the error string.
	 * This is set if load() fails.
	 * @return Error string
	 */
	QString errorString(void) const;

	/**
	 * Check a GCN memory card block to see if it matches any search patterns.
	 * @param buf	[in] GCN memory card block to check
	 * @param size	[in] Size of buf (Should be BLOCK_SIZE == 0x2000.)
	 * @return QVector of matches, or empty QVector if no matches were found.
	 */
	QVector<GcnSearchData> checkBlock(const void *buf, size_t size) const;

	/**
	 * Get a list of database files.
	 * This function checks various paths for *.xml.
	 * If two files with the same filename are found,
	 * the one in the higher-precedence directory gets
	 * higher precedence.
	 * @return List of database files
	 */
	static QVector<QString> GetDbFilenames(void);

	/**
	 * Add checksum definitions to an open file.
	 *
	 * NOTE: The file must NOT have checksum definitions before calling
	 * this function.
	 *
	 * @param file GcnFile
	 * @return True if definitions were added by this class; false if not.
	 */
	bool addChecksumDefs(GcnFile *file) const;
};
