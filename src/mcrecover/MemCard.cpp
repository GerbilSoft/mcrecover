/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * MemCard.cpp: Memory Card reader class.                                  *
 *                                                                         *
 * Copyright (c) 2011 by David Korth.                                      *
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

#include "MemCard.hpp"
#include "card.h"
#include "byteswap.h"

// MemCardFile
#include "MemCardFile.hpp"

// Checksum algorithms.
#include "Checksum.hpp"

// C includes. (C++ namespace)
#include <cstring>
#include <cstdio>

// C++ includes.
#include <limits>

// Qt includes.
#include <QtCore/QFile>
#include <QtCore/QList>
#include <QtCore/QTextCodec>

#define NUM_ELEMENTS(x) ((int)(sizeof(x) / sizeof(x[0])))

/** MemCardPrivate **/

class MemCardPrivate
{
	public:
		MemCardPrivate(MemCard *q, QString filename);
		~MemCardPrivate();
		void init(void);

	private:
		MemCard *const q;
		Q_DISABLE_COPY(MemCardPrivate);

	public:
		QString filename;
		QFile *file;

		// Filesize.
		int filesize;

		// Block size.
		// NOTE: This is always assumed to be 8 KB.
		static const int blockSize = 8192;

		/**
		 * Total number of blocks in the file.
		 * Includes the 5 reserved blocks at the beginning.
		 */
		int numBlocks;

		// QTextCodec for memory card text encoding.
		QTextCodec *textCodec;

		// Memory card data.
		// Table 0 == main; Table 1 == backup.
		card_header mc_header;		// Header.
		card_dat mc_dat_int[2];		// Directory tables.
		card_bat mc_bat_int[2];		// Block allocation tables.

		// Memory card data being used.
		card_dat *mc_dat;
		card_bat *mc_bat;

		// MemCardFile list.
		QList<MemCardFile*> lstMemCardFile;

		/**
		 * Used block map.
		 * NOTE: This is only valid for regular files, not "lost" files.
		 * Value indicates how many files are "using" that block.
		 * Should be 0 for free, 1 for normal files,
		 * and >1 for "lost" files that are overlapping other files.
		 */
		QVector<uint8_t> usedBlockMap;

	private:
		/**
		 * Load the memory card system information.
		 * @return 0 on success; non-zero on error.
		 */
		int loadSysInfo(void);

		/**
		 * Load a directory table.
		 * @param dat		[out] card_dat to store the directory table in.
		 * @param address	[in] Directory table address.
		 * @param checksum	[out] Calculated checksum. (AddSubDual16)
		 */
		void loadDirTable(card_dat *dat, uint32_t address, uint32_t *checksum);

		/**
		 * Load a block allocation table.
		 * @param bat		[out] card_bat to store the block allocation table in.
		 * @param address	[in] Directory table address.
		 * @param checksum	[out] Calculated checksum. (AddSubDual16)
		 */
		void loadBlockTable(card_bat *bat, uint32_t address, uint32_t *checksum);

		/**
		 * Load the MemCardFile list.
		 */
		void loadMemCardFileList(void);
};

MemCardPrivate::MemCardPrivate(MemCard *q, QString filename)
	: q(q)
	, file(NULL)
	, textCodec(NULL)
	, mc_dat(NULL)
	, mc_bat(NULL)
{
	// TODO: Set an error code somewhere.

	// Save the filename.
	this->filename = filename;
	if (filename.isEmpty()) {
		// No filename specified.
		file = NULL;
		return;
	}

	// Open the file.
	file = new QFile(filename, q);
	if (!file->open(QIODevice::ReadOnly)) {
		// Error opening the file.
		delete file;
		file = NULL;
		return;
	}

	// Get the filesize.
	filesize = (int)file->size();
	// TODO: Verify the filesize:
	// - Should be a power of two.
	// - Should at least be 40 KB for system information.

	// Calculate the number of blocks.
	numBlocks = (filesize / blockSize);

	// Initialize the used block map.
	// (The first 5 blocks are always used.)
	usedBlockMap = QVector<uint8_t>(numBlocks, 0);
	for (int i = 0; i < 5 && i < numBlocks; i++)
		usedBlockMap[i] = 1;

	// NOTE: Initialization must be done *after* MemCard is initialized!
}

MemCardPrivate::~MemCardPrivate()
{
	// Clear the MemCardFile list.
	qDeleteAll(lstMemCardFile);
	lstMemCardFile.clear();

	if (file) {
		file->close();
		delete file;
	}
}


/**
 * Initialize MemCardPrivate.
 * This must be run *after* MemCard is initialized!
 */
void MemCardPrivate::init(void)
{
	if (!file)
		return;

	// Load the memory card system information.
	// This includes the header, directory, and block allocation table.
	loadSysInfo();

	// Load the MemCardFile list.
	loadMemCardFileList();
}


/**
 * Load the memory card system information.
 * @return 0 on success; non-zero on error.
 */
int MemCardPrivate::loadSysInfo(void)
{
	if (!file)
		return -1;

	// TODO: Verify read sizes.

	// Header.
	file->seek(0);
	file->read((char*)&mc_header, sizeof(mc_header));

	// Byteswap the header contents.
	mc_header.formatTime	= be64_to_cpu(mc_header.formatTime);
	mc_header.sramBias	= be32_to_cpu(mc_header.sramBias);
	mc_header.sramLang	= be32_to_cpu(mc_header.sramLang);
	mc_header.device_id	= be16_to_cpu(mc_header.device_id);
	mc_header.size		= be16_to_cpu(mc_header.size);
	mc_header.encoding	= be16_to_cpu(mc_header.encoding);
	mc_header.chksum1	= be16_to_cpu(mc_header.chksum1);
	mc_header.chksum2	= be16_to_cpu(mc_header.chksum2);

	/**
	 * NOTE: formatTime appears to be in units of (CPU clock / 12).
	 * This means the time format will be different depending on if
	 * the card was formatted on a GameCube (or Wii in GCN mode)
	 * or on a Wii in Wii mode.
	 *
	 * Don't bother determining the actual formatTime right now.
	 */

	// Get the QTextCodec for the memory card's text encoding.
	if ((mc_header.encoding & SYS_FONT_ENCODING_MASK) == SYS_FONT_ENCODING_SJIS) {
		// Shift-JIS encoding.
		textCodec = QTextCodec::codecForName("Shift-JIS");
	}
	if (!textCodec) {
		// "ANSI" encoding is used.
		// Alternatively, Shift-JIS is used, but we couldn't find
		// a QTextCodec for Shift-JIS. This shouldn't happen, but
		// it's possible...
		textCodec = QTextCodec::codecForName("latin1");
	}

	// TODO: Adjust for block size?
	// TODO: Store the actual and expected DAT/BAT checksums?

	// Directory tables.
	uint32_t checksum_actual[2];
	loadDirTable(&mc_dat_int[0], CARD_SYSDIR, &checksum_actual[0]);
	loadDirTable(&mc_dat_int[1], CARD_SYSDIR_BACK, &checksum_actual[1]);

	// Get the expected checksums.
	uint32_t checksum_expected[2];
	checksum_expected[0] = (mc_dat_int[0].dircntrl.chksum1 << 16) |
			       (mc_dat_int[0].dircntrl.chksum2);
	checksum_expected[1] = (mc_dat_int[1].dircntrl.chksum1 << 16) |
			       (mc_dat_int[1].dircntrl.chksum2);

	// Determine which directory table to use.
	// - 1. Check for higher "updated" value.
	// - 2. Validate checksums.
	// - 3. If invalid checksum, use other one.
	// - 4. If both are invalid, error!
	// TODO: If both checksums are invalid, report an error. Using main for now.
	// TODO: Allow user to select?
	int dirTable = (mc_dat_int[1].dircntrl.updated > mc_dat_int[0].dircntrl.updated ? 1 : 0);

	// Verify the checksums of the selected directory table.
	if (checksum_expected[0] != checksum_actual[0]) {
		// Invalid checksum. Check the other directory table.
		dirTable = !dirTable;
		if (checksum_expected[1] != checksum_actual[1]) {
			// Both directory tables are invalid.
			// TODO: Report an error.
			// For now, default to main.
			fprintf(stderr, "WARNING: Both DATs are invalid. Using MAIN.\n");
			dirTable = 0;
		}
	}

	// Select the directory table.
	mc_dat = &mc_dat_int[dirTable];
	printf("Dir Table == %d\n", dirTable);

	// Block allocation tables.
	loadBlockTable(&mc_bat_int[0], CARD_SYSBAT, &checksum_actual[0]);
	loadBlockTable(&mc_bat_int[1], CARD_SYSBAT_BACK, &checksum_actual[1]);

	// Get the expected checksums.
	checksum_expected[0] = (mc_bat_int[0].chksum1 << 16) |
			       (mc_bat_int[0].chksum2);
	checksum_expected[1] = (mc_bat_int[1].chksum1 << 16) |
			       (mc_bat_int[1].chksum2);

	// Determine which block allocation table to use.
	// - 1. Check for higher "updated" value.
	// - 2. Validate checksums.
	// - 3. If invalid checksum, use other one.
	// - 4. If both are invalid, error!
	// TODO: If both checksums are invalid, report an error. Using main for now.
	// TODO: Allow user to select?
	int blockTable = (mc_bat_int[1].updated > mc_bat_int[0].updated ? 1 : 0);

	// Verify the checksums of the selected block allocation table.
	if (checksum_expected[0] != checksum_actual[0]) {
		// Invalid checksum. Check the other block allocation table.
		blockTable = !blockTable;
		if (checksum_expected[1] != checksum_actual[1]) {
			// Both block allocation tables are invalid.
			// TODO: Report an error.
			// For now, default to main.
			fprintf(stderr,"WARNING: Both BATs are invalid. Using MAIN.\n");
			blockTable = 0;
		}
	}

	// Select the directory table.
	mc_bat = &mc_bat_int[blockTable];
	printf("Block Table == %d\n", blockTable);
	return 0;
}


/**
 * Load a directory table.
 * @param dat		[out] card_dat to store the directory table in.
 * @param address	[in] Directory table address.
 * @param checksum	[out] Calculated checksum. (AddSubDual16)
 */
void MemCardPrivate::loadDirTable(card_dat *dat, uint32_t address, uint32_t *checksum)
{
	// TODO: Verify read size.
	file->seek(address);
	file->read((char*)dat, sizeof(*dat));

	// Calculate the checksums.
	if (checksum != NULL) {
		*checksum = Checksum::AddInvDual16(
			reinterpret_cast<const uint16_t*>(dat),
			(uint32_t)(sizeof(*dat) - 4));
	}

	// Byteswap the directory table contents.
	for (int i = 0; i < NUM_ELEMENTS(dat->entries); i++) {
		card_direntry *dirEntry	= &dat->entries[i];
		dirEntry->lastmodified	= be32_to_cpu(dirEntry->lastmodified);
		dirEntry->iconaddr	= be32_to_cpu(dirEntry->iconaddr);
		dirEntry->iconfmt	= be16_to_cpu(dirEntry->iconfmt);
		dirEntry->iconspeed	= be16_to_cpu(dirEntry->iconspeed);
		dirEntry->block		= be16_to_cpu(dirEntry->block);
		dirEntry->length	= be16_to_cpu(dirEntry->length);
		dirEntry->commentaddr	= be32_to_cpu(dirEntry->commentaddr);
	}

	// Byteswap the directory control block.
	dat->dircntrl.updated = be16_to_cpu(dat->dircntrl.updated);
	dat->dircntrl.chksum1 = be16_to_cpu(dat->dircntrl.chksum1);
	dat->dircntrl.chksum2 = be16_to_cpu(dat->dircntrl.chksum2);
}


/**
 * Load a block allocation table.
 * @param bat		[out] card_bat to store the block allocation table in.
 * @param address	[in] Directory table address.
 * @param checksum	[out] Calculated checksum. (AddSubDual16)
 */
void MemCardPrivate::loadBlockTable(card_bat *bat, uint32_t address, uint32_t *checksum)
{
	// TODO: Verify read size.
	file->seek(address);
	file->read((char*)bat, sizeof(*bat));

	// Calculate the checksums.
	if (checksum != NULL) {
		*checksum = Checksum::AddInvDual16(
			(reinterpret_cast<const uint16_t*>(bat) + 2),
			(uint32_t)(sizeof(*bat) - 4));
	}

	// Byteswap the block allocation table contents.
	bat->chksum1	= be16_to_cpu(bat->chksum1);
	bat->chksum2	= be16_to_cpu(bat->chksum2);
	bat->updated	= be16_to_cpu(bat->updated);
	bat->freeblocks	= be16_to_cpu(bat->freeblocks);
	bat->lastalloc	= be16_to_cpu(bat->lastalloc);

	for (int i = 0; i < NUM_ELEMENTS(bat->fat); i++)
		bat->fat[i] = be16_to_cpu(bat->fat[i]);
}


/**
 * Load the MemCardFile list.
 */
void MemCardPrivate::loadMemCardFileList(void)
{
	if (!file)
		return;

	// Clear the current MemCardFile list.
	qDeleteAll(lstMemCardFile);
	lstMemCardFile.clear();
	lstMemCardFile.reserve(NUM_ELEMENTS(mc_dat->entries));

	// Byteswap the directory table contents.
	for (int i = 0; i < NUM_ELEMENTS(mc_dat->entries); i++) {
		const card_direntry *dirEntry = &mc_dat->entries[i];

		// If the game code is 0xFFFFFFFF, the entry is empty.
		static const uint8_t gamecode_empty[4] = {0xFF, 0xFF, 0xFF, 0xFF};
		if (!memcmp(dirEntry->gamecode, gamecode_empty, sizeof(gamecode_empty)))
			continue;

		// Valid directory entry.
		MemCardFile *mcFile = new MemCardFile(q, i, mc_dat, mc_bat);
		lstMemCardFile.append(mcFile);

		// Mark the file's blocks as used.
		QVector<uint16_t> fatEntries = mcFile->fatEntries();
		foreach (uint16_t block, fatEntries) {
			if (block >= 5 && block < usedBlockMap.size()) {
				// Valid block.
				// Increment its entry in the usedBlockMap.
				if (usedBlockMap[block] < std::numeric_limits<uint8_t>::max())
					usedBlockMap[block]++;
			} else {
				// Invalid block.
				// TODO: Store an error value somewhere.
				fprintf(stderr, "WARNING: File %d has invalid FAT entry 0x%04X.\n", i, block);
			}
		}
	}

	// File list has changed.
	emit q->changed();
}


/** MemCard **/

MemCard::MemCard(const QString& filename, QObject *parent)
	: QObject(parent)
	, d(new MemCardPrivate(this, filename))
{
	// Initialize MemCardPrivate.
	d->init();
}

MemCard::~MemCard()
	{ delete d; }


/**
 * Check if the memory card is open.
 * @return True if open; false if not.
 */
bool MemCard::isOpen(void) const
	{ return !!(d->file); }

/**
 * Get the memory card filename.
 * @return Memory card filename, or empty string if not open.
 */
QString MemCard::filename(void) const
	{ return d->filename; }

/**
 * Get the size of the memory card, in blocks.
 * @return Size of memory card, in blocks. (Negative on error)
 */
int MemCard::sizeInBlocks(void) const
{
	if (!isOpen())
		return -1;
	return d->numBlocks;
}

/**
 * Get the number of free blocks.
 * @return Free blocks. (Negative on error)
 */
int MemCard::freeBlocks(void) const
{
	if (!isOpen())
		return -1;
	return d->mc_bat->freeblocks;
}

/**
 * Get the memory card block size, in bytes.
 * @return Memory card block size, in bytes. (Negative on error)
 */
int MemCard::blockSize(void) const
{
	if (!isOpen())
		return -1;
	return d->blockSize;
}


/**
 * Read a block.
 * @param buf Buffer to read the block data into.
 * @param siz Size of buffer.
 * @param blockIdx Block index.
 * @return Bytes read on success; negative on error.
 */
int MemCard::readBlock(void *buf, int siz, uint16_t blockIdx)
{
	if (!isOpen())
		return -1;
	if (siz < blockSize())
		return -2;
	
	// Read the specified block.
	d->file->seek((int)blockIdx * blockSize());
	return (int)d->file->read((char*)buf, blockSize());
}


/**
 * Get the memory card's serial number.
 * @return Memory card's serial number.
 */
QString MemCard::serialNumber(void) const
{
	// TODO: We're returning the 12-byte serial number.
	// Should we return the 8-byte F-Zero GX / PSO serial number instead?

	QString serial_text;
	char tmp[4];

	// TODO: ARRAY_SIZE() macro.
	serial_text.reserve(sizeof(d->mc_header.serial)*2);

	for (int i = 0; i < (int)sizeof(d->mc_header.serial); i++) {
		snprintf(tmp, sizeof(tmp), "%02X", d->mc_header.serial[i]);
		serial_text += QLatin1String(tmp);
	}

	return serial_text;
}


/**
 * Get the memory card encoding.
 * @return 0 for ANSI (ISO-8859-1); 1 for SJIS; negative on error.
 */
int MemCard::encoding(void) const
{
	if (!isOpen())
		return -1;
	return (d->mc_header.encoding & SYS_FONT_ENCODING_MASK);
}


/**
 * Get the QTextCodec for the memory card encoding.
 * @return QTextCodec.
 */
QTextCodec *MemCard::textCodec(void) const
	{ return d->textCodec; }


/**
 * Get the number of files in the file table.
 * @return Number of files, or negative on error.
 */
int MemCard::numFiles(void) const
{
	if (!isOpen())
		return -1;
	return d->lstMemCardFile.size();
}

/**
 * Is the card empty?
 * @return True if empty; false if not.
 */
bool MemCard::isEmpty(void) const
{
	if (!isOpen())
		return true;
	return d->lstMemCardFile.isEmpty();
}

/**
 * Get a MemCardFile object.
 * @param idx File number.
 * @return MemCardFile object, or NULL on error.
 */
MemCardFile *MemCard::getFile(int idx)
{
	if (!isOpen())
		return NULL;
	if (idx < 0 || idx >= d->lstMemCardFile.size())
		return NULL;
	return d->lstMemCardFile.at(idx);
}


/**
 * Get the used block map.
 * NOTE: This is only valid for regular files, not "lost" files.
 * @return Used block map.
 */
QVector<uint8_t> MemCard::usedBlockMap(void)
{
	if (!isOpen())
		return QVector<uint8_t>();
	return d->usedBlockMap;
}


/**
 * Remove all "lost" files.
 */
void MemCard::removeLostFiles(void)
{
	for (int i = d->lstMemCardFile.size() - 1; i >= 0; i--) {
		MemCardFile *mcFile = d->lstMemCardFile.at(i);
		if (mcFile->isLostFile()) {
			// This is a "lost" file. Remove it.
			d->lstMemCardFile.removeAt(i);
			emit fileRemoved(i);
		}
	}
}


/**
 * Add a "lost" file.
 * NOTE: This is a debugging version.
 * Add more comprehensive versions with a block map specification.
 * @return MemCardFile added to the MemCard, or NULL on error.
 */
MemCardFile *MemCard::addLostFile(const card_direntry *dirEntry)
{
	if (!isOpen())
		return NULL;

	// Initialize the FAT entries baesd on start/length.
	// TODO: Check for block collisions and skip used blocks.
	QVector<uint16_t> fatEntries;
	fatEntries.reserve(dirEntry->length);

	const uint16_t maxBlockNum = ((uint16_t)sizeInBlocks() - 1);
	if (maxBlockNum <= 5 || maxBlockNum > 4091) {
		// Invalid maximum block size. Don't initialize the FAT.
		// TODO: Print an error message.
	} else {
		// Initialize the FAT.
		uint16_t block = dirEntry->block;
		uint16_t length = dirEntry->length;
		for (; length > 0; length--, block++) {
			if (block > maxBlockNum)
				block = 5;
			fatEntries.append(block);
		}
	}

	return addLostFile(dirEntry, fatEntries);
}


/**
 * Add a "lost" file.
 * @param dirEntry Directory entry.
 * @param fatEntries FAT entries.
 * @return MemCardFile added to the MemCard, or NULL on error.
 */
MemCardFile *MemCard::addLostFile(const card_direntry *dirEntry, QVector<uint16_t> fatEntries)
{
	if (!isOpen())
		return NULL;

	MemCardFile *file = new MemCardFile(this, dirEntry, fatEntries);
	d->lstMemCardFile.append(file);
	emit fileAdded(d->lstMemCardFile.size() - 1);
	return file;
}
