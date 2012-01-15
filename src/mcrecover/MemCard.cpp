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

// C includes.
#include <string.h>

// Qt includes.
#include <QtCore/QFile>
#include <QtCore/QList>

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
	
	private:
		/**
		 * Load the memory card system information.
		 * @return 0 on success; non-zero on error.
		 */
		int loadSysInfo(void);
		
		/**
		 * Load a directory table.
		 * @param dat     [out] card_dat to store the directory table in.
		 * @param address  [in] Directory table address.
		 * @param chksum1 [out] Calculated checksum 1.
		 * @param chksum2 [out] Calculated checksum 2.
		 */
		void loadDirTable(card_dat *dat, uint32_t address, uint16_t *chksum1, uint16_t *chksum2);
		
		/**
		 * Load a block allocation table.
		 * @param bat     [out] card_bat to store the block allocation table in.
		 * @param address  [in] Directory table address.
		 * @param chksum1 [out] Calculated checksum 1.
		 * @param chksum2 [out] Calculated checksum 2.
		 */
		void loadBlockTable(card_bat *bat, uint32_t address, uint16_t *chksum1, uint16_t *chksum2);
		
		/**
		 * Load the MemCardFile list.
		 */
		void loadMemCardFileList(void);
		
		/**
		 * Calculate the card checksums.
		 * @param buf Input data.
		 * @param siz Length of input data.
		 * @param chksum1 Checksum 1.
		 * @param chksum2 Checksum 2.
		 */
		void calcChecksums(uint16_t *buf, size_t siz, uint16_t *chksum1, uint16_t *chksum2);
};

MemCardPrivate::MemCardPrivate(MemCard *q, QString filename)
	: q(q)
{
	// Save the filename.
	this->filename = filename;
	if (filename.isEmpty())
	{
		// No filename specified.
		file = NULL;
		return;
	}
	
	// Open the file.
	file = new QFile(filename, q);
	if (!file->open(QIODevice::ReadOnly))
	{
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
	
	// NOTE: Initialization must be done *after* MemCard is initialized!
}

MemCardPrivate::~MemCardPrivate()
{
	// Clear the MemCardFile list.
	qDeleteAll(lstMemCardFile);
	lstMemCardFile.clear();
	
	if (file)
	{
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
	for (int i = 0; i < NUM_ELEMENTS(mc_header.serial); i++)
		mc_header.serial[i] = be32_to_cpu(mc_header.serial[i]);
	
	mc_header.device_id	= be16_to_cpu(mc_header.device_id);
	mc_header.size		= be16_to_cpu(mc_header.size);
	mc_header.encoding	= be16_to_cpu(mc_header.encoding);
	mc_header.chksum1	= be16_to_cpu(mc_header.chksum1);
	mc_header.chksum2	= be16_to_cpu(mc_header.chksum2);
	
	// TODO: Adjust for block size?
	
	// Directory tables.
	uint16_t chksum[2][2];
	loadDirTable(&mc_dat_int[0], CARD_SYSDIR, &chksum[0][0], &chksum[0][1]);
	loadDirTable(&mc_dat_int[1],  CARD_SYSDIR_BACK, &chksum[1][0], &chksum[1][1]);
	
	// Determine which directory table to use.
	// - 1. Check for higher "updated" value.
	// - 2. Validate checksums.
	// - 3. If invalid checksum, use other one.
	// - 4. If both are invalid, error!
	// TODO: If both checksums are invalid, report an error. Using main for now.
	// TODO: Allow user to select?
	int dirTable = (mc_dat_int[1].dircntrl.updated > mc_dat_int[0].dircntrl.updated ? 1 : 0);
	
	// Verify the checksums of the selected directory table.
	if (mc_dat_int[dirTable].dircntrl.chksum1 != chksum[dirTable][0] ||
	    mc_dat_int[dirTable].dircntrl.chksum2 != chksum[dirTable][1])
	{
		// Invalid checksum. Check the other directory table.
		dirTable = !dirTable;
		if (mc_dat_int[dirTable].dircntrl.chksum1 != chksum[dirTable][0] ||
		    mc_dat_int[dirTable].dircntrl.chksum2 != chksum[dirTable][1])
		{
			// Both directory tables are invalid.
			// TODO: Report an error.
			// For now, default to main.
			printf("WARNING: Both DATs are invalid. Using MAIN.");
			dirTable = 0;
		}
	}
	
	// Select the directory table.
	mc_dat = &mc_dat_int[dirTable];
	printf("Dir Table == %d\n", dirTable);
	
	// Block allocation tables.
	loadBlockTable(&mc_bat_int[0], CARD_SYSBAT, &chksum[0][0], &chksum[0][1]);
	loadBlockTable(&mc_bat_int[1], CARD_SYSBAT_BACK, &chksum[1][0], &chksum[1][1]);
	
	// Determine which block allocation table to use.
	// - 1. Check for higher "updated" value.
	// - 2. Validate checksums.
	// - 3. If invalid checksum, use other one.
	// - 4. If both are invalid, error!
	// TODO: If both checksums are invalid, report an error. Using main for now.
	// TODO: Allow user to select?
	int blockTable = (mc_bat_int[1].updated > mc_bat_int[0].updated ? 1 : 0);
	
	// Verify the checksums of the selected block allocation table.
	if (mc_bat_int[blockTable].chksum1 != chksum[blockTable][0] ||
	    mc_bat_int[blockTable].chksum2 != chksum[blockTable][1])
	{
		// Invalid checksum. Check the other block allocation table.
		blockTable = !blockTable;
		if (mc_bat_int[blockTable].chksum1 != chksum[blockTable][0] ||
		    mc_bat_int[blockTable].chksum2 != chksum[blockTable][1])
		{
			// Both block allocation tables are invalid.
			// TODO: Report an error.
			// For now, default to main.
			printf("WARNING: Both BATs are invalid. Using MAIN.");
			blockTable = 0;
		}
	}
	
	// Select the directory table.
	mc_bat = &mc_bat_int[dirTable];
	printf("Block Table == %d\n", blockTable);
}


/**
 * Load a directory table.
 * @param dat     [out] card_dat to store the directory table in.
 * @param address  [in] Directory table address.
 * @param chksum1 [out] Calculated checksum 1.
 * @param chksum2 [out] Calculated checksum 2.
 */
void MemCardPrivate::loadDirTable(card_dat *dat, uint32_t address, uint16_t *chksum1, uint16_t *chksum2)
{
	// TODO: Verify read size.
	file->seek(address);
	file->read((char*)dat, sizeof(*dat));
	
	// Calculate the checksums.
	if (chksum1 != NULL && chksum2 != NULL)
		calcChecksums((uint16_t*)dat, ((int)sizeof(*dat) - 4), chksum1, chksum2);
	
	// Byteswap the directory table contents.
	for (int i = 0; i < NUM_ELEMENTS(dat->entries); i++)
	{
		card_direntry *direntry	= &dat->entries[i];
		direntry->lastmodified	= be32_to_cpu(direntry->lastmodified);
		direntry->iconaddr	= be32_to_cpu(direntry->iconaddr);
		direntry->iconfmt	= be16_to_cpu(direntry->iconfmt);
		direntry->iconspeed	= be16_to_cpu(direntry->iconspeed);
		direntry->block		= be16_to_cpu(direntry->block);
		direntry->length	= be16_to_cpu(direntry->length);
		direntry->commentaddr	= be32_to_cpu(direntry->commentaddr);
	}
	
	// Byteswap the directory control block.
	dat->dircntrl.updated = be16_to_cpu(dat->dircntrl.updated);
	dat->dircntrl.chksum1 = be16_to_cpu(dat->dircntrl.chksum1);
	dat->dircntrl.chksum2 = be16_to_cpu(dat->dircntrl.chksum2);
}


/**
 * Load a block allocation table.
 * @param bat     [out] card_bat to store the block allocation table in.
 * @param address  [in] Directory table address.
 * @param chksum1 [out] Calculated checksum 1.
 * @param chksum2 [out] Calculated checksum 2.
 */
void MemCardPrivate::loadBlockTable(card_bat *bat, uint32_t address, uint16_t *chksum1, uint16_t *chksum2)
{
	// TODO: Verify read size.
	file->seek(address);
	file->read((char*)bat, sizeof(*bat));
	
	// Calculate the checksums.
	if (chksum1 != NULL && chksum2 != NULL)
		calcChecksums(((uint16_t*)bat + 2), (sizeof(*bat) - 4), chksum1, chksum2);
	
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
	// Clear the current MemCardFile list.
	qDeleteAll(lstMemCardFile);
	lstMemCardFile.clear();
	lstMemCardFile.reserve(NUM_ELEMENTS(mc_dat->entries));
	
	// Byteswap the directory table contents.
	for (int i = 0; i < NUM_ELEMENTS(mc_dat->entries); i++)
	{
		const card_direntry *direntry = &mc_dat->entries[i];
		
		// If the game code is 0xFFFFFFFF, the entry is empty.
		static const uint8_t gamecode_empty[4] = {0xFF, 0xFF, 0xFF, 0xFF};
		if (!memcmp(direntry->gamecode, gamecode_empty, sizeof(gamecode_empty)))
			continue;
		
		// Valid directory entry.
		MemCardFile *mcf = new MemCardFile(q, i, mc_dat, mc_bat);
		lstMemCardFile.append(mcf);
	}
}


/**
 * Calculate the card checksums.
 * @param buf Input data.
 * @param siz Length of input data.
 * @param chksum1 Checksum 1.
 * @param chksum2 Checksum 2.
 */
void MemCardPrivate::calcChecksums(uint16_t *buf, size_t siz, uint16_t *chksum1, uint16_t *chksum2)
{
	*chksum1 = 0;
	*chksum2 = 0;
	siz /= 2;
	
	for (size_t i = 0; i < siz; i++)
	{
		*chksum1 += be16_to_cpu(buf[i]);
		*chksum2 += (be16_to_cpu(buf[i]) ^ 0xFFFF);
	}
	
	if (*chksum1 == 0xFFFF)
		*chksum1 = 0;
	if (*chksum2 == 0xFFFF)
		*chksum2 = 0;
}


/** MemCard **/

MemCard::MemCard(QObject *parent, const QString& filename)
	: QObject(parent)
	, d(new MemCardPrivate(this, filename))
{
	// Initialize MemCardPrivate.
	d->init();
}

MemCard::MemCard(const QString& filename)
	: d(new MemCardPrivate(this, filename))
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
 * @return Size of memory card, in blocks.
 */
int MemCard::sizeInBlocks(void) const
	{ return (d->filesize / d->blockSize); }

/**
 * Get the memory card block size, in bytes.
 * @return Memory card block size, in bytes.
 */
int MemCard::blockSize(void) const
	{ return d->blockSize; }


/**
 * Read a block.
 * @param buf Buffer to read the block data into.
 * @param siz Size of buffer.
 * @param blockIdx Block index.
 * @return Bytes read on success; negative on error.
 */
int MemCard::readBlock(void *buf, size_t siz, uint16_t blockIdx)
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
 * Get the memory card encoding.
 * @return 0 for ANSI; 1 for SJIS; negative on error.
 */
int MemCard::encoding(void) const
{
	if (!isOpen())
		return -1;
	if (d->mc_header.encoding > SYS_FONT_ENCODING_MASK)
		return -2;
	
	return d->mc_header.encoding;
}


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
 * Get a MemCardFile object.
 * @param idx File number.
 * @return MemCardFile object, or NULL on error.
 */
MemCardFile *MemCard::getFile(int idx)
{
	if (idx < 0 || idx >= d->lstMemCardFile.size())
		return NULL;
	
	return d->lstMemCardFile.at(idx);
}
