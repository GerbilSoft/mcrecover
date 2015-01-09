/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * GcnCard.hpp: GameCube memory card class.                                *
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

#ifndef __MCRECOVER_CARD_GCNCARD_HPP__
#define __MCRECOVER_CARD_GCNCARD_HPP__

#include "Card.hpp"

// Qt includes and classes.
#include <QtCore/QLinkedList>
class QTextCodec;

#include "card.h"
#include "Checksum.hpp"
#include "SearchData.hpp"

class MemCardFile;

class GcnCardPrivate;
class GcnCard : public Card
{
	Q_OBJECT

	Q_PROPERTY(int encoding READ encoding)
	Q_PROPERTY(int numFiles READ numFiles)
	Q_PROPERTY(bool empty READ isEmpty)

	// TODO: Register Checksum::ChecksumValue metatype?
	//Q_PROPERTY(Checksum::ChecksumValue headerChecksumValue READ headerChecksumValue)
	Q_PROPERTY(int activeDatIdx READ activeDatIdx WRITE setActiveDatIdx)
	Q_PROPERTY(int activeDatHdrIdx READ activeDatHdrIdx)
	Q_PROPERTY(int activeBatIdx READ activeBatIdx WRITE setActiveBatIdx)
	Q_PROPERTY(int activeBatHdrIdx READ activeBatHdrIdx)

	protected:
		GcnCard(QObject *parent = 0);
	public:
		~GcnCard();

	protected:
		Q_DECLARE_PRIVATE(GcnCard)

	public:
		/**
		 * Open an existing Memory Card image.
		 * @param filename Filename.
		 * @param parent Parent object.
		 * @return GcnCard object, or nullptr on error.
		 */
		static GcnCard *open(const QString& filename, QObject *parent);

		/**
		 * Format a new Memory Card image.
		 * @param filename Filename.
		 * @param parent Parent object.
		 * @return GcnCard object, or nullptr on error.
		 */
		static GcnCard *format(const QString& filename, QObject *parent);

	private:
		Q_DISABLE_COPY(GcnCard)

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
		 * Get the text encoding for a given region.
		 * @param region Region code. (If 0, use the memory card's encoding.)
		 * @return Text encoding.
		 */
		Encoding encodingForRegion(char region) const;

		/**
		 * Get the QTextCodec for a given region.
		 * @param region Region code. (If 0, use the memory card's encoding.)
		 * @return QTextCodec.
		 */
		QTextCodec *textCodec(char region = 0) const;

		/**
		 * Get the number of files in the file table.
		 * @return Number of files, or negative on error.
		 */
		int numFiles(void) const;

		/**
		 * Is the card empty?
		 * @return True if empty; false if not.
		 */
		bool isEmpty(void) const;

		/**
		 * Get a MemCardFile object.
		 * @param idx File number.
		 * @return MemCardFile object, or nullptr on error.
		 */
		MemCardFile *getFile(int idx);

		/**
		 * Remove all "lost" files.
		 */
		void removeLostFiles(void);

		/**
		 * Get the used block map.
		 * NOTE: This is only valid for regular files, not "lost" files.
		 * @return Used block map.
		 */
		QVector<uint8_t> usedBlockMap(void);

		/**
		 * Add a "lost" file.
		 * NOTE: This is a debugging version.
		 * Add more comprehensive versions with a block map specification.
		 * @return MemCardFile added to the GcnCard, or nullptr on error.
		 */
		MemCardFile *addLostFile(const card_direntry *dirEntry);

		/**
		 * Add a "lost" file.
		 * @param dirEntry Directory entry.
		 * @param fatEntries FAT entries.
		 * @return MemCardFile added to the GcnCard, or nullptr on error.
		 */
		MemCardFile *addLostFile(const card_direntry *dirEntry, const QVector<uint16_t> &fatEntries);

		/**
		 * Add "lost" files.
		 * @param filesFoundList List of SearchData.
		 * @return List of MemCardFiles added to the GcnCard, or empty list on error.
		 */
		QList<MemCardFile*> addLostFiles(const QLinkedList<SearchData> &filesFoundList);

		/**
		 * Get the header checksum value.
		 * NOTE: Header checksum is always AddInvDual16.
		 * @return Header checksum value.
		 */
		Checksum::ChecksumValue headerChecksumValue(void) const;

		/**
		 * Get the active Directory Table index.
		 * @return Active Directory Table index. (0 or 1)
		 */
		int activeDatIdx(void) const;

		/**
		 * Set the active Directory Table index.
		 * NOTE: This function reloads the file list, without lost files.
		 * @param idx Active Directory Table index. (0 or 1)
		 */
		void setActiveDatIdx(int idx);

		/**
		 * Get the active Directory Table index according to the card header.
		 * @return Active Directory Table index (0 or 1), or -1 if both are invalid.
		 */
		int activeDatHdrIdx(void) const;

		/**
		 * Is a Directory Table valid?
		 * @param idx Directory Table index. (0 or 1)
		 * @return True if valid; false if not valid or idx is invalid.
		 */
		bool isDatValid(int idx) const;

		/**
		 * Get the active Block Table index.
		 * @return Active Block Table index. (0 or 1)
		 */
		int activeBatIdx(void) const;

		/**
		 * Set the active Block Table index.
		 * NOTE: This function reloads the file list, without lost files.
		 * @param idx Active Block Table index. (0 or 1)
		 */
		void setActiveBatIdx(int idx);

		/**
		 * Get the active Block Table index according to the card header.
		 * @return Active Block Table index (0 or 1), or -1 if both are invalid.
		 */
		int activeBatHdrIdx(void) const;

		/**
		 * Is a Block Table valid?
		 * @param idx Block Table index. (0 or 1)
		 * @return True if valid; false if not valid or idx is invalid.
		 */
		bool isBatValid(int idx) const;
};

#endif /* __MCRECOVER_CARD_GCNCARD_HPP__ */
