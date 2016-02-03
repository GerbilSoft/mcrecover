/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * GcnSearchThread.hpp: GCN "lost" file search thread.                      *
 *                                                                         *
 * Copyright (c) 2013-2016 by David Korth.                                 *
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

#ifndef __MCRECOVER_DB_GCNSEARCHTHREAD_HPP__
#define __MCRECOVER_DB_GCNSEARCHTHREAD_HPP__

// Card definitions.
#include "card.h"

// Search Data struct.
#include "GcnSearchData.hpp"

// Qt includes.
#include <QtCore/QLinkedList>
#include <QtCore/QObject>
#include <QtCore/QString>

class GcnCard;

class GcnSearchThreadPrivate;
class GcnSearchThread : public QObject
{
	Q_OBJECT

	public:
		GcnSearchThread(QObject *parent = 0);
		~GcnSearchThread();

	protected:
		GcnSearchThreadPrivate *const d_ptr;
		Q_DECLARE_PRIVATE(GcnSearchThread)
	private:
		Q_DISABLE_COPY(GcnSearchThread)

	signals:
		/**
		 * Search has started.
		 * @param totalPhysBlocks Total number of blocks in the card.
		 * @param totalSearchBlocks Number of blocks being searched.
		 * @param firstPhysBlock First block being searched.
		 */
		void searchStarted(int totalPhysBlocks, int totalSearchBlocks, int firstPhysBlock);

		/**
		 * Search has been cancelled.
		 */
		void searchCancelled(void);

		/**
		 * Search has completed.
		 * @param lostFilesFound Number of "lost" files found.
		 */
		void searchFinished(int lostFilesFound);

		/**
		 * Update search status.
		 * @param currentPhysBlock Current physical block number being searched.
		 * @param currentSearchBlock Number of blocks searched so far.
		 * @param lostFilesFound Number of "lost" files found.
		 */
		void searchUpdate(int currentPhysBlock, int currentSearchBlock, int lostFilesFound);

		/**
		 * An error has occurred during the search.
		 * @param errorString Error string.
		 */
		void searchError(QString errorString);

	public:
		/**
		 * Load a GCN Memory Card File database.
		 * @param dbFilename Filename of GCN Memory Card File database.
		 * @return 0 on success; non-zero on error. (Check error string!)
		 */
		inline int loadGcnMcFileDb(const QString &dbFilename);

		/**
		 * Load multiple GCN Memory Card File databases.
		 * @param dbFilenames Filenames of GCN Memory Card File database.
		 * @return 0 on success; non-zero on error. (Check error string!)
		 */
		int loadGcnMcFileDbs(const QVector<QString> &dbFilenames);

		/**
		 * Get the list of files found in the last successful search.
		 * @return List of files found.
		 */
		QLinkedList<GcnSearchData> filesFoundList(void);

		/**
		 * Search a memory card for "lost" files.
		 * Synchronous search; non-threaded.
		 * @param card Memory Card to search.
		 * @param preferredRegion Preferred region.
		 * @param searchUsedBlocks If true, search all blocks, not just blocks marked as empty.
		 * @return Number of files found on success; negative on error.
		 *
		 * If successful, retrieve the file list using dirEntryList().
		 * If an error occurs, check the errorString(). (TODO)
		 */
		int searchMemCard(GcnCard *card, char preferredRegion = 0, bool searchUsedBlocks = false);

		/**
		 * Search a memory card for "lost" files.
		 * Asynchronous search; uses a separate thread.
		 * @param card Memory Card to search.
		 * @param preferredRegion Preferred region.
		 * @param searchUsedBlocks If true, search all blocks, not just blocks marked as empty.
		 * @return 0 if thread started successfully; non-zero on error.
		 *
		 * Search is completed when either of the following
		 * signals are emitted:
		 * - searchCancelled(): Search was cancelled. No files found.
		 * - searchFinished(): Search has completed.
		 * - searchError(): Search failed due to an error.
		 *
		 * NOTE: If the search could not be done asynchronously, it will
		 * be done synchronously, though the signals will still be emitted.
		 *
		 * In the case of searchFinished(), use dirEntryList() to get
		 * the list of files.
		 */
		int searchMemCard_async(GcnCard *card, char preferredRegion = 0, bool searchUsedBlocks = false);

	private slots:
		/**
		 * Search has been cancelled.
		 */
		void searchCancelled_slot(void);

		/**
		 * Search has completed.
		 * @param lostFilesFound Number of "lost" files found.
		 */
		void searchFinished_slot(int lostFilesFound);

		/**
		 * An error has occurred during the search.
		 * @param errorString Error string.
		 */
		void searchError_slot(const QString &errorString);
};

/**
 * Load a GCN Memory Card File database.
 * @param dbFilename Filename of GCN Memory Card File database.
 * @return 0 on success; non-zero on error. (Check error string!)
 */
inline int GcnSearchThread::loadGcnMcFileDb(const QString &dbFilename)
{
	QVector<QString> dbFilenames;
	dbFilenames.append(dbFilename);
	return loadGcnMcFileDbs(dbFilenames);
}

#endif /* __MCRECOVER_SEARCHTHREAD_HPP__ */
