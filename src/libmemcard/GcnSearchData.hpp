/***************************************************************************
 * GameCube Memory Card Recovery Program. [libmemcard]                     *
 * GcnSearchData.hpp: GCN search data.                                     *
 *                                                                         *
 * Copyright (c) 2013-2025 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#ifndef __LIBMEMCARD_GCNSEARCHDATA_HPP__
#define __LIBMEMCARD_GCNSEARCHDATA_HPP__

#include "card.h"
#include "Checksum.hpp"

struct GcnSearchData
{
	card_direntry dirEntry;
	std::vector<uint16_t> fatEntries;
	std::vector<Checksum::ChecksumDef> checksumDefs;
};

#endif /* __LIBMEMCARD_GCNSEARCHDATA_HPP__ */
