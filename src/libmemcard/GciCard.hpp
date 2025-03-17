/***************************************************************************
 * GameCube Memory Card Recovery Program [libmemcard]                      *
 * GciCard.hpp: GameCube GCI single-file class.                            *
 *                                                                         *
 * This is a wrapper class that allows loading of .gci files for editing   *
 * and template creation. Scanning for lost files is not supported.        *
 *                                                                         *
 * Copyright (c) 2012-2025 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#ifndef __LIBMEMCARD_GCICARD_HPP__
#define __LIBMEMCARD_GCICARD_HPP__

#include "Card.hpp"

#include "card.h"
#include "Checksum.hpp"
#include "GcnSearchData.hpp"

// C++ includes.
#include <list>

class GcnFile;

class GciCardPrivate;
class GciCard : public Card
{
	Q_OBJECT
	typedef Card super;

protected:
	explicit GciCard(QObject *parent = 0);

protected:
	Q_DECLARE_PRIVATE(GciCard)
private:
	Q_DISABLE_COPY(GciCard)

public:
	/**
	 * Open an existing GCI file.
	 * @param filename Filename
	 * @param parent Parent object
	 * @return GciCard object, or nullptr on error.
	 */
	static GciCard *open(const QString& filename, QObject *parent);

public:
	/** File system **/

	/**
	 * Set the active Directory Table index.
	 * NOTE: This function reloads the file list, without lost files.
	 * @param idx Active Directory Table index
	 */
	void setActiveDatIdx(int idx) final
	{
		// GCI doesn't have a directory table.
		Q_UNUSED(idx)
	}

	/**
	 * Set the active Block Table index.
	 * NOTE: This function reloads the file list, without lost files.
	 * @param idx Active Block Table index
	 */
	void setActiveBatIdx(int idx) final
	{
		// GCI doesn't have a block table.
		Q_UNUSED(idx)
	}

public:
	/** Card information **/

	/**
	 * Get the product name of this memory card.
	 * This refers to the class in general,
	 * and does not change based on size.
	 * @return Product name
	 */
	QString productName(void) const final;
};

#endif /* __LIBMEMCARD_GCICARD_HPP__ */
