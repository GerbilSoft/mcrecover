/***************************************************************************
 * GameCube Memory Card Recovery Program [libmemcard]                      *
 * Card.cpp: Memory Card physical layer. [base class]                      *
 *                                                                         *
 * Copyright (c) 2012-2025 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#include "Card.hpp"
#include "Card_p.hpp"
#include "File.hpp"

// C includes. (C++ namespace)
#include <cstring>
#include <cstdio>
#include <cassert>

// C++ includes.
#include <limits>

// Qt includes.
#include <QtCore/QFile>
#include <QtCore/QVector>

#define NUM_ELEMENTS(x) ((int)(sizeof(x) / sizeof(x[0])))

/** CardPrivate **/

CardPrivate::CardPrivate(Card *q, uint32_t blockSize,
			 int minBlocks, int maxBlocks,
			 int dat_count, int bat_count,
			 uint32_t headerSize)
	: q_ptr(q)
	, errors(QFlags<Card::Error>())
	, file(nullptr)
	, filesize(0)
	, readOnly(true)
	, canMakeWritable(false)
	, encoding(Card::Encoding::Unknown)
	, blockSize(blockSize)
	, headerSize(headerSize)
	, minBlocks(minBlocks)
	, maxBlocks(maxBlocks)
	, totalPhysBlocks(0)
	, totalUserBlocks(0)
	, freeBlocks(0)
{
	assert(isPow2(blockSize));
	assert(blockSize > 0);
	assert(minBlocks > 0);
	assert(maxBlocks >= minBlocks);
	assert(dat_count > 0 && dat_count < 32);
	assert(bat_count > 0 && bat_count < 32);

	// Directory Table information.
	dat_info.count = dat_count;
	dat_info.active = -1;
	dat_info.active_hdr = 0;
	dat_info.valid = 0;

	// Block Table information.
	bat_info.count = bat_count;
	bat_info.active = -1;
	bat_info.active_hdr = 0;
	bat_info.valid = 0;
}

CardPrivate::~CardPrivate()
{
	// Clear the File list.
	qDeleteAll(lstFiles);
	lstFiles.clear();

	if (file) {
		file->close();
		delete file;
	}
}

/**
 * Open a Memory Card image.
 * totalPhysBlocks is initialized after the file is opened.
 * totalUserBlocks and freeBlocks must be initialized by the subclass.
 * @param filename Memory Card image filename.
 * @param openMode File open mode.
 * @return 0 on success; non-zero on error. (also check errorString)
 */
int CardPrivate::open(const QString &filename, QIODevice::OpenModeFlag openMode)
{
	if (file) {
		// File is already open.
		// TODO: Don't allow this, or clear all variables?
		close();
	}

	if (filename.isEmpty()) {
		// No filename specified.
		// TODO: Translate the error message.
		// NOTE: Same message as Qt. (note the space between "file" and "name")
		this->errorString = QLatin1String("No file name specified");
		return -1;
	}

	// Open the file.
	Q_Q(Card);
	QFile *tmp_file = new QFile(filename, q);
	if (!tmp_file->open(openMode)) {
		// Error opening the file.
		// NOTE: Qt doesn't return the raw error number.
		// QFile::error() has a useless generic error number.
		// TODO: Translate the error message.
		this->errorString = tmp_file->errorString();
		delete tmp_file;
		return -1;
	}
	this->file = tmp_file;
	this->filename = filename;

	// Save the readOnly flag.
	this->readOnly = !(openMode & QIODevice::WriteOnly);

	// TODO: If formatting the card, skip all of this.

	// Get the filesize.
	this->filesize = file->size();

	// Calculate the size in blocks.
	totalPhysBlocks = ((this->filesize - headerSize)/ blockSize);

	// Make sure the size isn't out of range.
	if (totalPhysBlocks < minBlocks) {
		// Not enough blocks.
		this->errors |= Card::MCE_SZ_TOO_SMALL;
	} else if (totalPhysBlocks > maxBlocks) {
		// Too many blocks.
		// Only read up to maxBlocks.
		this->errors |= Card::MCE_SZ_TOO_BIG;
		this->filesize = (static_cast<quint64>(maxBlocks) * blockSize);
	}

	if (!isPow2(this->filesize)) {
		// Size is not a power of 2.
		this->errors |= Card::MCE_SZ_NON_POW2;
	}

	// Card is open.
	return 0;
}

/**
 * Close the currently-opened Memory Card image.
 * This will clear all cached file information.
 */
void CardPrivate::close(void)
{
	if (!file) {
		// Card is not open.
		return;
	}

	file->close();
	delete file;
	file = nullptr;

	// Clear the cached values.
	filename.clear();
	filesize = 0;
	totalPhysBlocks = 0;
	totalUserBlocks = 0;
	freeBlocks = 0;
}

/**
 * Find the most common byte in a block of data.
 * This is useful for determining header garbage.
 * @param buf		[in] Data block.
 * @param siz		[in] Size of buf.
 * @param most_byte	[out] Byte that appears the most times in buf.
 * @param count		[out] Number of times most_byte appears.
 */
void CardPrivate::findMostCommonByte(const uint8_t *buf, size_t siz, uint8_t *most_byte, int *count)
{
	int bytes[256];
	memset(bytes, 0, sizeof(bytes));

	// Check the buffer.
	// TODO: Loop unrolling optimization; decrement optimization?
	for (size_t i = 0; i < siz; i++, buf++) {
		++bytes[*buf];
	}

	// Find the most common byte.
	uint8_t tmpbyte = 255;
	int tmpcnt = bytes[255];
	for (int i = 254; i >= 0; i--) {
		if (bytes[i] > tmpcnt) {
			tmpbyte = (uint8_t)i;
			tmpcnt = bytes[i];
		}
	}

	if (most_byte) {
		*most_byte = tmpbyte;
	}
	if (count) {
		*count = tmpcnt;
	}
}

/** Card **/

/**
 * Create a new Card object.
 * @param d CardPrivate-derived private class.
 * @param parent Parent object.
 */
Card::Card(CardPrivate *d, QObject *parent)
	: super(parent)
	, d_ptr(d)
{
	qRegisterMetaType<Encoding>();
}

Card::~Card()
{
	delete d_ptr;
}

/**
 * Check if the memory card is open.
 * @return True if open; false if not.
 */
bool Card::isOpen(void) const
{
	Q_D(const Card);
	return !!(d->file);
}

/**
 * Get the last error string.
 * Usually used for open() errors.
 * TODO: Change to error code constants for translation?
 * @return Error string.
 */
QString Card::errorString(void) const
{
	Q_D(const Card);
	return d->errorString;
}

/** Writing functions. **/

/**
 * Is this card read-only?
 *
 * This is true if the card has not been set to writable,
 * or if there are errors on the card and hence it cannot
 * be set to writable.
 *
 * @return True if this card is read-only; false if not.
 */
bool Card::isReadOnly(void) const
{
	Q_D(const Card);
	if (!isOpen())
		return true;
	return d->readOnly;
}

/**
 * Attempt to switch the card from read-only to read-write or vice-versa.
 *
 * Note that this will fail with ENOTTY if any errors are detected
 * on the card, since writing to a card with errors can cause even
 * more problems.
 *
 * @param readOnly New readOnly value.
 * @return 0 on success; negative POSIX error code on error.
 * (Check this->errorString for more information.)
 */
int Card::setReadOnly(bool readOnly)
{
	Q_D(Card);
	if (!isOpen())
		return -EBADF;
	if (d->readOnly == readOnly)
		return 0;

	// Check if any errors are present.
	if (d->errors != 0) {
		// Errors are present.
		// Cannot reopen.
		return -ENOTTY;
	}

	if (!readOnly && !d->canMakeWritable) {
		// Cannot make this card writable.
		return -EROFS;
	}

	// Open mode.
	const QIODevice::OpenMode openMode = (readOnly ? QIODevice::ReadOnly : QIODevice::ReadWrite);

	// Attempt to open the file using a new QFile.
	// FIXME: Do we need to close the first QFile due to sharing?
	// Open the file.
	QFile *tmp_file = new QFile(d->filename, this);
	if (!tmp_file->open(openMode)) {
		// Error opening the file.
		// NOTE: Qt doesn't return the raw error number.
		// QFile::error() has a useless generic error number.
		// TODO: Translate the error message.
		// TODO: Convert into a POSIX error code?
		d->errorString = tmp_file->errorString();
		delete tmp_file;
		return -EIO;
	}

	// TODO: Validate that this file is the same as the one we had before.
	// TODO: Atomic swap of d->file and tmp_file.
	std::swap(d->file, tmp_file);
	d->readOnly = readOnly;
	tmp_file->close();
	delete tmp_file;
	return 0;
}

/**
 * Can this card be made writable?
 * @return True if it can; false if it can't.
 */
bool Card::canMakeWritable(void) const
{
	Q_D(const Card);
	return d->canMakeWritable;
}

/** Card information **/

/**
 * Get the memory card filename.
 * @return Memory card filename, or empty string if not open.
 */
QString Card::filename(void) const
{
	Q_D(const Card);
	return d->filename;
}

/**
 * Get the size of the memory card image, in bytes.
 * This is the full size of the memory card image.
 * @return Size of the memory card image, in bytes. (0 on error)
 */
quint64 Card::filesize(void) const
{
	if (!isOpen())
		return 0;
	Q_D(const Card);
	return d->file->size();
}

/**
 * Get the memory card block size, in bytes.
 * @return Memory card block size, in bytes. (Negative on error)
 */
int Card::blockSize(void) const
{
	if (!isOpen())
		return -1;
	Q_D(const Card);
	return d->blockSize;
}

/**
 * Get the minimum number of blocks allowed for this card.
 * @return Minimum number of blocks. (Negative on error)
 */
int Card::minBlocks(void) const
{
	if (!isOpen())
		return -1;
	Q_D(const Card);
	return d->minBlocks;
}

/**
 * Get the maximum number of blocks allowed for this card.
 * @return Maximum number of blocks. (Negative on error)
 */
int Card::maxBlocks(void) const
{
	if (!isOpen())
		return -1;
	Q_D(const Card);
	return d->maxBlocks;
}

/**
 * Get the total number of physical blocks.
 * This includes reserved blocks.
 * @return Total number of physical blocks. (Negative on error)
 */
int Card::totalPhysBlocks(void) const
{
	if (!isOpen())
		return -1;
	Q_D(const Card);
	return d->totalPhysBlocks;
}

/**
 * Get the total number of user-accessible blocks.
 * This does NOT include reserved blocks.
 * @return Total number of user-accessible blocks. (Negative on error)
 */
int Card::totalUserBlocks(void) const
{
	if (!isOpen())
		return -1;
	Q_D(const Card);
	return d->totalUserBlocks;
}

/**
 * Get the number of free blocks.
 * @return Free blocks. (Negative on error)
 */
int Card::freeBlocks(void) const
{
	if (!isOpen())
		return -1;
	Q_D(const Card);
	return d->freeBlocks;
}

/**
 * Get the text encoding used for filenames and descriptions.
 * @return Text encoding.
 */
Card::Encoding Card::encoding(void) const
{
	if (!isOpen())
		return Card::Encoding::Unknown;
	Q_D(const Card);
	return d->encoding;
}

/**
 * Get the card's color.
 * @return Color. (If not available, an invalid QColor is returned.)
 */
QColor Card::color(void) const
{
	if (!isOpen())
		return QColor();
	Q_D(const Card);
	return d->color;
}

/**
 * Get the card's format time.
 * @return Format time. (If not available, will return Unix epoch.)
 */
QDateTime Card::formatTime(void) const
{
	if (!isOpen())
		return QDateTime();
	Q_D(const Card);
	return d->formatTime;
}

/**
 * Get the card's icon.
 * @return Icon, or default system icon or null QPixmap if unavailable.
 */
QPixmap Card::icon(void) const
{
	if (!isOpen())
		return QPixmap();
	Q_D(const Card);
	return d->icon;
}

/** File system **/

/**
 * Get the number of directory tables.
 * @return Number of directory tables. (-1 on error)
 */
int Card::datCount(void) const
{
	if (!isOpen())
		return -1;
	Q_D(const Card);
	return d->dat_info.count;
}

/**
 * Get the active Directory Table index.
 * @return Active Directory Table index. (-1 on error)
 */
int Card::activeDatIdx(void) const
{
	if (!isOpen())
		return -1;
	Q_D(const Card);
	return d->dat_info.active;
}

/**
 * Get the active Directory Table index according to the card header.
 * @return Active Directory Table index, or -1 if all are invalid.
 */
int Card::activeDatHdrIdx(void) const
{
	if (!isOpen())
		return -1;
	Q_D(const Card);
	return d->dat_info.active_hdr;
}

/**
 * Is a Directory Table valid?
 * @param idx Directory Table index.
 * @return True if valid; false if not valid or idx is invalid.
 */
bool Card::isDatValid(int idx) const
{
	if (!isOpen())
		return false;
	Q_D(const Card);
	if (idx < 0 || idx >= d->dat_info.count)
		return false;
	return d->isDatValid(idx);
}

/**
 * Get the number of block tables.
 * @return Number of block tables. (-1 on error)
 */
int Card::batCount(void) const
{
	if (!isOpen())
		return -1;
	Q_D(const Card);
	return d->bat_info.count;
}

/**
 * Get the active Block Table index.
 * @return Active Block Table index. (-1 on error)
 */
int Card::activeBatIdx(void) const
{
	if (!isOpen())
		return -1;
	Q_D(const Card);
	return d->bat_info.active;
}

/**
 * Get the active Block Table index according to the card header.
 * @return Active Block Table index, or -1 if both are invalid.
 */
int Card::activeBatHdrIdx(void) const
{
	if (!isOpen())
		return -1;
	Q_D(const Card);
	return d->bat_info.active_hdr;
}

/**
 * Is a Block Table valid?
 * @param idx Block Table index.
 * @return True if valid; false if not valid or idx is invalid.
 */
bool Card::isBatValid(int idx) const
{
	if (!isOpen())
		return false;
	Q_D(const Card);
	if (idx < 0 || idx >= d->bat_info.count)
		return false;
	return d->isBatValid(idx);
}

/**
 * Is a Free Block count valid?
 * @param idx Block Table index.
 * @return True if valid; false if not valid or idx is invalid.
 */
bool Card::isFreeBlockCountValid(int idx) const
{
	if (!isOpen())
		return false;
	Q_D(const Card);
	if (idx < 0 || idx >= d->bat_info.count)
		return false;
	return d->isFreeBlockCountValid(idx);
}

/** Card I/O **/

/**
 * Read a block.
 * @param buf Buffer to read the block data into.
 * @param siz Size of buffer. (Must be >= blockSize.)
 * @param blockIdx Block index.
 * @return Bytes read on success; negative POSIX error code on error.
 */
int Card::readBlock(void *buf, int siz, uint16_t blockIdx)
{
	Q_D(Card);
	if (!isOpen())
		return EBADF;
	else if (siz < (int)d->blockSize)
		return -EINVAL;
	else if (siz == 0)
		return 0;

	// Read the specified block.
	const qint64 pos = ((qint64)blockIdx * d->blockSize) + d->headerSize;
	if (!d->file->seek(pos))
		return -EIO;	// TODO: Proper error code?
	int ret = (int)d->file->read((char*)buf, d->blockSize);
	return (ret >= 0 ? ret : -EIO);
}

/**
 * Write a block.
 * @param buf Buffer containing the data to write.
 * @param siz Size of buffer. (Must be equal to blockSize.)
 * @param blockIdx Block index.
 * @return Bytes written on success; negative POSIX error code on error.
 */
int Card::writeBlock(const void *buf, int siz, uint16_t blockIdx)
{
	Q_D(Card);
	if (!isOpen())
		return -EBADF;
	else if (siz < (int)d->blockSize)
		return -EINVAL;
	else if (siz == 0)
		return 0;

	// Make sure the card isn't read-only.
	if (d->readOnly)
		return -EROFS;

	// Write the specified block.
	const qint64 pos = ((qint64)blockIdx * d->blockSize) + d->headerSize;
	if (!d->file->seek(pos))
		return -EIO;    // TODO: Proper error code?
	// TODO: Check for errors?
	int ret = (int)d->file->write((char*)buf, d->blockSize);
	return (ret >= 0 ? ret : -EIO);
}

// TODO: Add readBlocks() and writeBlocks() functions?

/** File management **/

/**
 * Get the number of files in the file table.
 * @return Number of files, or negative on error.
 */
int Card::fileCount(void) const
{
	if (!isOpen())
		return -1;
	Q_D(const Card);
	return d->lstFiles.size();
}

/**
 * Is the card empty?
 * @return True if empty; false if not.
 */
bool Card::isEmpty(void) const
{
	if (!isOpen())
		return true;
	Q_D(const Card);
	return d->lstFiles.isEmpty();
}

/**
 * Get a GcnFile object.
 * @param idx File number.
 * @return GcnFile object, or nullptr on error.
 */
File *Card::getFile(int idx)
{
	if (!isOpen())
		return nullptr;
	Q_D(Card);
	if (idx < 0 || idx >= d->lstFiles.size())
		return nullptr;
	return d->lstFiles.at(idx);
}

/**
 * Get File objects that match the given types.
 * @param types Types of files to return (default is FileTypes::All)
 */
QVector<File*> Card::getFiles(FileTypes types)
{
	QVector<File*> ret;
	if (!isOpen())
		return ret;

	Q_D(Card);
	switch (types) {
		case FileTypes::None:
		default:
			// Unhandled value.
			assert(!"Unhandled \"types\" value.");
			break;

		case FileTypes::All:
			// Return all of the files.
			return d->lstFiles;

		case FileTypes::Normal: {
			// Return normal files only.
			ret.reserve(d->lstFiles.size());
			foreach (File *file, d->lstFiles) {
				if (!file->isLostFile()) {
					ret.append(file);
				}
			}
			break;
		}

		case FileTypes::Lost: {
			// Return "lost" files only.
			ret.reserve(d->lstFiles.size());
			foreach (File *file, d->lstFiles) {
				if (file->isLostFile()) {
					ret.append(file);
				}
			}
			break;
		}
	}

	return ret;
}

/**
 * Remove all "lost" files.
 */
void Card::removeLostFiles(void)
{
	Q_D(Card);
	for (int i = d->lstFiles.size() - 1; i >= 0; i--) {
		const File *file = d->lstFiles.at(i);
		if (file->isLostFile()) {
			// This is a "lost" file. Remove it.
			emit filesAboutToBeRemoved(i, i);
			d->lstFiles.remove(i);
			emit filesRemoved();
		}
	}
}

/** Errors **/

/**
 * Have any errors been detected in this Memory Card?
 * @return Error flags.
 */
QFlags<Card::Error> Card::errors(void) const
{
	Q_D(const Card);
	if (!isOpen())
		return QFlags<Card::Error>();
	return d->errors;
}

/**
 * Get the garbage byte information.
 * This function is only valid if
 * Errors contains MCE_HEADER_GARBAGE.
 * @param bad_byte	[out] Bad byte value.
 * @param count		[out] Number of times the byte appeared.
 * @param total		[out] Total number of bytes checked.
 * @return 0 on success; non-zero on error.
 * (If bad bytes weren't detected, this function will fail.)
 */
int Card::garbageInfo(uint8_t *bad_byte, int *count, int *total) const
{
	Q_D(const Card);
	if (!isOpen()) {
		return -EBADF;
	} else if (!(d->errors & MCE_INVALID_HEADER)) {
		// TODO: Better error?
		return -ENOTTY;
	}

	if (bad_byte) {
		*bad_byte = d->garbage.bad_byte;
	}
	if (count) {
		*count = d->garbage.count;
	}
	if (total) {
		*total = d->garbage.total;
	}

	return 0;
}
