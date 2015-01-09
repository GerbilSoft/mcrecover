/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * Card.cpp: Memory Card physical layer. [base class]                      *
 *                                                                         *
 * Copyright (c) 2012-2015 by David Korth.                                 *
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

#include "Card.hpp"
#include "Card_p.hpp"

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

CardPrivate::CardPrivate(Card *q, uint32_t blockSize, int minBlocks, int maxBlocks)
	: q_ptr(q)
	, errors(0)
	, file(nullptr)
	, filesize(0)
	, encoding(Card::ENCODING_CP1252)
	, blockSize(blockSize)
	, minBlocks(minBlocks)
	, maxBlocks(maxBlocks)
	, totalPhysBlocks(0)
	, totalUserBlocks(0)
	, freeBlocks(0)
{
	assert(isPow2(blockSize));
	assert(blockSize > 0);
	assert(minBlocks > 0);
	assert(maxBlocks > minBlocks);
}

CardPrivate::~CardPrivate()
{
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
 * @param mode File open mode.
 * @return 0 on success; non-zero on error. (also check errorString)
 */
int CardPrivate::open(const QString &filename, QIODevice::OpenModeFlag mode)
{
	if (file) {
		// File is already open.
		// TODO: Don't allow this, or clear all variables?
		close();
	}

	if (filename.isEmpty()) {
		// No filename specified.
		// TODO: Translate the error message.
		this->errorString = QLatin1String("No filename specified");
		return -1;
	}

	// Open the file.
	Q_Q(Card);
	QFile *tmp_file = new QFile(filename, q);
	if (!tmp_file->open(mode)) {
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

	// TODO: If formatting the card, skip all of this.

	// Get the filesize.
	this->filesize = file->size();

	// Calculate the size in blocks.
	totalPhysBlocks = (this->filesize / blockSize);

	// Make sure the size isn't out of range.
	if (totalPhysBlocks < minBlocks) {
		// Not enough blocks.
		this->errors |= Card::MCE_SZ_TOO_SMALL;
	} else if (totalPhysBlocks > maxBlocks) {
		// Too many blocks.
		// Only read up to maxBlocks.
		this->errors |= Card::MCE_SZ_TOO_BIG;
		this->filesize = (maxBlocks * blockSize);
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

/** Card **/

/**
 * Create a new Card object.
 * @param d CardPrivate-derived private class.
 * @param parent Parent object.
 */
Card::Card(CardPrivate *d, QObject *parent)
	: QObject(parent)
	, d_ptr(d)
{ }

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
	// TODO: Return an error if not open?
	if (!isOpen())
		return Card::ENCODING_CP1252;
	Q_D(const Card);
	return d->encoding;
}

/**
 * Get a QTextCodec for the specified encoding.
 * @param encoding Encoding.
 * @return QTextCodec. (If unavailable, defaults to cp1252.)
 */
QTextCodec *Card::textCodec(Encoding encoding)
{
	// Static codec initialization.
	// NOTE: Assuming cp1252 always works.
	static QTextCodec *cp1252 = QTextCodec::codecForName("cp1252");
	static QTextCodec *shiftJis = QTextCodec::codecForName("Shift_JIS");

	switch (encoding) {
		case ENCODING_CP1252:
		default:
			return cp1252;

		case ENCODING_SHIFTJIS:
			return (shiftJis ? shiftJis : cp1252);
	}
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
 * Get a QTextCodec for this memory card.
 * @return QTextCodec.
 */
QTextCodec *Card::textCodec(void) const
{
	if (!isOpen())
		return nullptr;
	Q_D(const Card);
	return textCodec(d->encoding);
}

/** Card I/O **/

/**
 * Read a block.
 * @param buf Buffer to read the block data into.
 * @param siz Size of buffer. (Must be >= blockSize.)
 * @param blockIdx Block index.
 * @return Bytes read on success; negative on error.
 */
int Card::readBlock(void *buf, int siz, uint16_t blockIdx)
{
	if (!isOpen())
		return -1;
	if (siz < blockSize())
		return -2;

	// Read the specified block.
	Q_D(Card);
	d->file->seek((int)blockIdx * blockSize());
	return (int)d->file->read((char*)buf, blockSize());
}

// TODO: Add a readBlocks() function?

// TODO: Add basic file functions, after creating a File class.

/** Errors **/

/**
 * Have any errors been detected in this Memory Card?
 * @return Error flags.
 */
QFlags<Card::Error> Card::errors(void) const
{
	Q_D(const Card);
	if (!isOpen())
		return 0;
	return d->errors;
}
