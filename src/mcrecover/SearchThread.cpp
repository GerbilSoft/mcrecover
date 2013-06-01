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

// C includes.
#include <cstdio>

// Qt includes.
#include <QtCore/QLinkedList>
#include <QtCore/QStack>

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

		/**
		 * Search a memory card for "lost" files.
		 * Internal function used by both threaded and non-threaded versions.
		 * @param card Memory Card to search.
		 * @return List of "lost" files.
		 *
		 * If an error occurs, an empty list will be returned,
		 * and an error string will be set. (TODO)
		 */
		QLinkedList<card_direntry> searchMemCard_int(MemCard *card);
};


SearchThreadPrivate::SearchThreadPrivate(SearchThread* q)
	: q(q)
	, db(new GcnMcFileDb(q))
{ }

SearchThreadPrivate::~SearchThreadPrivate()
{
	delete db;
}


/**
 * Search a memory card for "lost" files.
 * Internal function used by both threaded and non-threaded versions.
 * @param card Memory Card to search.
 * @param emitSignals If true, emit signals.
 * @return List of "lost" files.
 */
QLinkedList<card_direntry> SearchThreadPrivate::searchMemCard_int(MemCard *card)
{
	if (!db) {
		// Database is not loaded.
		// TODO: Set an error string somewhere.
		return QLinkedList<card_direntry>();
	}

	// Search blocks for lost files.
	const int blockSize = card->blockSize();
	void *buf = malloc(blockSize);

	// List of directory entries.
	// QLinkedList allows us to prepend items, so we do that
	// in order to turn "reverse-order" into "correct-order".
	// TODO: Use malloc()'d dirEntry?
	QLinkedList<card_direntry> dirEntries;

	// Current directory entry.
	card_direntry dirEntry;

	fprintf(stderr, "--------------------------------\n");
	fprintf(stderr, "SCANNING MEMORY CARD...\n");

	// TODO: totalSearchBlocks should be based on used block map.
	const int totalBlocks = card->sizeInBlocks();
	const int totalSearchBlocks = (card->sizeInBlocks() - 5);
	emit q->searchStarted(totalBlocks, totalSearchBlocks);

	int currentSearchBlock = 0;
	for (int i = (totalBlocks - 1); i > 5; i--, currentSearchBlock++) {
		fprintf(stderr, "Searching block: %d...\n", i);
		emit q->searchUpdate(i, currentSearchBlock, totalSearchBlocks, dirEntries.size());

		int ret = card->readBlock(buf, blockSize, i);
		if (ret != blockSize) {
			// Error reading block.
			fprintf(stderr, "ERROR reading block %d - readBlock() returned %d.\n", i, ret);
			continue;
		}

		// Check the block in the database.
		ret = db->checkBlock(buf, blockSize, &dirEntry);
		if (!ret) {
			// Matched!
			fprintf(stderr, "FOUND A MATCH: %-.4s%-.2s %-.32s\n",
				dirEntry.gamecode,
				dirEntry.company,
				dirEntry.filename);
			fprintf(stderr, "bannerFmt == %02X, iconAddress == %08X, iconFormat == %02X, iconSpeed == %02X\n",
				dirEntry.bannerfmt, dirEntry.iconaddr, dirEntry.iconfmt, dirEntry.iconspeed);

			// NOTE: dirEntry's block start is not set by d->db->checkBlock().
			// Set it here.
			dirEntry.block = i;
			if (dirEntry.length == 0) {
				// This only happens if an entry is either
				// missing a <dirEntry>, or has <length>0</length>.
				// TODO: Check for this in GcnMcFileDb.
				dirEntry.length = 1;
			}

			// Add the directory entry to the list.
			dirEntries.prepend(dirEntry);
		}
	}

	emit q->searchFinished(dirEntries);

	fprintf(stderr, "Finished scanning memory card.\n");
	fprintf(stderr, "--------------------------------\n");
	return dirEntries;
}


/** SearchThread **/

SearchThread::SearchThread(QObject *parent)
	: QThread(parent)
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
 * Search a memory card for "lost" files.
 * Synchronous search; non-threaded.
 * @param card Memory Card to search.
 * @return List of "lost" files.
 */
QLinkedList<card_direntry> SearchThread::searchMemCard(MemCard *card)
{
	return d->searchMemCard_int(card);
}
