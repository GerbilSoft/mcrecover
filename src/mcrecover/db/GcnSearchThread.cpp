/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * GcnSearchThread.cpp: GCN "lost" file search thread.                     *
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

#include "GcnSearchThread.hpp"

// GcnCard
#include "card/GcnCard.hpp"

// GCN Memory Card File Database.
#include "db/GcnMcFileDb.hpp"

// Worker object.
#include "GcnSearchWorker.hpp"

// Qt includes.
#include <QtCore/QLinkedList>
#include <QtCore/QStack>
#include <QtCore/QThread>

class GcnSearchThreadPrivate
{
	public:
		explicit GcnSearchThreadPrivate(GcnSearchThread *q);
		~GcnSearchThreadPrivate();

	protected:
		GcnSearchThread *const q_ptr;
		Q_DECLARE_PUBLIC(GcnSearchThread)
	private:
		Q_DISABLE_COPY(GcnSearchThreadPrivate)

	public:
		// GCN Memory Card File databases.
		QVector<GcnMcFileDb*> dbs;

		// Worker object.
		// NOTE: This object cannot have a parent;
		// otherwise, QObject::moveToThread() won't work.
		GcnSearchWorker *worker;

		// Worker thread.
		QThread *workerThread;

		/**
		 * Stop the worker thread.
		 */
		void stopWorkerThread(void);
};

GcnSearchThreadPrivate::GcnSearchThreadPrivate(GcnSearchThread* q)
	: q_ptr(q)
	, worker(new GcnSearchWorker())
	, workerThread(nullptr)
{
	// Signal passthrough.
	QObject::connect(worker, &GcnSearchWorker::searchStarted,
			 q, &GcnSearchThread::searchStarted);
	QObject::connect(worker, &GcnSearchWorker::searchUpdate,
			 q, &GcnSearchThread::searchUpdate);

	// We have to handle these signals in order to move
	// the worker object back to the main thread.
	QObject::connect(worker, &GcnSearchWorker::searchCancelled,
			 q, &GcnSearchThread::searchCancelled_slot);
	QObject::connect(worker, &GcnSearchWorker::searchFinished,
			 q, &GcnSearchThread::searchFinished_slot);
	QObject::connect(worker, &GcnSearchWorker::searchError,
			 q, &GcnSearchThread::searchError_slot);
}	

GcnSearchThreadPrivate::~GcnSearchThreadPrivate()
{
	delete worker;
	qDeleteAll(dbs);
	dbs.clear();
}

/**
 * Stop the worker thread.
 */
void GcnSearchThreadPrivate::stopWorkerThread(void)
{
	if (!workerThread)
		return;

	workerThread->quit();
	workerThread->wait();
	// TODO: Maybe we should keep the thread allocated?
	delete workerThread;
	workerThread = nullptr;
}

/** GcnSearchThread **/

GcnSearchThread::GcnSearchThread(QObject *parent)
	: super(parent)
	, d_ptr(new GcnSearchThreadPrivate(this))
{ }

GcnSearchThread::~GcnSearchThread()
{
	Q_D(GcnSearchThread);
	delete d;
}

/** Read-only properties. **/

/**
 * Get the last error string.
 *
 * NOTE: This is NOT cleared if no error occurs.
 * It should only be checked if an error occurred.
 *
 * @return Last error string.
 */
QString GcnSearchThread::errorString(void) const
{
	Q_D(const GcnSearchThread);
	return d->worker->errorString();
}

/** Functions. **/

/**
 * Load multiple GCN Memory Card File databases.
 * @param dbFilenames Filenames of GCN Memory Card File database.
 * @return 0 on success; non-zero on error. (Check error string!)
 */
int GcnSearchThread::loadGcnMcFileDbs(const QVector<QString> &dbFilenames)
{
	Q_D(GcnSearchThread);
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
QLinkedList<GcnSearchData> GcnSearchThread::filesFoundList(void)
{
	// TODO: Not while thread is running...
	Q_D(GcnSearchThread);
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
int GcnSearchThread::searchMemCard(GcnCard *card, char preferredRegion, bool searchUsedBlocks)
{
	Q_D(GcnSearchThread);

	// TODO: Mutex?
	if (d->workerThread) {
		// Thread is running.
		return -255;	// TODO: Error code constant?
	}

	// Don't do anything if no databases are loaded.
	if (d->dbs.isEmpty())
		return 0;

	// Set the GcnSearchWorker's properties.
	d->worker->setCard(card);
	d->worker->setDatabases(d->dbs);
	d->worker->setPreferredRegion(preferredRegion);
	d->worker->setSearchUsedBlocks(searchUsedBlocks);
	d->worker->setOrigThread(nullptr);

	// Search for files.
	return d->worker->searchMemCard();
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
int GcnSearchThread::searchMemCard_async(GcnCard *card, char preferredRegion, bool searchUsedBlocks)
{
	Q_D(GcnSearchThread);

	// TODO: Mutex?
	if (d->workerThread) {
		// Thread is already running.
		return -255;	// TODO: Error code constant?
	}

	// Don't do anything if no databases are loaded.
	if (d->dbs.isEmpty())
		return 0;

	// Set up the worker thread.
	// TODO: Do synchronous search if this fails.
	d->workerThread = new QThread(this);
	d->worker->moveToThread(d->workerThread);

	// Set the GcnSearchWorker's properties.
	d->worker->setCard(card);
	d->worker->setDatabases(d->dbs);
	d->worker->setPreferredRegion(preferredRegion);
	d->worker->setSearchUsedBlocks(searchUsedBlocks);
	d->worker->setOrigThread(QThread::currentThread());

	connect(d->workerThread, &QThread::started,
		d->worker, &GcnSearchWorker::searchMemCard_threaded);

	// Start the thread.
	d->workerThread->start();

	// Thread initialized successfully.
	return 0;
}

/** Slots. **/

/**
 * Search has been cancelled.
 */
void GcnSearchThread::searchCancelled_slot(void)
{
	Q_D(GcnSearchThread);
	if (d->workerThread) {
		// Worker moved itself back to this thread.
		d->stopWorkerThread();
	}

	emit searchCancelled();
}

/**
 * Search has completed.
 * @param lostFilesFound Number of "lost" files found.
 */
void GcnSearchThread::searchFinished_slot(int lostFilesFound)
{
	Q_D(GcnSearchThread);
	if (d->workerThread) {
		// Worker moved itself back to this thread.
		d->stopWorkerThread();
	}

	emit searchFinished(lostFilesFound);
}

/**
 * An error has occurred during the search.
 * @param errorString Error string.
 */
void GcnSearchThread::searchError_slot(const QString &errorString)
{
	Q_D(GcnSearchThread);
	if (d->workerThread) {
		// Worker moved itself back to this thread.
		d->stopWorkerThread();
	}

	emit searchError(errorString);
}
