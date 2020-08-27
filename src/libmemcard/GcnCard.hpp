/***************************************************************************
 * GameCube Memory Card Recovery Program [libmemcard]                      *
 * GcnCard.hpp: GameCube memory card class.                                *
 *                                                                         *
 * Copyright (c) 2012-2018 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#ifndef __LIBMEMCARD_GCNCARD_HPP__
#define __LIBMEMCARD_GCNCARD_HPP__

#include "Card.hpp"

#include "card.h"
#include "Checksum.hpp"
#include "GcnSearchData.hpp"

// C++ includes.
#include <list>

class GcnFile;

class GcnCardPrivate;
class GcnCard : public Card
{
	Q_OBJECT
	typedef Card super;

	Q_PROPERTY(int encoding READ encoding)

	// TODO: Register Checksum::ChecksumValue metatype?
	//Q_PROPERTY(Checksum::ChecksumValue headerChecksumValue READ headerChecksumValue)

	protected:
		explicit GcnCard(QObject *parent = 0);
	public:
		virtual ~GcnCard();

	protected:
		Q_DECLARE_PRIVATE(GcnCard)
	private:
		Q_DISABLE_COPY(GcnCard)

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

	public:
		/** File system **/

		/**
		 * Set the active Directory Table index.
		 * NOTE: This function reloads the file list, without lost files.
		 * @param idx Active Directory Table index.
		 */
		void setActiveDatIdx(int idx) final;

		/**
		 * Set the active Block Table index.
		 * NOTE: This function reloads the file list, without lost files.
		 * @param idx Active Block Table index.
		 */
		void setActiveBatIdx(int idx) final;

	public:
		/** Card information **/

		/**
		 * Get the product name of this memory card.
		 * This refers to the class in general,
		 * and does not change based on size.
		 * @return Product name.
		 */
		QString productName(void) const final;

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
		 * @return GcnFile added to the GcnCard, or nullptr on error.
		 */
		GcnFile *addLostFile(const card_direntry *dirEntry);

		/**
		 * Add a "lost" file.
		 * @param dirEntry Directory entry.
		 * @param fatEntries FAT entries.
		 * @return GcnFile added to the GcnCard, or nullptr on error.
		 */
		GcnFile *addLostFile(const card_direntry *dirEntry, const QVector<uint16_t> &fatEntries);

		/**
		 * Add "lost" files.
		 * @param filesFoundList List of SearchData.
		 * @return List of GcnFiles added to the GcnCard, or empty list on error.
		 */
		QList<GcnFile*> addLostFiles(const std::list<GcnSearchData> &filesFoundList);

		/**
		 * Get the header checksum value.
		 * NOTE: Header checksum is always AddInvDual16.
		 * @return Header checksum value.
		 */
		Checksum::ChecksumValue headerChecksumValue(void) const;
};

#endif /* __LIBMEMCARD_GCNCARD_HPP__ */
