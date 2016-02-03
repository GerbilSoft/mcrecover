/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * GcnSearchData.hpp: GCN search data.                                     *
 *                                                                         *
 * Copyright (c) 2013-2016 by David Korth.                                 *
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

#ifndef __MCRECOVER_DB_GCNSEARCHDATA_HPP__
#define __MCRECOVER_DB_GCNSEARCHDATA_HPP__

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

#endif /* __MCRECOVER_SEARCHDATA_HPP__ */
