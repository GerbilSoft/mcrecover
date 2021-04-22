/***************************************************************************
 * GameCube Memory Card Recovery Program [libmemcard]                      *
 * VmuCard.cpp: Dreamcast VMU memory card class.                           *
 *                                                                         *
 * Copyright (c) 2015-2021 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#include "VmuCard.hpp"
#include "util/byteswap.h"

#include "VmuFile.hpp"
#include "GcToolsQt.hpp"
#include "TimeFuncs.hpp"

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
	typedef CardPrivate super;

	public:
		explicit VmuCardPrivate(VmuCard *q);
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

		// Directory. (Assuming it's always 13 blocks.)
		// TODO: Variable-length?
		#define VMU_DIR_LEN 13
		#define VMU_DIR_ENTRIES (VMU_BLOCK_SIZE * VMU_DIR_LEN / sizeof(vmu_dir_entry))
		vmu_dir_entry mc_dir[VMU_DIR_ENTRIES];

	private:
		/**
		 * Load the memory card system information.
		 * @return 0 on success; non-zero on error.
		 */
		int loadSysInfo(void);

		/**
		 * Load the FAT.
		 * @return 0 on success; non-zero on error.
		 */
		int loadFat(void);

		/**
		 * Calculate the block counts.
		 */
		void calcBlockCounts(void);

		/**
		 * Load the directory.
		 * @return 0 on success; non-zero on error.
		 */
		int loadDir(void);

		/**
		 * Load the File list.
		 */
		void loadFileList(void);
};

VmuCardPrivate::VmuCardPrivate(VmuCard *q)
	: super(q,
		VMU_BLOCK_SIZE,	// 512-byte blocks.
		256,	// Minimum card size, in blocks.
		256,	// Maximum card size, in blocks.
		1,	// Number of directory tables.
		1)	// Number of block tables.
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
	static_assert(VMU_CARD_ICON_DATA_HEADER_LEN == 24, "VMU_CARD_ICON_DATA_HEADER_LEN is incorrect");
	static_assert(sizeof(vmu_card_icon_header) == VMU_CARD_ICON_DATA_HEADER_LEN, "vmu_card_icon_header has the wrong size");
	static_assert(VMU_CARD_ICON_MONO_DATA_LEN == 128, "VMU_CARD_ICON_MONO_DATA_LEN is incorrect");
	static_assert(sizeof(vmu_card_icon_mono_data) == VMU_CARD_ICON_MONO_DATA_LEN, "vmu_card_icon_mono_data has the wrong size");
	static_assert(VMU_CARD_ICON_COLOR_DATA_LEN == 544, "VMU_CARD_ICON_COLOR_DATA_LEN is incorrect");
	static_assert(sizeof(vmu_card_icon_color_data) == VMU_CARD_ICON_COLOR_DATA_LEN, "vmu_card_icon_color_data has the wrong size");

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
	ret = loadSysInfo();
	if (ret != 0) {
		// Error loading system information.
		return ret;
	}

	// Load the File list.
	// TODO: Change it to return an error code?
	loadFileList();

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

	// FIXME: Assuming 1 DAT and 1 BAT.
	dat_info.active_hdr = 0;
	dat_info.active = 0;
	dat_info.valid = 1;
	bat_info.active_hdr = 0;
	bat_info.active = 0;
	bat_info.valid = 1;
	// NOTE: VMU doesn't store a free block count.
	// TODO: Indicate this in a "supported features" flag.
	bat_info.valid_freeblocks = 1;

	// Root block.
	file->seek(VMU_ROOT_BLOCK_ADDRESS * blockSize);
	qint64 sz = file->read((char*)&mc_root, sizeof(mc_root));
	if (sz < (qint64)sizeof(mc_root)) {
		// Error reading the root block.
		// Zero the root block, directory, and FAT.
		memset(&mc_root, 0x00, sizeof(mc_root));
		memset(&mc_fat, 0x00, sizeof(mc_fat));
		memset(&mc_dir, 0x00, sizeof(mc_dir));

		// Use cp1252 encoding by default.
		// TODO: "No encoding".
		this->encoding = Card::ENCODING_CP1252;
		return -2;
	}

	// The first 16 bytes should be 0x55.
	for (int i = 0; i < NUM_ELEMENTS(mc_root.format55); i++) {
		if (mc_root.format55[i] != 0x55) {
			// Invalid header.
			// TODO: Set an error flag instead of returning?
			return -2;
		}
	}

#if SYS_BYTEORDER != SYS_LIL_ENDIAN
	// Byteswap the root block contents.
	mc_root.fat_addr	= le16_to_cpu(mc_root.fat_addr);
	mc_root.fat_size	= le16_to_cpu(mc_root.fat_size);
	mc_root.dir_addr	= le16_to_cpu(mc_root.dir_addr);
	mc_root.dir_size	= le16_to_cpu(mc_root.dir_size);
	mc_root.icon		= le16_to_cpu(mc_root.icon);
	mc_root.user_blocks	= le16_to_cpu(mc_root.user_blocks);
#endif /* SYS_BYTEORDER != SYS_LIL_ENDIAN */

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

	// Set the format time.
	this->formatTime = TimeFuncs::fromVmuTimestamp(mc_root.timestamp);

	// VMU icon.
	if (mc_root.icon <= 123) {
		// Extract the icon from the sprite sheet.
		// TODO: Load the sprite sheet once and save it as a static variable?
		QImage sprsheet(QLatin1String(":/hw/bios.png"));
		QRect rect(mc_root.icon / 16, mc_root.icon % 16, VMU_ICON_W, VMU_ICON_H);
		QImage img = sprsheet.copy(rect);

		// The included icons use a black-and-white palette.
		// Update it to match the VMU's LCD colors.
		QVector<QRgb> colorTable = img.colorTable();
		if (colorTable.count() == 2) {
			for (int i = colorTable.count()-1; i >= 0; i--) {
				QRgb color = colorTable[i] & 0xFFFFFF;
				if (color == 0) {
					// Black. Change to "blue".
					colorTable[i] = qRgb(8, 24, 132);
				} else {
					// White. Change to "green".
					colorTable[i] = qRgb(140, 206, 173);
				}
			}
			img.setColorTable(colorTable);
		}

		// Convert to QPixmap.
		this->icon = QPixmap::fromImage(img);
	}

	// Load the FAT.
	int ret = loadFat();
	if (ret != 0) {
		// Error loading the FAT.
		// Zero the root block, directory, and FAT.
		memset(&mc_root, 0x00, sizeof(mc_root));
		memset(&mc_fat, 0x00, sizeof(mc_fat));
		memset(&mc_dir, 0x00, sizeof(mc_dir));
		return -3;
	}

	// Load the directory.
	ret = loadDir();
	if (ret != 0) {
		// Error loading the directory.
		// Zero the root block, directory, and FAT.
		memset(&mc_root, 0x00, sizeof(mc_root));
		memset(&mc_fat, 0x00, sizeof(mc_fat));
		memset(&mc_dir, 0x00, sizeof(mc_dir));
		return -4;
	}

	return 0;
}

/**
 * Load the FAT.
 * @return 0 on success; non-zero on error.
 */
int VmuCardPrivate::loadFat(void)
{
	// Only single-block FATs are supported right now.
	if (mc_root.fat_size != 1) {
		// FAT is the wrong size.
		// TODO: Set an error flag.
		return -1;
	} else if (mc_root.fat_addr >= VMU_ROOT_BLOCK_ADDRESS) {
		// FAT address is invalid.
		return -2;
	}

	file->seek(mc_root.fat_addr * blockSize);
	quint64 sz = file->read((char*)&mc_fat, sizeof(mc_fat));
	if (sz != sizeof(mc_fat)) {
		// Error reading the FAT.
		return -3;
	}

#if SYS_BYTEORDER != SYS_LIL_ENDIAN
	// Byteswap the FAT.
	for (int i = (NUM_ELEMENTS(mc_fat.fat)-1); i >= 0; i--) {
		mc_fat.fat[i] = le16_to_cpu(mc_fat.fat[i]);
	}
#endif /* SYS_BYTEORDER != SYS_LIL_ENDIAN */

	// Calculate the block counts.
	calcBlockCounts();

	return 0;
}

/**
 * Calculate the block counts.
 */
void VmuCardPrivate::calcBlockCounts(void)
{
	// Set the block counts.
	totalUserBlocks = mc_root.user_blocks;
	if (totalUserBlocks > totalPhysBlocks) {
		// User block count is too high.
		// TODO: Set an error.
		totalUserBlocks = totalPhysBlocks;
	}

	int cnt = 0;
	for (int i = (totalUserBlocks-1); i >= 0; i--) {
		if (mc_fat.fat[i] == VMU_FAT_BLOCK_UNALLOCATED)
			cnt++;
	}
	freeBlocks = cnt;

	Q_Q(VmuCard);
	emit q->blockCountChanged(totalPhysBlocks, totalUserBlocks, freeBlocks);
}

/**
 * Load the directory.
 * @return 0 on success; non-zero on error.
 */
int VmuCardPrivate::loadDir(void)
{
	// Only 13-block directories are supported right now.
	if (mc_root.dir_size != VMU_DIR_LEN) {
		// Directory is the wrong size.
		// TODO: Set an error flag.
		return -1;
	} else if (mc_root.dir_addr >= VMU_ROOT_BLOCK_ADDRESS) {
		// Directory address is invalid.
		return -2;
	}

	// Read the directory.
	// NOTE: The VMS file system likes to store files backwards.
	// Block 253 is the first block of directory;
	// Block 252 is the second block, etc.
	const int lastBlock = (mc_root.dir_addr - mc_root.dir_size + 1);
	vmu_dir_entry *dir = mc_dir;
	for (int block = mc_root.dir_addr; block >= lastBlock;
	     block--, dir += (VMU_BLOCK_SIZE / sizeof(*dir))) {
		const int address = (block * blockSize);
		file->seek(address);
		qint64 sz = file->read((char*)dir, VMU_BLOCK_SIZE);
		if (sz < (qint64)VMU_BLOCK_SIZE) {
			// Error reading the directory table.
			return -3;
		}
	}

#if SYS_BYTEORDER != SYS_LIL_ENDIAN
	// Byteswap the directory table contents.
	for (int i = 0; i < NUM_ELEMENTS(mc_dir); i++) {
		vmu_dir_entry *dirEntry	= &mc_dir[i];
		dirEntry->address	= le16_to_cpu(dirEntry->address);
		dirEntry->size		= le16_to_cpu(dirEntry->size);
		dirEntry->header_addr	= le16_to_cpu(dirEntry->header_addr);
	}
#endif /* SYS_BYTEORDER != SYS_LIL_ENDIAN */

	return 0;
}

/**
 * Load the File list.
 */
void VmuCardPrivate::loadFileList(void)
{
	if (!file)
		return;

	Q_Q(VmuCard);

	// Clear the current File list.
	int init_size = lstFiles.size();
	if (init_size > 0)
		emit q->filesAboutToBeRemoved(0, (init_size - 1));
	qDeleteAll(lstFiles);
	lstFiles.clear();
	if (init_size > 0)
		emit q->filesRemoved();

	// Reset the used block map.
	// TODO
	//resetUsedBlockMap();

	QVector<File*> lstFiles_new;
	lstFiles_new.reserve(NUM_ELEMENTS(mc_dir));

	// Byteswap the directory table contents.
	for (int i = 0; i < NUM_ELEMENTS(mc_dir); i++) {
		const vmu_dir_entry *dirEntry = &mc_dir[i];

		// If the filetype is 0x00, the file is empty.
		if (dirEntry->filetype == 0x00)
			continue;

		// Valid directory entry.
		VmuFile *vmuFile = new VmuFile(q, dirEntry, &mc_fat);
		lstFiles_new.append(vmuFile);

		// Is this file ICONDATA_VMS?
		if (vmuFile->filename() == QLatin1String("ICONDATA_VMS")) {
			// Found ICONDATA_VMS.
			const GcImage *img = vmuFile->vmu_icondata_color();
			if (!img) {
				// Color icon is missing.
				// Check the monochrome icon.
				img = vmuFile->vmu_icondata_mono();
			}

			if (img) {
				// ICONDATA_VMS has an icon.
				this->icon = QPixmap::fromImage(gcImageToQImage(img));
			}
		}

		// Mark the file's blocks as used.
		// TODO
		/*
		QVector<uint16_t> fatEntries = vmuFile->fatEntries();
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
		*/
	}

	if (!lstFiles_new.isEmpty()) {
		// Files have been added to the memory card.
		emit q->filesAboutToBeInserted(0, (lstFiles_new.size() - 1));
		// NOTE: QVector::swap() was added in qt-4.8.
		lstFiles = lstFiles_new;
		emit q->filesInserted();
	}

	// Block count has changed.
	emit q->blockCountChanged(totalPhysBlocks, totalUserBlocks, freeBlocks);
}

/** VmuCard **/

VmuCard::VmuCard(QObject *parent)
	: super(new VmuCardPrivate(this), parent)
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

/** File system **/

/**
 * Set the active Directory Table index.
 * NOTE: This function reloads the file list, without lost files.
 * @param idx Active Directory Table index.
 */
void VmuCard::setActiveDatIdx(int idx)
{
	if (!isOpen())
		return;
	// DC has a single directory table.
	Q_UNUSED(idx)
}

/**
 * Set the active Block Table index.
 * NOTE: This function reloads the file list, without lost files.
 * @param idx Active Block Table index. (0 or 1)
 */
void VmuCard::setActiveBatIdx(int idx)
{
	if (!isOpen())
		return;
	// DC has a single block table.
	Q_UNUSED(idx)
}

/** Card information **/

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
