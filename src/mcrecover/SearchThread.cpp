/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * SearchThread.cpp: "Lost" file search thread.                            *
 *                                                                         *
 * Copyright (c) 2013-2015 by David Korth.                                 *
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

#include "SearchThread.hpp"

// GcnCard
#include "card/GcnCard.hpp"

// GCN Memory Card File Database.
#include "db/GcnMcFileDb.hpp"

// Worker object.
#include "SearchThreadWorker.hpp"

// Qt includes.
#include <QtCore/QLinkedList>
#include <QtCore/QStack>
#include <QtCore/QThread>

class SearchThreadPrivate
{
	public:
		SearchThreadPrivate(SearchThread *q);
		~SearchThreadPrivate();

	protected:
		SearchThread *const q_ptr;
		Q_DECLARE_PUBLIC(SearchThread)
	private:
		Q_DISABLE_COPY(SearchThreadPrivate)

	public:
		// GCN Memory Card File databases.
		QVector<GcnMcFileDb*> dbs;

		// Worker object.
		// NOTE: This object cannot have a parent;
		// otherwise, QObject::moveToThread() won't work.
		SearchThreadWorker *worker;

		// Worker thread.
		QThread *workerThread;

		/**
		 * Stop the worker thread.
		 */
		void stopWorkerThread(void);
};

SearchThreadPrivate::SearchThreadPrivate(SearchThread* q)
	: q_ptr(q)
	, worker(new SearchThreadWorker())
	, workerThread(nullptr)
{
	// Signal passthrough.
	QObject::connect(worker, SIGNAL(searchStarted(int,int,int)),
			 q, SIGNAL(searchStarted(int,int,int)));
	QObject::connect(worker, SIGNAL(searchUpdate(int,int,int)),
			 q, SIGNAL(searchUpdate(int,int,int)));

	// We have to handle these signals in order to move
	// the worker object back to the main thread.
	QObject::connect(worker, SIGNAL(searchCancelled()),
			 q, SLOT(searchCancelled_slot()));
	QObject::connect(worker, SIGNAL(searchFinished(int)),
			 q, SLOT(searchFinished_slot(int)));
	QObject::connect(worker, SIGNAL(searchError(QString)),
			 q, SLOT(searchError_slot(QString)));
}	

SearchThreadPrivate::~SearchThreadPrivate()
{
	delete worker;
	qDeleteAll(dbs);
	dbs.clear();
}

/**
 * Stop the worker thread.
 */
void SearchThreadPrivate::stopWorkerThread(void)
{
	if (!workerThread)
		return;

	workerThread->quit();
	workerThread->wait();
	// TODO: Maybe we should keep the thread allocated?
	delete workerThread;
	workerThread = nullptr;
}

/** SearchThread **/

SearchThread::SearchThread(QObject *parent)
	: QObject(parent)
	, d_ptr(new SearchThreadPrivate(this))
{ }

SearchThread::~SearchThread()
{
	Q_D(SearchThread);
	delete d;
}

/**
 * Load multiple GCN Memory Card File databases.
 * @param dbFilenames Filenames of GCN Memory Card File database.
 * @return 0 on success; non-zero on error. (Check error string!)
 */
int SearchThread::loadGcnMcFileDbs(const QVector<QString> &dbFilenames)
{
	Q_D(SearchThread);
	qDeleteAll(d->dbs);
	d->dbs.clear();

	if (dbFilenames.isEmpty())
		return 0;

	// Load the databases.
	foreach (QString dbFilename, dbFilenames) {
		GcnMcFileDb *db = new GcnMcFileDb(this);
		int ret = db->load(dbFilename);
		if (!ret)
			d->dbs.append(db);
		else
			delete db;
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
 * Get the list of files found in the last successful search.
 * @return List of files found.
 */
QLinkedList<SearchData> SearchThread::filesFoundList(void)
{
	// TODO: Not while thread is running...
	Q_D(SearchThread);
	return d->worker->filesFoundList();
}

/**
 * Search a memory card for "lost" files.
 * Synchronous search; non-threaded.
 * @param card Memory Card to search
 * @param preferredRegion Preferred region.
 * @param searchUsedBlocks If true, search all blocks, not just blocks marked as empty.
 * @return Number of files found on success; negative on error.
 *
 * If successful, retrieve the file list using dirEntryList().
 * If an error occurs, check the errorString(). (TODO)
 */
int SearchThread::searchMemCard(GcnCard *card, char preferredRegion, bool searchUsedBlocks)
{
	Q_D(SearchThread);

	// TODO: Mutex?
	if (d->workerThread) {
		// Thread is running.
		return -255;	// TODO: Error code constant?
	}

	// Don't do anything if no databases are loaded.
	if (d->dbs.isEmpty())
		return 0;

	// Search for files.
	return d->worker->searchMemCard(card, d->dbs, preferredRegion, searchUsedBlocks);
}

/**
 * Search a memory card for "lost" files.
 * Asynchronous search; uses a separate thread.
 * @param card Memory Card to search.
 * @param preferredRegion Preferred region.
 * @param searchUsedBlocks If true, search all blocks, not just empty blocks.
 * @return 0 if the thread started successfully; non-zero on error.
 *
 * Search is completed when either of the following
 * signals are emitted:
 * - searchCancelled(): Search was cancelled. No files found.
 * - searchFinished(): Search has completed.
 * - searchError(): Search failed due to an error.
 */
int SearchThread::searchMemCard_async(GcnCard *card, char preferredRegion, bool searchUsedBlocks)
{
	Q_D(SearchThread);

	// TODO: Mutex?
	if (d->workerThread) {
		// Thread is already running.
		return -255;	// TODO: Error code constant?
	}

	// Don't do anything if no databases are loaded.
	if (d->dbs.isEmpty())
		return 0;

	// Set up the worker thread.
	d->workerThread = new QThread(this);
	d->worker->moveToThread(d->workerThread);
	d->worker->setThreadInfo(card, d->dbs, QThread::currentThread(), preferredRegion, searchUsedBlocks);

	connect(d->workerThread, SIGNAL(started()),
		d->worker, SLOT(searchMemCard_threaded()));

	// Start the thread.
	d->workerThread->start();

	// Thread initialized successfully.
	return 0;
}

/** Slots. **/

/**
 * Search has been cancelled.
 */
void SearchThread::searchCancelled_slot(void)
{
	Q_D(SearchThread);
	if (d->workerThread)
		d->stopWorkerThread();

	emit searchCancelled();
}

/**
 * Search has completed.
 * @param lostFilesFound Number of "lost" files found.
 */
void SearchThread::searchFinished_slot(int lostFilesFound)
{
	Q_D(SearchThread);
	if (d->workerThread)
		d->stopWorkerThread();

	emit searchFinished(lostFilesFound);
}

/**
 * An error has occurred during the search.
 * @param errorString Error string.
 */
void SearchThread::searchError_slot(const QString &errorString)
{
	Q_D(SearchThread);
	if (d->workerThread) {
		d->worker->moveToThread(QThread::currentThread());
		d->stopWorkerThread();
	}

	emit searchError(errorString);
}
