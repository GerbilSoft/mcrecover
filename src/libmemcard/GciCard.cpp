/***************************************************************************
 * GameCube Memory Card Recovery Program [libmemcard]                      *
 * GciCard.cpp: GameCube GCI single-file class.                            *
 *                                                                         *
 * This is a wrapper class that allows loading of .gci files for editing   *
 * and template creation. Scanning for lost files is not supported.        *
 *                                                                         *
 * Copyright (c) 2012-2025 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#include "GciCard.hpp"
#include "util/byteswap.h"

// GcnFile
#include "GcnFile.hpp"

// C includes. (C++ namespace)
#include <cstring>
#include <cstdio>

// C++ includes.
#include <limits>
using std::list;

#define NUM_ELEMENTS(x) ((int)(sizeof(x) / sizeof(x[0])))

/** GciCardPrivate **/

#include "Card_p.hpp"
class GciCardPrivate : public CardPrivate
{
	typedef CardPrivate super;

public:
	explicit GciCardPrivate(GciCard *q);
	void init(void);

protected:
	Q_DECLARE_PUBLIC(GciCard)
private:
	Q_DISABLE_COPY(GciCardPrivate)

private:
	// Cached copy of the GCI directory entry
	card_direntry dirEntry;

public:
	/**
	 * Open an existing GCI file.
	 * @param filename GCI filename
	 * @return 0 on success; non-zero on error. (also check errorString)
	 */
	int open(const QString &filename);
};

GciCardPrivate::GciCardPrivate(GciCard *q)
	: super(q,
		8192,	// 8 KB blocks.
		1,	// Minimum card size, in blocks.
		2043,	// Maximum card size, in blocks.
		1,	// Number of directory tables.
		1,	// Number of block tables.
		64)	// Header size. (offset to actual data area)
{
	// Clear variables.
	memset(&dirEntry, 0, sizeof(dirEntry));

	// GCI files are *not* writable.
	canMakeWritable = false;
}

/**
 * Open an existing Memory Card image.
 * @param filename Memory Card image filename
 * @return 0 on success; non-zero on error. (also check errorString)
 */
int GciCardPrivate::open(const QString &filename)
{
	int ret = CardPrivate::open(filename, QIODevice::ReadOnly);
	if (ret != 0) {
		// Error opening the file.
		return ret;
	}

	// Load the directory entry.
	// This is the first 64 bytes of the GCI file.
	file->seek(0);
	qint64 sz = file->read((char*)&dirEntry, sizeof(dirEntry));
	if (sz != (qint64)sizeof(dirEntry)) {
		// Error reading the card header.
		this->errors |= Card::MCE_SHORT_READ;
		memset(&dirEntry, 0, sizeof(dirEntry));
		return -1;
	}

	// Fake block count.
	totalUserBlocks = totalPhysBlocks;
	freeBlocks = 0;

	// Block and directory tables are "valid".
	bat_info.active = 0;
	dat_info.active = 0;
	bat_info.valid = 1;
	dat_info.valid = 1;
	bat_info.valid_freeblocks = 1;
	dat_info.valid_freeblocks = 1;

#if SYS_BYTEORDER != SYS_BIG_ENDIAN
	// Byteswap the directory entry.
	dirEntry.lastmodified	= be32_to_cpu(dirEntry.lastmodified);
	dirEntry.iconaddr	= be32_to_cpu(dirEntry.iconaddr);
	dirEntry.iconfmt	= be16_to_cpu(dirEntry.iconfmt);
	dirEntry.iconspeed	= be16_to_cpu(dirEntry.iconspeed);
	dirEntry.block		= be16_to_cpu(dirEntry.block);
	dirEntry.length		= be16_to_cpu(dirEntry.length);
	dirEntry.commentaddr	= be32_to_cpu(dirEntry.commentaddr);
#endif /* SYS_BYTEORDER != SYS_BIG_ENDIAN */

	// Set encoding to Shift-JIS if the region byte is 'J'.
	this->encoding = (dirEntry.gamecode[3] == 'J')
		? Card::Encoding::Shift_JIS
		: Card::Encoding::CP1252;

	// Adjust the "first block" value, since it might be incorrect.
	dirEntry.block = 0;

	// Add the directory entry to the file list.
	Q_Q(GciCard);
	GcnFile *const mcFile = new GcnFile(q, &dirEntry, std::vector<uint16_t>());
	emit q->filesAboutToBeInserted(0, 0);
	lstFiles.append(mcFile);
	emit q->filesInserted();

	// Block count has changed.
	emit q->blockCountChanged(totalPhysBlocks, totalUserBlocks, freeBlocks);
	return 0;
}

/** GciCard **/

GciCard::GciCard(QObject *parent)
	: super(new GciCardPrivate(this), parent)
{}

/**
 * Open an existing GCI file.
 * @param filename Filename
 * @param parent Parent object
 * @return GciCard object, or nullptr on error.
 */
GciCard *GciCard::open(const QString& filename, QObject *parent)
{
	GciCard *const gciFile = new GciCard(parent);
	GciCardPrivate *const d = gciFile->d_func();
	d->open(filename);
	// NOTE: GCI files aren't powers of two, so clear that error.
	// TODO: Report a different incorrect size error. (Must be a multiple of 8 KiB + 64.)
	d->errors &= ~Card::MCE_SZ_NON_POW2;
	return gciFile;
}

/** Card information **/

/**
 * Get the product name of this memory card.
 * This refers to the class in general,
 * and does not change based on size.
 * @return Product name
 */
QString GciCard::productName(void) const
{
	return tr("GameCube save file");
}
