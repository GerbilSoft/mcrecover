/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * VmuCard.cpp: Dreamcast VMU memory card class.                           *
 *                                                                         *
 * Copyright (c) 2015 by David Korth.                                      *
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

#include "VmuCard.hpp"
#include "util/byteswap.h"

// C includes. (C++ namespace)
#include <cstring>
#include <cstdio>
#include <cerrno>

// C++ includes.
#include <limits>

#define NUM_ELEMENTS(x) ((int)(sizeof(x) / sizeof(x[0])))

// VMU card definitions.
#include "vmu.h"

/** VmuCardPrivate **/

#include "Card_p.hpp"
class VmuCardPrivate : public CardPrivate
{
	public:
		VmuCardPrivate(VmuCard *q);
		virtual ~VmuCardPrivate();
		void init(void);

	protected:
		Q_DECLARE_PUBLIC(VmuCard)
	private:
		Q_DISABLE_COPY(VmuCardPrivate)

	public:
		/**
		 * Open an existing VMU image.
		 * @param filename VMU image filename.
		 * @return 0 on success; non-zero on error. (also check errorString)
		 */
		int open(const QString &filename);

		/**
		 * Format a new VMU image.
		 * @param filename VMU image filename.
		 * TODO: Parameters.
		 * @return 0 on success; non-zero on error. (also check errorString)
		 */
		int format(const QString &filename);

	public:
		vmu_root_block mc_root;
		vmu_fat mc_fat; // TODO: Multi-block FATs?
		// TODO: Directory.

	private:
		/**
		 * Load the memory card system information.
		 * @return 0 on success; non-zero on error.
		 */
		int loadSysInfo(void);
};

VmuCardPrivate::VmuCardPrivate(VmuCard *q)
	: CardPrivate(q, VMU_BLOCK_SIZE, 256, 256)
{
	// Make sure vmu.h is correct.
	static_assert(VMU_BLOCK_SIZE == 512, "VMU_BLOCK_SIZE is incorrect");
	static_assert(sizeof(vmu_timestamp) == 8, "vmu_timestamp has the wrong size");
	static_assert(sizeof(vmu_root_block) == VMU_BLOCK_SIZE, "vmu_root_block has the wrong size");
	static_assert(sizeof(vmu_fat) == VMU_BLOCK_SIZE, "vmu_fat has the wrong size");
	static_assert(VMU_DIR_ENTRY_LEN == 0x20, "vmu_dir_entry is incorrect");
	static_assert(sizeof(vmu_dir_entry) == VMU_DIR_ENTRY_LEN, "vmu_dir_entry has the wrong size");
	static_assert(VMU_FILE_HEADER_LEN == 96, "VMU_FILE_HEADER_LEN is incorrect");
	static_assert(sizeof(vmu_file_header) == VMU_FILE_HEADER_LEN, "vmu_file_header has the wrong size");
	static_assert(VMU_ICON_PALETTE_LEN == 32, "VMU_ICON_PALETTE_LEN is incorrect");
	static_assert(sizeof(vmu_icon_palette) == VMU_ICON_PALETTE_LEN, "vmu_icon_palette has the wrong size");
	static_assert(VMU_ICON_DATA_LEN == 512, "VMU_ICON_DATA_LEN is incorrect");
	static_assert(sizeof(vmu_icon_data) == VMU_ICON_DATA_LEN, "vmu_icon_data has the wrong size");
	static_assert(VMU_EYECATCH_NONE_LEN == 0, "VMU_EYECATCH_NONE_LEN is incorrect");
	static_assert(VMU_EYECATCH_TRUECOLOR_LEN == 8064, "VMU_EYECATCH_TRUECOLOR_LEN is incorrect");
	static_assert(sizeof(vmu_eyecatch_truecolor) == VMU_EYECATCH_TRUECOLOR_LEN, "vmu_eyecatch_truecolor has the wrong size");
	static_assert(VMU_EYECATCH_PALETTE_256_LEN == 4544, "VMU_EYECATCH_PALETTE_256_LEN is incorrect");
	static_assert(sizeof(vmu_eyecatch_palette_256) == VMU_EYECATCH_PALETTE_256_LEN, "vmu_eyecatch_palette_256 has the wrong size");
	static_assert(VMU_EYECATCH_PALETTE_16_LEN == 2048, "VMU_EYECATCH_PALETTE_256_LEN is incorrect");
	static_assert(sizeof(vmu_eyecatch_palette_16) == VMU_EYECATCH_PALETTE_16_LEN, "vmu_eyecatch_palette_16 has the wrong size");

	// Clear variables.
	memset(&mc_root, 0, sizeof(mc_root));
	memset(&mc_fat, 0, sizeof(mc_fat));
}

VmuCardPrivate::~VmuCardPrivate()
{
	// TODO: Remove this?
}

/**
 * Open an existing VMU image.
 * @param filename VMU image image filename.
 * @return 0 on success; non-zero on error. (also check errorString)
 */
int VmuCardPrivate::open(const QString &filename)
{
	int ret = CardPrivate::open(filename, QIODevice::ReadOnly);
	if (ret != 0) {
		// Error opening the file.
		return ret;
	}

	// Load the VMU-specific data.

	// Load the memory card system information.
	// This includes the root block, directory, and FAT.
	loadSysInfo();

	return 0;
}

/**
 * Format a new VMU image.
 * @param filename VMU image filename.
 * TODO: Parameters.
 * @return 0 on success; non-zero on error. (also check errorString)
 */
int VmuCardPrivate::format(const QString &filename)
{
	// TODO
	Q_UNUSED(filename);
	return -ENOSYS;
}

/**
 * Load the memory card system information.
 * TODO: Error code constants?
 * @return 0 on success; non-zero on error.
 */
int VmuCardPrivate::loadSysInfo(void)
{
	if (!file)
		return -1;

	// Root block.
	file->seek(VMU_ROOT_BLOCK_ADDRESS * blockSize);
	qint64 sz = file->read((char*)&mc_root, sizeof(mc_root));
	if (sz < (qint64)sizeof(mc_root)) {
		// Error reading the root block.
		// Zero the root block, directory, and FAT.
		// TODO
		memset(&mc_root, 0x00, sizeof(mc_root));
		memset(&mc_fat, 0x00, sizeof(mc_fat));

		// Use cp1252 encoding by default.
		this->encoding = Card::ENCODING_CP1252;

		return -2;
	}

	// Byteswap the root block contents.
	mc_root.fat_addr	= le16_to_cpu(mc_root.fat_addr);
	mc_root.fat_size	= le16_to_cpu(mc_root.fat_size);
	mc_root.dir_addr	= le16_to_cpu(mc_root.dir_addr);
	mc_root.dir_size	= le16_to_cpu(mc_root.dir_size);
	mc_root.icon		= le16_to_cpu(mc_root.icon);
	mc_root.user_blocks	= le16_to_cpu(mc_root.user_blocks);

	// Assume cp1252 encoding.
	// There's no per-card "encoding" value like on GameCube.
	// Actual encoding is determined by the system being used.
	// TODO: Add a "no encoding" option?
	this->encoding = Card::ENCODING_CP1252;

	// VMU color.
	if (mc_root.color_type == VMU_COLOR_CUSTOM) {
		// Custom color specified.
		this->color = QColor(
			mc_root.color_red,
			mc_root.color_green,
			mc_root.color_blue,
			mc_root.color_alpha);
	} else {
		// "Standard" color.
		this->color = QColor();
	}

	// TODO: Card timestamp is in BCD, so set it.

	// TODO: Load the FAT and directory.
	// Set the block counts.
	// TODO: Determine free blocks by looking at the FAT.
	// TODO: Verify that user_blocks is in range.
	totalUserBlocks = mc_root.user_blocks;
	freeBlocks = totalUserBlocks; // TODO
	Q_Q(VmuCard);
	emit q->blockCountChanged(totalPhysBlocks, totalUserBlocks, freeBlocks);
	return 0;
}

/** VmuCard **/

VmuCard::VmuCard(QObject *parent)
	: Card(new VmuCardPrivate(this), parent)
{ }

VmuCard::~VmuCard()
{
	// TODO: Is this needed anymore?
}

/**
 * Open an existing VMU image.
 * @param filename VMU image filename.
 * @param parent Parent object.
 * @return VmuCard object, or nullptr on error.
 */
VmuCard *VmuCard::open(const QString& filename, QObject *parent)
{
	VmuCard *vmuCard = new VmuCard(parent);
	VmuCardPrivate *const d = vmuCard->d_func();
	d->open(filename);
	return vmuCard;
}

/**
 * Format a new Memory Card image.
 * @param filename VMU image filename.
 * @param parent Parent object.
 * @return VmuCard object, or nullptr on error.
 */
VmuCard *VmuCard::format(const QString& filename, QObject *parent)
{
	// Format a new VmuCard.
	// TODO: Parameters.
	VmuCard *vmuCard = new VmuCard(parent);
	VmuCardPrivate *const d = vmuCard->d_func();
	d->format(filename);
	return vmuCard;
}

/**
 * Get the product name of this memory card.
 * This refers to the class in general,
 * and does not change based on size.
 * @return Product name.
 */
QString VmuCard::productName(void) const
{
	return tr("Dreamcast Visual Memory Unit");
}
