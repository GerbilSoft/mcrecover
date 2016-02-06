/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * Card.hpp: Memory Card physical layer. [base class]                      *
 *                                                                         *
 * Copyright (c) 2012-2016 by David Korth.                                 *
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
#include <QtGui/QColor>

// Date/Time
#include "GcnDateTime.hpp"

class File;

class CardPrivate;
class Card : public QObject
{
	Q_OBJECT
	typedef QObject super;

	Q_ENUMS(Encoding)
	Q_FLAGS(Error Errors)

	Q_PROPERTY(bool open READ isOpen)
	Q_PROPERTY(QString errorString READ errorString)

	// Card size.
	Q_PROPERTY(int blockSize READ blockSize)
	Q_PROPERTY(int totalPhysBlocks READ totalPhysBlocks)
	Q_PROPERTY(int totalUserBlocks READ totalUserBlocks)
	Q_PROPERTY(int freeBlocks READ freeBlocks)

	// Card information.
	Q_PROPERTY(QString productName READ productName)
	Q_PROPERTY(QString filename READ filename)
	Q_PROPERTY(int filesize READ filesize)
	Q_PROPERTY(Encoding encoding READ encoding)
	Q_PROPERTY(QColor color READ color NOTIFY colorChanged)
	Q_PROPERTY(GcnDateTime formatTime READ formatTime)

	// File system.
	// TODO: Notify signals for activeDatIdx / activeBatIdx?
	Q_PROPERTY(int datCount READ datCount)
	Q_PROPERTY(int activeDatIdx READ activeDatIdx WRITE setActiveDatIdx /*NOTIFY activeDatIdxChanged*/)
	Q_PROPERTY(int activeDatHdrIdx READ activeDatHdrIdx)
	Q_PROPERTY(int batCount READ batCount)
	Q_PROPERTY(int activeBatIdx READ activeBatIdx WRITE setActiveBatIdx /*NOTIFY activeBatIdxChanged*/)
	Q_PROPERTY(int activeBatHdrIdx READ activeBatHdrIdx)

	// File management.
	Q_PROPERTY(int fileCount READ fileCount)
	Q_PROPERTY(bool empty READ isEmpty)

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

		/**
		 * Color has changed.
		 * @param color New color.
		 */
		void colorChanged(const QColor &color);

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

	public:
		/** Card information **/

		/**
		 * Get the product name of this memory card.
		 * This refers to the class in general,
		 * and does not change based on size.
		 * @return Product name.
		 */
		virtual QString productName(void) const = 0;

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
		 * Get the minimum number of blocks allowed for this card.
		 * @return Minimum number of blocks. (Negative on error)
		 */
		int minBlocks(void) const;

		/**
		 * Get the maximum number of blocks allowed for this card.
		 * @return Maximum number of blocks. (Negative on error)
		 */
		int maxBlocks(void) const;

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
			ENCODING_UNKNOWN = 0,	// Unknown
			ENCODING_CP1252,	// cp1252
			ENCODING_SHIFTJIS,	// Shift-JIS

			ENCODING_MAX
		};

		/**
		 * Get the text encoding used for filenames and descriptions.
		 * Indicated by the card header.
		 * If not present, returns ENCODING_UNKNOWN.
		 * @return Text encoding.
		 */
		Encoding encoding(void) const;

		/**
		 * Get the card's color.
		 * @return Color. (If not available, an invalid QColor is returned.)
		 */
		QColor color(void) const;

		/**
		 * Get the card's format time.
		 * @return Format time. (If not available, will return Unix epoch.)
		 */
		GcnDateTime formatTime(void) const;

		/**
		 * Get the card's icon.
		 * @return Icon, or default system icon or null QPixmap if unavailable.
		 */
		QPixmap icon(void) const;

	public:
		/** File system **/
		// TODO: Negative POSIX code on error, or just -1?

		/**
		 * Get the number of directory tables.
		 * @return Number of directory tables. (-1 on error)
		 */
		int datCount(void) const;

		/**
		 * Get the active Directory Table index.
		 * @return Active Directory Table index. (-1 on error)
		 */
		int activeDatIdx(void) const;

		/**
		 * Set the active Directory Table index.
		 * NOTE: This function reloads the file list, without lost files.
		 * @param idx Active Directory Table index.
		 */
		virtual void setActiveDatIdx(int idx) = 0;

		/**
		 * Get the active Directory Table index according to the card header.
		 * @return Active Directory Table index, or -1 if both are invalid.
		 */
		int activeDatHdrIdx(void) const;

		/**
		 * Is a Directory Table valid?
		 * @param idx Directory Table index.
		 * @return True if valid; false if not valid or if idx is invalid.
		 */
		bool isDatValid(int idx) const;

		/**
		 * Get the number of block tables.
		 * @return Number of block tables. (-1 on error)
		 */
		int batCount(void) const;

		/**
		 * Get the active Block Table index.
		 * @return Active Block Table index. (-1 on error)
		 */
		int activeBatIdx(void) const;

		/**
		 * Set the active Block Table index.
		 * NOTE: This function reloads the file list, without lost files.
		 * @param idx Active Block Table index, or -1 if both are invalid.
		 */
		virtual void setActiveBatIdx(int idx) = 0;

		/**
		 * Get the active Block Table index according to the card header.
		 * @return Active Block Table index, or -1 if both are invalid.
		 */
		int activeBatHdrIdx(void) const;

		/**
		 * Is a Block Table valid?
		 * @param idx Block Table index.
		 * @return True if valid; false if not valid or idx is invalid.
		 */
		bool isBatValid(int idx) const;

	public:
		/** Card I/O **/

		/**
		 * Read a block.
		 * @param buf Buffer to read the block data into.
		 * @param siz Size of buffer. (Must be >= blockSize.)
		 * @param blockIdx Block index.
		 * @return Bytes read on success; negative on error.
		 */
		int readBlock(void *buf, int siz, uint16_t blockIdx);

		/** File management **/
	signals:
		/**
		 * Files are about to be added to the GcnCard.
		 * @param start First file index.
		 * @param end Last file index.
		 */
		void filesAboutToBeInserted(int start, int end);

		/**
		 * Files have been added to the GcnCard.
		 */
		void filesInserted(void);

		/**
		 * Files are about to be removed from the GcnCard.
		 * @param start First file index.
		 * @param end Last file index.
		 */
		void filesAboutToBeRemoved(int start, int end);

		/**
		 * Files have been removed from the GcnCard.
		 */
		void filesRemoved(void);

	public:
		/**
		 * Get the number of files in the file table.
		 * @return Number of files, or negative on error.
		 */
		int fileCount(void) const;

		/**
		 * Is the card empty?
		 * @return True if empty; false if not.
		 */
		bool isEmpty(void) const;

		/**
		 * Get a File object.
		 * @param idx File number.
		 * @return File object, or nullptr on error.
		 */
		File *getFile(int idx);

		/**
		 * Remove all "lost" files.
		 */
		void removeLostFiles(void);

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
			// Short read occurrd. (I/O error)
			MCE_SHORT_READ		= 0x08,

			// Header checksum is invalid.
			// NOTE: What about "header is invalid 
			MCE_INVALID_HEADER	= 0x10,

			// Header is invalid, and is mostly
			// the same byte, indicating the dump
			// may have failed.
			MCE_HEADER_GARBAGE	= 0x20,

			// All DATs are invalid.
			MCE_INVALID_DATS	= 0x100,
			// All BATs are invalid.
			MCE_INVALID_BATS	= 0x200,

			// TODO: File open errors?
		};
		Q_DECLARE_FLAGS(Errors, Error)

		/**
		 * Have any errors been detected in this Memory Card?
		 * @return Error flags.
		 */
		QFlags<Error> errors(void) const;

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
		int garbageInfo(uint8_t *bad_byte, int *count, int *total) const;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(Card::Errors);

#endif /* __MCRECOVER_CARD_CARD_HPP__ */
