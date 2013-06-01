/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * SearchThread.cpp: "Lost" file search thread.                            *
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

#include "SearchThread.hpp"

// MemCard
#include "MemCard.hpp"

// GCN Memory Card File Database.
#include "GcnMcFileDb.hpp"

// Worker object.
#include "SearchThreadWorker.hpp"

// C includes.
#include <cstdio>

// Qt includes.
#include <QtCore/QLinkedList>
#include <QtCore/QStack>
#include <QtCore/QThread>

// TODO: Remove this...
#include <QtGui/QMessageBox>

class SearchThreadPrivate
{
	public:
		SearchThreadPrivate(SearchThread *q);
		~SearchThreadPrivate();

	private:
		SearchThread *const q;
		Q_DISABLE_COPY(SearchThreadPrivate);

	public:
		// GCN Memory Card File Database.
		GcnMcFileDb *db;

		// Worker object.
		SearchThreadWorker *worker;
};


SearchThreadPrivate::SearchThreadPrivate(SearchThread* q)
	: q(q)
	, db(new GcnMcFileDb(q))
	, worker(new SearchThreadWorker(q))
{
	// Signal passthrough.
	QObject::connect(worker, SIGNAL(searchStarted(int,int,int)),
			 q, SIGNAL(searchStarted(int,int,int)));
	QObject::connect(worker, SIGNAL(searchCancelled()),
			 q, SIGNAL(searchCancelled()));
	QObject::connect(worker, SIGNAL(searchFinished(int)),
			 q, SIGNAL(searchFinished(int)));
	QObject::connect(worker, SIGNAL(searchUpdate(int,int,int)),
			 q, SIGNAL(searchUpdate(int,int,int)));
	QObject::connect(worker, SIGNAL(searchError(QString)),
			 q, SIGNAL(searchError(QString)));
}	

SearchThreadPrivate::~SearchThreadPrivate()
{
	delete worker;
	delete db;
}


/** SearchThread **/

SearchThread::SearchThread(QObject *parent)
	: QObject(parent)
	, d(new SearchThreadPrivate(this))
{ }

SearchThread::~SearchThread()
{
	delete d;
}


/**
 * Load a GCN Memory Card File database.
 * @param filename Filename of GCN Memory Card File database.
 * @return 0 on success; non-zero on error. (Check error string!)
 */
int SearchThread::loadGcnMcFileDb(QString filename)
{
	int ret = d->db->load(filename);
	if (ret != 0) {
		// TODO: Set the error string.
		// For now, just show a message box.
		QMessageBox::critical(NULL,
			tr("Database Load Error"),
			tr("Error loading the GCN Memory Card File database:") +
			QLatin1String("\n\n") + d->db->errorString());
	}

	return ret;
}


/**
 * Get the list of directory entries from the last successful search.
 * @return List of directory entries.
 */
QLinkedList<card_direntry> SearchThread::dirEntryList(void)
{
	// TODO: Not while thread is running...
	return d->worker->dirEntryList();
}


/**
 * Search a memory card for "lost" files.
 * Synchronous search; non-threaded.
 * @param card Memory Card to search.
 * @return Number of files found on success; negative on error.
 *
 * If successful, retrieve the file list using dirEntryList().
 * If an error occurs, check the errorString(). (TODO)(
 */
int SearchThread::searchMemCard(MemCard *card)
{
	// Search for files.
	// TODO: Not while thread is running...
	return d->worker->searchMemCard(card, d->db);
}


/**
 * Search a memory card for "lost" files.
 * Asynchronous search; uses a separate thread.
 * @param card Memory Card to search.
 * @return 0 if thread started successfully; non-zero on error.
 *
 * Search is completed when either of the following
 * signals are emitted:
 * - searchCancelled(): Search was cancelled. No files found.
 * - searchFinished(): Search has completed.
 * - searchError(): Search failed due to an error.
 *
 * In the case of searchFinished(), use dirEntryList() to get
 * the list of files.
 */
int SearchThread::searchMemCard_async(MemCard *card)
{
	// TODO
	return -1;
}
