/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * SearchThreadWorker.hpp: SearchThread "worker" object.                   *
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

#ifndef __MCRECOVER_SEARCHTHREADWORKER_HPP__
#define __MCRECOVER_SEARCHTHREADWORKER_HPP__

// Card definitions.
#include "card.h"

// Search Data struct.
#include "SearchData.hpp"

// Qt includes.
#include <QtCore/QLinkedList>
#include <QtCore/QObject>
#include <QtCore/QString>

// Qt classes.
class QThread;

// Forward declarations.
class MemCard;
class GcnMcFileDb;

// SearchThreadWorker private class.
class SearchThreadWorkerPrivate;

class SearchThreadWorker : public QObject
{
	Q_OBJECT

	public:
		SearchThreadWorker(QObject *parent = 0);
		~SearchThreadWorker();

	private:
		friend class SearchThreadWorkerPrivate;
		SearchThreadWorkerPrivate *const d;
		Q_DISABLE_COPY(SearchThreadWorker);

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
		 * @param filename Filename of GCN Memory Card File database.
		 * @return 0 on success; non-zero on error. (Check error string!)
		 */
		int loadGcnMcFileDb(QString filename);

		/**
		 * Get the list of files found in the last successful search.
		 * @return List of files found.
		 */
		QLinkedList<SearchData> filesFoundList(void);

		/**
		 * Search a memory card for "lost" files.
		 * @param card Memory Card to search.
		 * @param db GcnMcFileDb to use.
		 * @param searchUsedBlocks If true, search all blocks, not just empty blocks.
		 * @return Number of files found on success; negative on error.
		 *
		 * If successful, retrieve the file list using dirEntryList().
		 * If an error occurs, check the errorString(). (TODO)(
		 */
		int searchMemCard(MemCard *card, const GcnMcFileDb *db, bool searchUsedBlocks = false);

		/**
		 * Set internal information for threading purposes.
		 * This is basically the parameters to searchMemCard().
		 * We can't pass these when starting the thread, so
		 * we have to set them up first.
		 * @param card Memory Card to search.
		 * @param db GcnMcFileDb to use.
		 * @param orig_thread Thread to move back to once completed.
		 * @param searchUsedBlocks If true, search all blocks, not just empty blocks.
		 */
		void setThreadInfo(MemCard *card, const GcnMcFileDb *db,
				   QThread *orig_thread, bool searchUsedBlocks = false);

	public slots:
		/**
		 * Search the memory card for "lost" files.
		 * This version should be connected to a QThread's SIGNAL(started()).
		 * Thread information must have been set using setThreadInfo().
		 */
		void searchMemCard_threaded(void);
};

#endif /* __MCRECOVER_SEARCHTHREADWORKER_HPP__ */
