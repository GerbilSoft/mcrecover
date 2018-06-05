/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * GcnCheckFiles.hpp: Validate checksums on normal GCN files.              *
 *                                                                         *
 * Copyright (c) 2013-2018 by David Korth.                                 *
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
 * You should have received a copy of the GNU General Public License       *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.   *
 ***************************************************************************/

#ifndef __MCRECOVER_DB_GCNCHECKFILES_HPP__
#define __MCRECOVER_DB_GCNCHECKFILES_HPP__

// Search Data struct.
#include "GcnSearchData.hpp"

// Qt includes.
#include <QtCore/QObject>
#include <QtCore/QString>

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
		 * Load a GCN Memory Card File database.
		 * TODO: Singleton DB file management class.
		 * @param dbFilename Filename of GCN Memory Card File database.
		 * @return 0 on success; non-zero on error. (Check error string!)
		 */
		inline int loadGcnMcFileDb(const QString &dbFilename);

		/**
		 * Load multiple GCN Memory Card File databases.
		 * TODO: Singleton DB file management class.
		 * @param dbFilenames Filenames of GCN Memory Card File database.
		 * @return 0 on success; non-zero on error. (Check error string!)
		 */
		int loadGcnMcFileDbs(const QVector<QString> &dbFilenames);

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

/**
 * Load a GCN Memory Card File database.
 * @param dbFilename Filename of GCN Memory Card File database.
 * @return 0 on success; non-zero on error. (Check error string!)
 */
inline int GcnCheckFiles::loadGcnMcFileDb(const QString &dbFilename)
{
	QVector<QString> dbFilenames;
	dbFilenames.append(dbFilename);
	return loadGcnMcFileDbs(dbFilenames);
}

#endif /* __MCRECOVER_DB_GCNCHECKFILES_HPP__ */
