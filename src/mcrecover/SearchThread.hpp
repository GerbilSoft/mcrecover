/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * SearchThread.hpp: "Lost" file search thread.                            *
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

#ifndef __MCRECOVER_SEARCHTHREAD_HPP__
#define __MCRECOVER_SEARCHTHREAD_HPP__

// Card definitions.
#include "card.h"

// Qt includes.
#include <QtCore/QLinkedList>
#include <QtCore/QString>
#include <QtCore/QThread>

// MemCard
class MemCard;

// SearchThread private class.
class SearchThreadPrivate;

class SearchThread : public QThread
{
	Q_OBJECT

	public:
		SearchThread(QObject *parent = 0);
		~SearchThread();

	private:
		friend class SearchThreadPrivate;
		SearchThreadPrivate *const d;
		Q_DISABLE_COPY(SearchThread);

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
		 * Search a memory card for "lost" files.
		 * Synchronous search; non-threaded.
		 * @param card Memory Card to search.
		 * @return Number of files found on success; negative on error.
		 *
		 * If successful, retrieve the file list using dirEntryList().
		 * If an error occurs, check the errorString(). (TODO)(
		 */
		int searchMemCard(MemCard *card);

		/**
		 * Get the list of directory entries from the last successful search.
		 * @return List of directory entries.
		 */
		QLinkedList<card_direntry> dirEntryList(void);
};

#endif /* __MCRECOVER_SEARCHTHREAD_HPP__ */
