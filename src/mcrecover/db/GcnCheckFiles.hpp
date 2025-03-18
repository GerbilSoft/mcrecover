/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * GcnCheckFiles.hpp: Validate checksums on normal GCN files.              *
 *                                                                         *
 * Copyright (c) 2013-2025 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#pragma once

// Search Data struct
#include "GcnSearchData.hpp"

// Qt includes
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QVector>

class GcnCard;
class GcnFile;

class GcnCheckFilesPrivate;
class GcnCheckFiles : public QObject
{
	Q_OBJECT
	typedef QObject super;

public:
	explicit GcnCheckFiles(QObject *parent = 0);
	virtual ~GcnCheckFiles();

protected:
	GcnCheckFilesPrivate *const d_ptr;
	Q_DECLARE_PRIVATE(GcnCheckFiles)
private:
	Q_DISABLE_COPY(GcnCheckFiles)

public:
	/**
	 * Load multiple GCN Memory Card File databases.
	 * TODO: Singleton DB file management class
	 * @param dbFilenames Filenames of GCN Memory Card File database
	 * @return 0 on success; non-zero on error. (Check error string!)
	 */
	int loadGcnMcFileDbs(const QVector<QString> &dbFilenames);

	/**
	 * Load a GCN Memory Card File database.
	 * TODO: Singleton DB file management class.
	 * @param dbFilename Filename of GCN Memory Card File database
	 * @return 0 on success; non-zero on error. (Check error string!)
	 */
	inline int loadGcnMcFileDb(const QString &dbFilename)
	{
		QVector<QString> dbFilenames;
		dbFilenames.append(dbFilename);
		return loadGcnMcFileDbs(dbFilenames);
	}

public:
	/**
	 * Add checksum definitions to a file if it doesn't
	 * already have any.
	 *
	 * TODO: Return value?
	 */
	void addChecksumDefs(GcnFile *file) const;

	/**
	 * Add checksum definitions to all files on a GcnCard
	 * if they don't already have any.
	 *
	 * TODO: Return value?
	 */
	void addChecksumDefs(GcnCard *card) const;
};
