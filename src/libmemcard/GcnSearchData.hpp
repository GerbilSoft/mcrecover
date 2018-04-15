/***************************************************************************
 * GameCube Memory Card Recovery Program. [libmemcard]                     *
 * GcnSearchData.hpp: GCN search data.                                     *
 *                                                                         *
 * Copyright (c) 2013-2018 by David Korth.                                 *
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
 * You should have received a copy of the GNU General Public License       *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.   *
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
