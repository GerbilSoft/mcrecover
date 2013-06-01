/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * SearchThreadWorker.cpp: SearchThread "worker" object.                   *
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

#include "SearchThreadWorker.hpp"

// MemCard
#include "MemCard.hpp"

// GCN Memory Card File Database
#include "GcnMcFileDb.hpp"

// C includes.
#include <cstdio>

// Qt includes.
#include <QtCore/QLinkedList>
#include <QtCore/QStack>

class SearchThreadWorkerPrivate
{
	public:
		SearchThreadWorkerPrivate(SearchThreadWorker *q);

	private:
		SearchThreadWorker *const q;
		Q_DISABLE_COPY(SearchThreadWorkerPrivate);

	public:
		// List of directory entries from the last successful search.
		// QLinkedList allows us to prepend items, so we do that
		// in order to turn "reverse-order" into "correct-order".
		// TODO: Use malloc()'d dirEntry?
		QLinkedList<card_direntry> dirEntryList;
};


SearchThreadWorkerPrivate::SearchThreadWorkerPrivate(SearchThreadWorker* q)
	: q(q)
{ }


/** SearchThreadWorker **/

SearchThreadWorker::SearchThreadWorker(QObject *parent)
	: QObject(parent)
	, d(new SearchThreadWorkerPrivate(this))
{ }

SearchThreadWorker::~SearchThreadWorker()
{
	delete d;
}


/**
 * Get the list of directory entries from the last successful search.
 * @return List of directory entries.
 */
QLinkedList<card_direntry> SearchThreadWorker::dirEntryList(void)
{
	// TODO: Not while thread is running...
	return d->dirEntryList;
}


/**
 * Search a memory card for "lost" files.
 * @param card Memory Card to search.
 * @param db GcnMcFileDb to use.
 * @return Number of files found on success; negative on error.
 *
 * If successful, retrieve the file list using dirEntryList().
 * If an error occurs, check the errorString(). (TODO)(
 */
int SearchThreadWorker::searchMemCard(MemCard *card, const GcnMcFileDb *db)
{
	d->dirEntryList.clear();

	if (!db) {
		// Database is not loaded.
		// TODO: Set an error string somewhere.
		return -1;
	}

	// Search blocks for lost files.
	const int blockSize = card->blockSize();
	void *buf = malloc(blockSize);

	// Current directory entry.
	card_direntry dirEntry;

	fprintf(stderr, "--------------------------------\n");
	fprintf(stderr, "SCANNING MEMORY CARD...\n");

	// TODO: totalSearchBlocks should be based on used block map.
	const int totalPhysBlocks = card->sizeInBlocks();
	const int totalSearchBlocks = (card->sizeInBlocks() - 5);
	const int firstPhysBlock = (totalPhysBlocks - 1);
	emit searchStarted(totalPhysBlocks, totalSearchBlocks, firstPhysBlock);

	int currentSearchBlock = 0;
	for (int i = firstPhysBlock; i >= 5; i--, currentSearchBlock++) {
		fprintf(stderr, "Searching block: %d...\n", i);
		emit searchUpdate(i, currentSearchBlock, d->dirEntryList.size());

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
			d->dirEntryList.prepend(dirEntry);
		}
	}

	emit searchFinished(d->dirEntryList.size());

	fprintf(stderr, "Finished scanning memory card.\n");
	fprintf(stderr, "--------------------------------\n");
	return d->dirEntryList.size();
}
