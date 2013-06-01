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

// C++ includes.
#include <limits>

// Qt includes.
#include <QtCore/QLinkedList>
#include <QtCore/QVector>


class SearchThreadWorkerPrivate
{
	public:
		SearchThreadWorkerPrivate(SearchThreadWorker *q);

	private:
		SearchThreadWorker *const q;
		Q_DISABLE_COPY(SearchThreadWorkerPrivate);

	public:
		/**
		 * List of directory entries from the last successful search.
		 * QLinkedList allows us to prepend items, so we do that
		 * in order to turn "reverse-order" into "correct-order".
		 * TODO: Use malloc()'d dirEntry?
		 */
		QLinkedList<card_direntry> dirEntryList;

		/**
		 * List of FAT entries for directory entries.
		 */
		QLinkedList<QVector<uint16_t> > fatEntriesList;

		// searchMemCard() parameters used when this worker
		// is called by a thread's started() signal.
		struct {
			MemCard *card;
			const GcnMcFileDb *db;

			// Original thread.
			QThread *orig_thread;

			// If true, search all blocks, not just empty blocks.
			bool searchUsedBlocks;
		} thread_info;
};


SearchThreadWorkerPrivate::SearchThreadWorkerPrivate(SearchThreadWorker* q)
	: q(q)
{
	// NULL these out by default.
	thread_info.card = NULL;
	thread_info.db = NULL;
	thread_info.orig_thread = NULL;
	thread_info.searchUsedBlocks = false;
}


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
 * Get the list of FAT entries from the last successful search.
 * @return List of FAT entries.
 */
QLinkedList<QVector<uint16_t> > SearchThreadWorker::fatEntriesList(void)
{
	// TODO: Not while thread is running...
	return d->fatEntriesList;
}


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
int SearchThreadWorker::searchMemCard(MemCard *card, const GcnMcFileDb *db,
				      bool searchUsedBlocks)
{
	d->dirEntryList.clear();
	d->fatEntriesList.clear();

	if (!db) {
		// Database is not loaded.
		// TODO: Set an error string somewhere.
		return -1;
	}

	// Block search list.
	QVector<uint16_t> blockSearchList;
	const int totalPhysBlocks = card->sizeInBlocks();

	// Used block map.
	QVector<uint8_t> usedBlockMap;
	if (!searchUsedBlocks) {
		// Only search empty blocks.
		usedBlockMap = card->usedBlockMap();

		// Put together a block search list.
		blockSearchList.reserve(totalPhysBlocks - card->freeBlocks() - 5);
		for (int i = (usedBlockMap.size() - 1); i >= 5; i--) {
			if (usedBlockMap[i] == 0)
				blockSearchList.append((uint16_t)i);
		}
	} else {
		// Search through all blocks.
		usedBlockMap = QVector<uint8_t>(card->sizeInBlocks(), 0);

		// Put together a block search list.
		blockSearchList.reserve(totalPhysBlocks - 5);
		for (int i = (usedBlockMap.size() - 1); i >= 5; i--) {
			blockSearchList.append((uint16_t)i);
		}
	}

	if (blockSearchList.isEmpty()) {
		// Should not happen...
		// TODO: Set an error string somewhere.
		return -2;
	}

	// Current directory entry.
	card_direntry dirEntry;

	// Block buffer.
	const int blockSize = card->blockSize();
	void *buf = malloc(blockSize);

	fprintf(stderr, "--------------------------------\n");
	fprintf(stderr, "SCANNING MEMORY CARD...\n");

	// TODO: totalSearchBlocks should be based on used block map.
	const int totalSearchBlocks = blockSearchList.size();
	int currentPhysBlock = blockSearchList.value(0);
	emit searchStarted(totalPhysBlocks, totalSearchBlocks, currentPhysBlock);

	int currentSearchBlock = -1;	// compensate for currentSearchBlock++
	foreach (currentPhysBlock, blockSearchList) {
		currentSearchBlock++;
		fprintf(stderr, "Searching block: %d...\n", currentPhysBlock);
		emit searchUpdate(currentPhysBlock, currentSearchBlock, d->dirEntryList.size());

		int ret = card->readBlock(buf, blockSize, currentPhysBlock);
		if (ret != blockSize) {
			// Error reading block.
			fprintf(stderr, "ERROR reading block %d - readBlock() returned %d.\n", currentPhysBlock, ret);
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
			dirEntry.block = currentPhysBlock;
			if (dirEntry.length == 0) {
				// This only happens if an entry is either
				// missing a <dirEntry>, or has <length>0</length>.
				// TODO: Check for this in GcnMcFileDb.
				dirEntry.length = 1;
			}

			// Construct the FAT entries for this file.
			QVector<uint16_t> fatEntries;
			fatEntries.reserve(dirEntry.length);

			// First block is always valid.
			fatEntries.append(dirEntry.block);

			uint16_t blocksRemaining = (dirEntry.length - 1);
			uint16_t block = (dirEntry.block + 1);
			bool wasWrapped = false;

			// Skip used blocks and go after empty blocks only.
			while (blocksRemaining > 0) {
				if (block >= totalPhysBlocks) {
					// Wraparound.
					// Do NOT mark the wrapped blocks as used,
					// since they might be used by actual files.
					block = 5;
					wasWrapped = true;
					continue;
				} else if (block == dirEntry.block) {
					// ERROR: We wrapped around!
					// Use the "naive" algorithm after the last valid block.
					break;
				}

				// Check if this block is used.
				if (usedBlockMap[block] == 0) {
					// Block is not used.
					printf("next block: %04X\n", block);
					fatEntries.append(block);
					if (!wasWrapped)
						usedBlockMap[block]++;
					blocksRemaining--;
				}

				// Next block.
				block++;
			}

			// Naive block algorithm for the remaining blocks.
			block = (fatEntries.value(fatEntries.size() - 1) + 1);
			wasWrapped = false;
			while (blocksRemaining > 0) {
				if (block >= totalPhysBlocks) {
					// Wraparound.
					// Do NOT mark the wrapped blocks as used,
					// since they might be used by actual files.
					block = 5;
					continue;
				}

				// Add this block.
				fatEntries.append(block);
				if (usedBlockMap[block] < std::numeric_limits<uint8_t>::max()) {
					if (!wasWrapped)
						usedBlockMap[block]++;
				}
				block++;
				blocksRemaining--;
			}

			// Add the directory entry to the list.
			d->dirEntryList.prepend(dirEntry);

			// Add the FAT entries to the list.
			d->fatEntriesList.prepend(fatEntries);
		}
	}

	// Send an update for the last block.
	emit searchUpdate(5, currentSearchBlock, d->dirEntryList.size());

	// Search is finished.
	emit searchFinished(d->dirEntryList.size());

	fprintf(stderr, "Finished scanning memory card.\n");
	fprintf(stderr, "--------------------------------\n");
	return d->dirEntryList.size();
}


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
void SearchThreadWorker::setThreadInfo(MemCard *card, const GcnMcFileDb *db,
				       QThread *orig_thread, bool searchUsedBlocks)
{
	d->thread_info.card = card;
	d->thread_info.db = db;
	d->thread_info.orig_thread = orig_thread;
	d->thread_info.searchUsedBlocks = searchUsedBlocks;
}


/**
 * Search the memory card for "lost" files.
 * This version should be connected to a QThread's SIGNAL(started()).
 * Thread information must have been set using setThreadInfo().
 */
void SearchThreadWorker::searchMemCard_threaded(void)
{
	if (!d->thread_info.card ||
	    !d->thread_info.db ||
	    !d->thread_info.orig_thread)
	{
		// Thread information was not set.
		if (d->thread_info.orig_thread) {
			// Move back to the original thread.
			moveToThread(d->thread_info.orig_thread);
		}

		// TODO: Set an error string.
		emit searchError(QLatin1String("SearchThreadWorker: Thread information was not set."));
		return;
	}

	// Search the memory card.
	searchMemCard(d->thread_info.card, d->thread_info.db, d->thread_info.searchUsedBlocks);

	// Move back to the original thread.
	moveToThread(d->thread_info.orig_thread);
}
