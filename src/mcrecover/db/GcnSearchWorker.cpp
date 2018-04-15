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
#include "libmemcard/GcnCard.hpp"

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
		explicit GcnSearchWorkerPrivate(GcnSearchWorker *q);

	protected:
		GcnSearchWorker *const q_ptr;
		Q_DECLARE_PUBLIC(GcnSearchWorker)
	private:
		Q_DISABLE_COPY(GcnSearchWorkerPrivate)

	public:
		// Last error string.
		QString errorString;

		/**
		 * List of files found in the last successful search.
		 * QLinkedList allows us to prepend items, so we do that
		 * in order to turn "reverse-order" into "correct-order".
		 * TODO: Use malloc()'d SearchData?
		 */
		QLinkedList<GcnSearchData> filesFoundList;

		// Properties.
		GcnCard *card;
		QVector<GcnMcFileDb*> databases;
		char preferredRegion;
		bool searchUsedBlocks;

		// Original thread.
		QThread *origThread;
};

GcnSearchWorkerPrivate::GcnSearchWorkerPrivate(GcnSearchWorker* q)
	: q_ptr(q)
	, card(nullptr)
	, preferredRegion(0)
	, searchUsedBlocks(false)
	, origThread(nullptr)
{ }

/** GcnSearchWorker **/

GcnSearchWorker::GcnSearchWorker(QObject *parent)
	: super(parent)
	, d_ptr(new GcnSearchWorkerPrivate(this))
{ }

GcnSearchWorker::~GcnSearchWorker()
{
	Q_D(GcnSearchWorker);
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
QString GcnSearchWorker::errorString(void) const
{
	Q_D(const GcnSearchWorker);
	return d->errorString;
}

/**
 * Get the list of files found in the last successful search.
 * @return List of files found.
 */
QLinkedList<GcnSearchData> GcnSearchWorker::filesFoundList(void) const
{
	// TODO: Not while thread is running...
	Q_D(const GcnSearchWorker);
	return d->filesFoundList;
}

/** Properties. **/

/**
 * Get the GcnCard.
 * @return GcnCard.
 */
GcnCard *GcnSearchWorker::card(void) const
{
	Q_D(const GcnSearchWorker);
	return d->card;
}

/**
 * Set the GcnCard.
 * @param card GcnCard.
 */
void GcnSearchWorker::setCard(GcnCard *card)
{
	// TODO: Not if searching?
	Q_D(GcnSearchWorker);
	d->card = card;
}

/**
 * Get the vector of GCN file databases.
 * @return GCN file databases.
 */
QVector<GcnMcFileDb*> GcnSearchWorker::databases(void) const
{
	Q_D(const GcnSearchWorker);
	return d->databases;
}

/**
 * Set the vector of GCN file databases.
 * @param databases GCN file databases.
 */
void GcnSearchWorker::setDatabases(const QVector<GcnMcFileDb*> &databases)
{
	// TODO: Not if searching?
	Q_D(GcnSearchWorker);
	d->databases = databases;
}

/**
 * Get the preferred region.
 * @return Preferred region.
 */
char GcnSearchWorker::preferredRegion(void) const
{
	Q_D(const GcnSearchWorker);
	return d->preferredRegion;
}

/**
 * Set the preferred region.
 * @param preferredRegion Preferred region.
 */
void GcnSearchWorker::setPreferredRegion(char preferredRegion)
{
	// TODO: Not if searching?
	Q_D(GcnSearchWorker);
	d->preferredRegion = preferredRegion;
}

/**
 * Search used blocks?
 * @return True if searching used blocks; false if not.
 */
bool GcnSearchWorker::searchUsedBlocks(void) const
{
	Q_D(const GcnSearchWorker);
	return d->searchUsedBlocks;
}

/**
 * Should we search used blocks?
 * @param searchUsedBlocks True to search used blocks; false to not.
 */
void GcnSearchWorker::setSearchUsedBlocks(bool searchUsedBlocks)
{
	// TODO: Not if searching?
	Q_D(GcnSearchWorker);
	d->searchUsedBlocks = searchUsedBlocks;
}

/**
 * Get the "original thread".
 *
 * This is the thread the object attaches to after
 * the search is complete. If nullptr, no attachment
 * will be done, and the data may be lost.
 *
 * @return Original thread.
 */
QThread *GcnSearchWorker::origThread(void) const
{
	Q_D(const GcnSearchWorker);
	return d->origThread;
}

/**
 * Set the "original thread".
 *
 * This is the thread the object attaches to after
 * the search is complete. If nullptr, no attachment
 * will be done, and the data may be lost.
 *
 * @param origThread Original thread.
 */
void GcnSearchWorker::setOrigThread(QThread *origThread)
{
	// TODO: Not if searching?
	Q_D(GcnSearchWorker);
	d->origThread = origThread;
}

/** Search functions. **/

/**
 * Search a memory card for "lost" files.
 * Properties must have been set previously.
 * @return Number of files found on success; negative on error.
 *
 * If successful, retrieve the file list using filesEntryList().
 * If an error occurs, check the errorString(). (TODO)
 */
int GcnSearchWorker::searchMemCard(void)
{
	Q_D(GcnSearchWorker);
	d->filesFoundList.clear();

	if (!d->card) {
		// No card specified.
		d->errorString = tr("searchMemCard(): A card was not set.");
		emit searchError(d->errorString);
		return -1;
	}

	if (d->databases.isEmpty()) {
		// Database is not loaded.
		// TODO: Set an error string somewhere.
		d->errorString = tr("searchMemCard(): No databases were loaded.");
		emit searchError(d->errorString);
		return -1;
	}

	// FIXME: GCN-specific assumptions used here. (first block is 5, etc)
	// Add more information to Card to indicate the usable area.

	// Block search list.
	QVector<uint16_t> blockSearchList;
	const int totalPhysBlocks = d->card->totalPhysBlocks();

	// Used block map.
	QVector<uint8_t> usedBlockMap;
	if (!d->searchUsedBlocks) {
		// Only search empty blocks.
		usedBlockMap = d->card->usedBlockMap();

		// Put together a block search list.
		blockSearchList.reserve(totalPhysBlocks - d->card->freeBlocks() - 5);
		for (int i = (usedBlockMap.size() - 1); i >= 5; i--) {
			if (usedBlockMap[i] == 0) {
				blockSearchList.append((uint16_t)i);
			}
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
		d->errorString = tr("searchMemCard(): No blocks to search.");
		emit searchError(d->errorString);
		return 0;
	}

	// Block buffer.
	const int blockSize = d->card->blockSize();
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

		int ret = d->card->readBlock(buf, blockSize, currentPhysBlock);
		if (ret != blockSize) {
			// Error reading block.
			fprintf(stderr, "ERROR reading block %d - readBlock() returned %d.\n", currentPhysBlock, ret);
			continue;
		}

		// Check the block in the databases.
		QVector<GcnSearchData> searchDataEntries;
		foreach (GcnMcFileDb *db, d->databases) {
			QVector<GcnSearchData> curEntries = db->checkBlock(buf, blockSize);
			searchDataEntries += curEntries;
		}

		// TODO: Search for preferred region. For now, just use the first hit.
		if (!searchDataEntries.isEmpty()) {
			// Matched!
			GcnSearchData searchData;
			if (searchDataEntries.size() == 1 || d->preferredRegion == 0) {
				// Only one entry, or no preferred region.
				searchData = searchDataEntries.at(0);
			} else {
				// Find an entry matching the preferred region.
				bool isMatch = false;
				printf("\n");
				for (int i = 0; i < searchDataEntries.size(); i++) {
					const GcnSearchData &schk = searchDataEntries.at(i);
					if (schk.dirEntry.gamecode[3] == d->preferredRegion) {
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
	free(buf);
	return d->filesFoundList.size();
}

/**
 * Search the memory card for "lost" files.
 * This version should be connected to a QThread's SIGNAL(started()).
 * Thread information must have been set using setThreadInfo().
 */
void GcnSearchWorker::searchMemCard_threaded(void)
{
	Q_D(GcnSearchWorker);

	if (!d->card ||
	    d->databases.isEmpty() ||
	    !d->origThread)
	{
		// Thread information was not set.
		if (d->origThread) {
			// Move back to the original thread.
			moveToThread(d->origThread);
		}

		d->errorString = tr("GcnSearchWorker: Thread information was not set.");
		emit searchError(d->errorString);
		return;
	}

	// Search the memory card.
	searchMemCard();

	// Move back to the original thread.
	moveToThread(d->origThread);
}
