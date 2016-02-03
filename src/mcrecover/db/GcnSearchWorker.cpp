/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * GcnSearchWorker.hpp: GCN "lost" file search worker.                     *
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

#include "GcnSearchWorker.hpp"

// GcnCard
#include "card/GcnCard.hpp"

// GCN Memory Card File Database
#include "db/GcnMcFileDb.hpp"

// Checksum algorithm class.
#include "Checksum.hpp"

// C includes. (C++ namespace)
#include <cstdio>

// C++ includes.
#include <limits>

// Qt includes.
#include <QtCore/QLinkedList>
#include <QtCore/QVector>

/** GcnSearchWorkerPrivate **/

class GcnSearchWorkerPrivate
{
	public:
		GcnSearchWorkerPrivate(GcnSearchWorker *q);

	protected:
		GcnSearchWorker *const q_ptr;
		Q_DECLARE_PUBLIC(GcnSearchWorker)
	private:
		Q_DISABLE_COPY(GcnSearchWorkerPrivate)

	public:
		/**
		 * List of files found in the last successful search.
		 * QLinkedList allows us to prepend items, so we do that
		 * in order to turn "reverse-order" into "correct-order".
		 * TODO: Use malloc()'d SearchData?
		 */
		QLinkedList<GcnSearchData> filesFoundList;

		// searchMemCard() parameters used when this worker
		// is called by a thread's started() signal.
		struct {
			GcnCard *card;
			QVector<GcnMcFileDb*> dbs;

			// Original thread.
			QThread *orig_thread;

			// Preferred region.
			char preferredRegion;

			// If true, search all blocks, not just empty blocks.
			bool searchUsedBlocks;
		} thread_info;
};

GcnSearchWorkerPrivate::GcnSearchWorkerPrivate(GcnSearchWorker* q)
	: q_ptr(q)
{
	// NULL these out by default.
	thread_info.card = nullptr;
	thread_info.dbs.clear();
	thread_info.orig_thread = nullptr;
	thread_info.searchUsedBlocks = false;
}

/** GcnSearchWorker **/

GcnSearchWorker::GcnSearchWorker(QObject *parent)
	: QObject(parent)
	, d_ptr(new GcnSearchWorkerPrivate(this))
{ }

GcnSearchWorker::~GcnSearchWorker()
{
	Q_D(GcnSearchWorker);
	delete d;
}

/**
 * Get the list of files found in the last successful search.
 * @return List of files found.
 */
QLinkedList<GcnSearchData> GcnSearchWorker::filesFoundList(void)
{
	// TODO: Not while thread is running...
	Q_D(GcnSearchWorker);
	return d->filesFoundList;
}

/**
 * Search a memory card for "lost" files.
 * @param card Memory Card to search.
 * @param dbs Vector of GcnMcFileDb to use.
 * @param preferredRegion Preferred region.
 * @param searchUsedBlocks If true, search all blocks, not just blocks marked as empty.
 * @return Number of files found on success; negative on error.
 *
 * If successful, retrieve the file list using dirEntryList().
 * If an error occurs, check the errorString(). (TODO)
 */
int GcnSearchWorker::searchMemCard(GcnCard *card, const QVector<GcnMcFileDb*> &dbs,
				      char preferredRegion, bool searchUsedBlocks)
{
	Q_D(GcnSearchWorker);
	d->filesFoundList.clear();

	if (dbs.isEmpty()) {
		// Database is not loaded.
		// TODO: Set an error string somewhere.
		emit searchCancelled();
		return -1;
	}

	// FIXME: GCN-specific assumptions used here. (first block is 5, etc)
	// Add more information to Card to indicate the usable area.

	// Block search list.
	QVector<uint16_t> blockSearchList;
	const int totalPhysBlocks = card->totalPhysBlocks();

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
		// TODO: Mark system blocks as used?
		usedBlockMap = QVector<uint8_t>(totalPhysBlocks, 0);

		// Put together a block search list.
		blockSearchList.reserve(totalPhysBlocks - 5);
		for (int i = (usedBlockMap.size() - 1); i >= 5; i--) {
			blockSearchList.append((uint16_t)i);
		}
	}

	if (blockSearchList.isEmpty()) {
		// No blocks to search.
		// This may happen if searchUsedBlocks == false
		// and the card is full.
		// TODO: Set an error string somewhere.
		emit searchCancelled();
		return 0;
	}

	// Block buffer.
	const int blockSize = card->blockSize();
	void *buf = malloc(blockSize);

	fprintf(stderr, "--------------------------------\n");
	fprintf(stderr, "SCANNING MEMORY CARD...\n");

	const int totalSearchBlocks = blockSearchList.size();
	int currentPhysBlock = blockSearchList.value(0);
	emit searchStarted(totalPhysBlocks, totalSearchBlocks, currentPhysBlock);

	int currentSearchBlock = -1;	// compensate for currentSearchBlock++
	foreach (currentPhysBlock, blockSearchList) {
		currentSearchBlock++;
		fprintf(stderr, "Searching block: %d...\n", currentPhysBlock);
		emit searchUpdate(currentPhysBlock, currentSearchBlock, d->filesFoundList.size());

		int ret = card->readBlock(buf, blockSize, currentPhysBlock);
		if (ret != blockSize) {
			// Error reading block.
			fprintf(stderr, "ERROR reading block %d - readBlock() returned %d.\n", currentPhysBlock, ret);
			continue;
		}

		// Check the block in the databases.
		QVector<GcnSearchData> searchDataEntries;
		foreach (GcnMcFileDb *db, dbs) {
			QVector<GcnSearchData> curEntries = db->checkBlock(buf, blockSize);
			searchDataEntries += curEntries;
		}

		// TODO: Search for preferred region. For now, just use the first hit.
		if (!searchDataEntries.isEmpty()) {
			// Matched!
			GcnSearchData searchData;
			if (searchDataEntries.size() == 1 || preferredRegion == 0) {
				// Only one entry, or no preferred region.
				searchData = searchDataEntries.at(0);
			} else {
				// Find an entry matching the preferred region.
				bool isMatch = false;
				printf("\n");
				for (int i = 0; i < searchDataEntries.size(); i++) {
					const GcnSearchData &schk = searchDataEntries.at(i);
					if (schk.dirEntry.gamecode[3] == preferredRegion) {
						// Found a match!
						searchData = schk;
						isMatch = true;
						break;
					}
				}

				if (!isMatch) {
					// No region match. Use the first entry.
					searchData = searchDataEntries.at(0);
				}
			}

			// NOTE: GcnMcFileDb doesn't initialize fatEntries.
			// Hence, we have to make a copy and initialize the list.
			fprintf(stderr, "FOUND A MATCH: %-.4s%-.2s %-.32s\n",
				searchData.dirEntry.gamecode,
				searchData.dirEntry.company,
				searchData.dirEntry.filename);
			fprintf(stderr, "bannerFmt == %02X, iconAddress == %08X, iconFormat == %02X, iconSpeed == %02X\n",
				searchData.dirEntry.bannerfmt,
				searchData.dirEntry.iconaddr,
				searchData.dirEntry.iconfmt,
				searchData.dirEntry.iconspeed);

			// NOTE: dirEntry's block start is not set by d->db->checkBlock().
			// Set it here.
			searchData.dirEntry.block = currentPhysBlock;
			if (searchData.dirEntry.length == 0) {
				// This only happens if an entry is either
				// missing a <dirEntry>, or has <length>0</length>.
				// TODO: Check for this in GcnMcFileDb.
				searchData.dirEntry.length = 1;
			}

			// Construct the FAT entries for this file.
			searchData.fatEntries.clear();
			searchData.fatEntries.reserve(searchData.dirEntry.length);

			// First block is always valid.
			searchData.fatEntries.append(searchData.dirEntry.block);
			if (usedBlockMap[searchData.dirEntry.block] < std::numeric_limits<uint8_t>::max())
				usedBlockMap[searchData.dirEntry.block]++;

			uint16_t blocksRemaining = (searchData.dirEntry.length - 1);
			uint16_t block = (searchData.dirEntry.block + 1);
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
				} else if (block == searchData.dirEntry.block) {
					// ERROR: We wrapped around!
					// Use the "naive" algorithm after the last valid block.
					break;
				}

				// Check if this block is used.
				if (usedBlockMap[block] == 0) {
					// Block is not used.
					searchData.fatEntries.append(block);
					if (!wasWrapped)
						usedBlockMap[block]++;
					blocksRemaining--;
				}

				// Next block.
				block++;
			}

			// Naive block algorithm for the remaining blocks.
			block = (searchData.fatEntries.value(searchData.fatEntries.size() - 1) + 1);
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
				searchData.fatEntries.append(block);
				if (usedBlockMap[block] < std::numeric_limits<uint8_t>::max()) {
					if (!wasWrapped)
						usedBlockMap[block]++;
				}
				block++;
				blocksRemaining--;
			}

			// Add the search data to the list.
			d->filesFoundList.prepend(searchData);
		}
	}

	// Send an update for the last block.
	emit searchUpdate(5, currentSearchBlock, d->filesFoundList.size());

	// Search is finished.
	emit searchFinished(d->filesFoundList.size());

	fprintf(stderr, "Finished scanning memory card.\n");
	fprintf(stderr, "--------------------------------\n");
	return d->filesFoundList.size();
}

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
void GcnSearchWorker::setThreadInfo(GcnCard *card, const QVector<GcnMcFileDb*> &dbs,
				       QThread *orig_thread,
				       char preferredRegion, bool searchUsedBlocks)
{
	Q_D(GcnSearchWorker);
	d->thread_info.card = card;
	d->thread_info.dbs = dbs; // TODO: Convert to QVector<const GcnMcFileDb*>?
	d->thread_info.orig_thread = orig_thread;
	d->thread_info.preferredRegion = preferredRegion;
	d->thread_info.searchUsedBlocks = searchUsedBlocks;
}

/**
 * Search the memory card for "lost" files.
 * This version should be connected to a QThread's SIGNAL(started()).
 * Thread information must have been set using setThreadInfo().
 */
void GcnSearchWorker::searchMemCard_threaded(void)
{
	Q_D(GcnSearchWorker);

	if (!d->thread_info.card ||
	    d->thread_info.dbs.isEmpty() ||
	    !d->thread_info.orig_thread)
	{
		// Thread information was not set.
		if (d->thread_info.orig_thread) {
			// Move back to the original thread.
			moveToThread(d->thread_info.orig_thread);
		}

		// TODO: Set an error string.
		emit searchError(QLatin1String("GcnSearchWorker: Thread information was not set."));
		return;
	}

	// Search the memory card.
	searchMemCard(d->thread_info.card, d->thread_info.dbs,
		      d->thread_info.preferredRegion, d->thread_info.searchUsedBlocks);

	// Move back to the original thread.
	moveToThread(d->thread_info.orig_thread);
}
