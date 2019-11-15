/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * GcnSearchWorker.hpp: GCN "lost" file search worker.                     *
 *                                                                         *
 * Copyright (c) 2013-2016 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#ifndef __MCRECOVER_DB_GCNSEARCHWORKER_HPP__
#define __MCRECOVER_DB_GCNSEARCHWORKER_HPP__

// Card definitions.
#include "card.h"

// Search Data struct.
#include "GcnSearchData.hpp"

// Qt includes.
#include <QtCore/QLinkedList>
#include <QtCore/QObject>
#include <QtCore/QString>

// Qt classes.
class QThread;

// Forward declarations.
class GcnCard;
class GcnMcFileDb;

class GcnSearchWorkerPrivate;
class GcnSearchWorker : public QObject
{
	Q_OBJECT
	typedef QObject super;

	Q_PROPERTY(QString errorString READ errorString)
	Q_PROPERTY(QLinkedList<GcnSearchData> filesFoundList READ filesFoundList)

	Q_PROPERTY(GcnCard* card READ card WRITE setCard)
	Q_PROPERTY(QVector<GcnMcFileDb*> databases READ databases WRITE setDatabases)
	Q_PROPERTY(char preferredRegion READ preferredRegion WRITE setPreferredRegion)
	Q_PROPERTY(bool searchUsedBlocks READ searchUsedBlocks WRITE setSearchUsedBlocks)
	Q_PROPERTY(QThread* origThread READ origThread WRITE setOrigThread)

	public:
		explicit GcnSearchWorker(QObject *parent = 0);
		virtual ~GcnSearchWorker();

	protected:
		GcnSearchWorkerPrivate *const d_ptr;
		Q_DECLARE_PRIVATE(GcnSearchWorker)
	private:
		Q_DISABLE_COPY(GcnSearchWorker)

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
		/** Read-only properties. **/

		/**
		 * Get the last error string.
		 *
		 * NOTE: This is NOT cleared if no error occurs.
		 * It should only be checked if an error occurred.
		 *
		 * @return Last error string.
		 */
		QString errorString(void) const;

		/**
		 * Get the list of files found in the last successful search.
		 * @return List of files found.
		 */
		QLinkedList<GcnSearchData> filesFoundList(void) const;

	public:
		/** Properties. **/

		/**
		 * Get the GcnCard.
		 * @return GcnCard.
		 */
		GcnCard *card(void) const;

		/**
		 * Set the GcnCard.
		 * @param card GcnCard.
		 */
		void setCard(GcnCard *card);

		/**
		 * Get the vector of GCN file databases.
		 * @return GCN file databases.
		 */
		QVector<GcnMcFileDb*> databases(void) const;

		/**
		 * Set the vector of GCN file databases.
		 * @param databases GCN file databases.
		 */
		void setDatabases(const QVector<GcnMcFileDb*> &databases);

		/**
		 * Get the preferred region.
		 * @return Preferred region.
		 */
		char preferredRegion(void) const;

		/**
		 * Set the preferred region.
		 * @param preferredRegion Preferred region.
		 */
		void setPreferredRegion(char preferredRegion);

		/**
		 * Search used blocks?
		 * @return True if searching used blocks; false if not.
		 */
		bool searchUsedBlocks(void) const;

		/**
		 * Should we search used blocks?
		 * @param searchUsedBlocks True to search used blocks; false to not.
		 */
		void setSearchUsedBlocks(bool searchUsedBlocks);

		/**
		 * Get the "original thread".
		 *
		 * This is the thread the object attaches to after
		 * the search is complete. If nullptr, no attachment
		 * will be done, and the data may be lost.
		 *
		 * @return Original thread.
		 */
		QThread *origThread(void) const;

		/**
		 * Set the "original thread".
		 *
		 * This is the thread the object attaches to after
		 * the search is complete. If nullptr, no attachment
		 * will be done, and the data may be lost.
		 *
		 * @param origThread Original thread.
		 */
		void setOrigThread(QThread *origThread);

	public:
		/** Search functions. **/

		/**
		 * Search a memory card for "lost" files.
		 * Properties must have been set previously.
		 * @return Number of files found on success; negative on error.
		 *
		 * If successful, retrieve the file list using filesEntryList().
		 * If an error occurs, check the errorString(). (TODO)
		 */
		int searchMemCard(void);

		/**
		 * Set internal information for threading purposes.
		 * This is basically the parameters to searchMemCard().
		 * We can't pass these when starting the thread, so
		 * we have to set them up first.
		 * @param card Memory Card to search.
		 * @param dbs Vector of GcnMcFileDb to use.
		 * @param orig_thread Thread to move back to once completed.
		 * @param preferredRegion Preferred region.
		 * @param searchUsedBlocks If true, search all blocks, not just blocks marked as empty.
		 */
		void setThreadInfo(GcnCard *card, const QVector<GcnMcFileDb*> &dbs,
				QThread *orig_thread,
				char preferredRegion = 0, bool searchUsedBlocks = false);

	public slots:
		/**
		 * Search the memory card for "lost" files.
		 * This version should be connected to a QThread's SIGNAL(started()).
		 * Thread information must have been set using setThreadInfo().
		 */
		void searchMemCard_threaded(void);
};

#endif /* __MCRECOVER_DB_GCNSEARCHWORKER_HPP__ */
