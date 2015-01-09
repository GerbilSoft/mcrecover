/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * Card.hpp: Memory Card physical layer. [base class]                      *
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

#ifndef __MCRECOVER_CARD_CARD_HPP__
#define __MCRECOVER_CARD_CARD_HPP__

// C includes.
#include <stdint.h>

// Qt includes and classes.
#include <QtCore/QObject>
#include <QtCore/QTextCodec>

// Card private class.
class CardPrivate;

class Card : public QObject
{
	Q_OBJECT

	Q_ENUMS(Encoding)
	Q_FLAGS(Error Errors)

	Q_PROPERTY(bool open READ isOpen)
	Q_PROPERTY(QString errorString READ errorString)
	Q_PROPERTY(QString filename READ filename)
	Q_PROPERTY(int filesize READ filesize)
	Q_PROPERTY(int encoding READ encoding)

	// Card size.
	Q_PROPERTY(int blockSize READ blockSize)
	Q_PROPERTY(int totalPhysBlocks READ totalPhysBlocks)
	Q_PROPERTY(int totalUserBlocks READ totalUserBlocks)
	Q_PROPERTY(int freeBlocks READ freeBlocks)

	protected:
		Card(CardPrivate *d, QObject *parent = 0);
	public:
		virtual ~Card();

	// TODO: Add an open() function that autodetects the card type?

	protected:
		CardPrivate *const d_ptr;
		Q_DECLARE_PRIVATE(Card)
	private:
		Q_DISABLE_COPY(Card)

	// TODO: Add basic file handling to the base class.
	// Requires "CardFile" class.
	signals:
		/**
		 * Block count has changed.
		 * @param totalPhysBlocks Total physical blocks.
		 * @param totalUserBlocks Total user-accessible blocks.
		 * @param freeBlocks Number of free blocks.
		 */
		void blockCountChanged(int totalPhysBlocks, int totalUserBlocks, int freeBlocks);

	public:
		/**
		 * Check if the memory card is open.
		 * @return True if open; false if not.
		 */
		bool isOpen(void) const;

		/**
		 * Get the last error string.
		 * Usually used for open() errors.
		 * TODO: Change to error code constants for translation?
		 * @return Error string.
		 */
		QString errorString(void) const;

		/** Card information **/

		/**
		 * Get the memory card filename.
		 * @return Memory card filename, or empty string if not open.
		 */
		QString filename(void) const;

		/**
		 * Get the size of the memory card image, in bytes.
		 * This is the full size of the memory card image.
		 * @return Size of the memory card image, in bytes. (Negative on error)
		 */
		quint64 filesize(void) const;

		/**
		 * Get the memory card block size, in bytes.
		 * @return Memory card block size, in bytes. (Negative on error)
		 */
		int blockSize(void) const;

		/**
		 * Get the total number of physical blocks.
		 * This includes reserved blocks.
		 * @return Total number of physical blocks. (Negative on error)
		 */
		int totalPhysBlocks(void) const;

		/**
		 * Get the total number of user-accessible blocks.
		 * This does NOT include reserved blocks.
		 * @return Total number of user-accessible blocks. (Negative on error)
		 */
		int totalUserBlocks(void) const;

		/**
		 * Get the number of free blocks.
		 * @return Free blocks. (Negative on error)
		 */
		int freeBlocks(void) const;

		/**
		 * Text encoding enumeration.
		 */
		enum Encoding {
			ENCODING_CP1252 = 0,
			ENCODING_SHIFTJIS = 1,
		};

		/**
		 * Get the text encoding used for filenames and descriptions.
		 * @return Text encoding.
		 */
		Encoding encoding(void) const;

		/**
		 * Get a QTextCodec for the specified encoding.
		 * @param encoding Encoding.
		 * @return QTextCodec. (If unavailable, defaults to cp1252.)
		 */
		static QTextCodec *textCodec(Encoding encoding);

		/**
		 * Get a QTextCodec for this memory card.
		 * @return QTextCodec.
		 */
		QTextCodec *textCodec(void) const;

		/** Card I/O **/

		/**
		 * Read a block.
		 * @param buf Buffer to read the block data into.
		 * @param siz Size of buffer. (Must be >= blockSize.)
		 * @param blockIdx Block index.
		 * @return Bytes read on success; negative on error.
		 */
		int readBlock(void *buf, int siz, uint16_t blockIdx);

		/** Errors **/

		/**
		 * Memory card errors.
		 */
		enum Error {
			// Errors are ordered in order of severity.

			// Memory card is too small. (GCN: 512 KB min)
			MCE_SZ_TOO_SMALL	= 0x01,
			// Memory card is too big. (GCN: 16 MB max)
			MCE_SZ_TOO_BIG		= 0x02,
			// Memory card size is not a power of two.
			MCE_SZ_NON_POW2		= 0x04,

			// Header checksum is invalid.
			MCE_INVALID_HEADER	= 0x10,

			// GCN-specific errors.

			// Both DATs are invalid.
			MCE_INVALID_DATS	= 0x20,
			// Bot BATs are invalid.
			MCE_INVALID_BATS	= 0x40,

			// TODO: File open errors?
		};
		Q_DECLARE_FLAGS(Errors, Error)

		/**
		 * Have any errors been detected in this Memory Card?
		 * @return Error flags.
		 */
		QFlags<Error> errors(void) const;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(Card::Errors);

#endif /* __MCRECOVER_CARD_CARD_HPP__ */
