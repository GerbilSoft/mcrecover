/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * GcnCheckFiles.cpp: Validate checksums on normal GCN files.              *
 *                                                                         *
 * Copyright (c) 2013-2018 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#include "GcnCheckFiles.hpp"

// GcnCard
#include "libmemcard/GcnCard.hpp"
#include "libmemcard/GcnFile.hpp"

// GCN Memory Card File Database.
#include "db/GcnMcFileDb.hpp"

// Checksum algorithm class.
#include "libgctools/Checksum.hpp"

// Qt includes.
#include <QtCore/QLinkedList>
#include <QtCore/QStack>
#include <QtCore/QThread>

class GcnCheckFilesPrivate
{
	public:
		explicit GcnCheckFilesPrivate(GcnCheckFiles *q);
		~GcnCheckFilesPrivate();

	protected:
		GcnCheckFiles *const q_ptr;
		Q_DECLARE_PUBLIC(GcnCheckFiles)
	private:
		Q_DISABLE_COPY(GcnCheckFilesPrivate)

	public:
		// GCN Memory Card File databases.
		QVector<GcnMcFileDb*> dbs;
};

GcnCheckFilesPrivate::GcnCheckFilesPrivate(GcnCheckFiles* q)
	: q_ptr(q)
{ }	

GcnCheckFilesPrivate::~GcnCheckFilesPrivate()
{
	qDeleteAll(dbs);
}

/** GcnCheckFiles **/

GcnCheckFiles::GcnCheckFiles(QObject *parent)
	: super(parent)
	, d_ptr(new GcnCheckFilesPrivate(this))
{ }

GcnCheckFiles::~GcnCheckFiles()
{
	Q_D(GcnCheckFiles);
	delete d;
}

/** Functions. **/

/**
 * Load multiple GCN Memory Card File databases.
 * TODO: Singleton DB file management class.
 * @param dbFilenames Filenames of GCN Memory Card File database.
 * @return 0 on success; non-zero on error. (Check error string!)
 */
int GcnCheckFiles::loadGcnMcFileDbs(const QVector<QString> &dbFilenames)
{
	Q_D(GcnCheckFiles);
	qDeleteAll(d->dbs);
	d->dbs.clear();

	if (dbFilenames.isEmpty())
		return 0;

	// Load the databases.
	foreach (const QString &dbFilename, dbFilenames) {
		GcnMcFileDb *db = new GcnMcFileDb(this);
		int ret = db->load(dbFilename);
		if (!ret) {
			d->dbs.append(db);
		} else {
			delete db;
		}
	}

	// TODO: Report if any DBs were unable to be loaded.
	// For now, just error if no DBs could be loaded.
	if (d->dbs.isEmpty()) {
		// TODO: Set the error string.
		return -1;
	}

	return 0;
}

/**
 * Add checksum definitions to a file if it doesn't
 * already have any.
 *
 * TODO: Return value?
 */
void GcnCheckFiles::addChecksumDefs(GcnFile *file) const
{
	if (file->checksumStatus() != Checksum::CHKST_UNKNOWN) {
		// Checksum has already been obtained for this file.
		return;
	}

	Q_D(const GcnCheckFiles);
	foreach (GcnMcFileDb *db, d->dbs) {
		bool ok = db->addChecksumDefs(file);
		if (ok)
			break;
	}
}

/**
 * Add checksum definitions to all files on a GcnCard
 * if they don't already have any.
 *
 * TODO: Return value?
 */
void GcnCheckFiles::addChecksumDefs(GcnCard *card) const
{
	const int fileCount = card->fileCount();
	for (int i = 0; i < fileCount; i++) {
		// NOTE: nullptr check *shouldn't* be needed...
		GcnFile *file = qobject_cast<GcnFile*>(card->getFile(i));
		if (file != nullptr) {
			addChecksumDefs(file);
		}
	}
}
