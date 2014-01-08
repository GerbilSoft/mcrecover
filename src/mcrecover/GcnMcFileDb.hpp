/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * GcnMcFileDb.hpp: GCN Memory Card File Database class.                   *
 *                                                                         *
 * Copyright (c) 2013 by David Korth.                                      *
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

#ifndef __MCRECOVER_GCNMCFILEDB_HPP__
#define __MCRECOVER_GCNMCFILEDB_HPP__

// Card definitions.
#include "card.h"

// Checksum algorithm class.
#include "Checksum.hpp"

// Search data.
#include "SearchData.hpp"

// Qt includes.
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QVector>

class GcnMcFileDbPrivate;

class GcnMcFileDb : public QObject
{
	Q_OBJECT

	public:
		GcnMcFileDb(QObject *parent = 0);
		~GcnMcFileDb();

	private:
		friend class GcnMcFileDbPrivate;
		GcnMcFileDbPrivate *const d;
		Q_DISABLE_COPY(GcnMcFileDb);

	public:
		/**
		 * Load a GCN Memory Card File database.
		 * @param filename Filename of the database file.
		 * @return 0 on success; non-zero on error.
		 */
		int load(const QString &filename);

		/**
		 * Get the error string.
		 * This is set if load() fails.
		 * @return Error string.
		 */
		QString errorString(void) const;

		/**
		 * Check a GCN memory card block to see if it matches any search patterns.
		 * @param buf	[in] GCN memory card block to check.
		 * @param siz	[in] Size of buf. (Should be BLOCK_SIZE == 0x2000.)
		 * @return QVector of matches, or empty QVector if no matches were found.
		 */
		QVector<SearchData> checkBlock(const void *buf, int siz) const;

		/**
		 * Get a list of database files.
		 * This function checks various paths for *.xml.
		 * If two files with the same filename are found,
		 * the one in the higher-precedence directory gets
		 * higher precedence.
		 * @return List of database files.
		 */
		static QVector<QString> GetDbFilenames(void);
};

#endif /* __MCRECOVER_GCNMCFILEDB_HPP__ */
