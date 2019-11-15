/***************************************************************************
 * GameCube Memory Card Recovery Program. [libmemcard]                     *
 * GcnSearchData.hpp: GCN search data.                                     *
 *                                                                         *
 * Copyright (c) 2013-2018 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#ifndef __LIBMEMCARD_GCNSEARCHDATA_HPP__
#define __LIBMEMCARD_GCNSEARCHDATA_HPP__

#include "card.h"
#include "Checksum.hpp"

// Qt includes.
#include <QtCore/QVector>

struct GcnSearchData
{
	card_direntry dirEntry;
	QVector<uint16_t> fatEntries;
	QVector<Checksum::ChecksumDef> checksumDefs;
};

#endif /* __LIBMEMCARD_GCNSEARCHDATA_HPP__ */
