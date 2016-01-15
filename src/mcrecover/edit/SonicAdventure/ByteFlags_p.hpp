/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * ByteFlags_p.hpp: Generic byte flags base class. (PRIVATE)               *
 * Used for things where a single object has multiple flags                *
 * stored as a byte.                                                       *
 *                                                                         *
 * Copyright (c) 2015 by David Korth.                                      *
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

#ifndef __MCRECOVER_EDIT_SONICADVENTURE_BYTEFLAGS_P_HPP__
#define __MCRECOVER_EDIT_SONICADVENTURE_BYTEFLAGS_P_HPP__

// Qt includes.
#include <QtCore/QHash>
#include <QtCore/QString>
#include <QtCore/QVector>

// Bit flag struct.
// (Works for ByteFlags as well.)
#include "bit_flag.h"

class ByteFlagsPrivate
{
	public:
		/**
		 * Initialize ByteFlagsPrivate.
		 *
		 * Derived ByteFlags classes should have their own private class
		 * that derives from ByteFlagsPrivate.
		 *
		 * @param total_flags Total number of flags the user can edit.
		 * @param byte_flags Byte flag descriptions.
		 * @param count Number of bit_flags entries. (must be >= total_flags)
		 */
		ByteFlagsPrivate(int total_flags, const bit_flag_t *byte_flags, int count);
		virtual ~ByteFlagsPrivate();

	private:
		Q_DISABLE_COPY(ByteFlagsPrivate)

	public:
		// Object descriptions.
		// TODO: Shared copy shared by a specific derived
		// class that's only deleted once all instances
		// of said class are deleted?
		// TODO: An array might be more efficient, even if
		// it wastes some memory...
		QHash<int, QString> objs_desc;

		// Objects, each with 8 flags.
		QVector<uint8_t> objs;
};

#endif /* __MCRECOVER_EDIT_SONICADVENTURE_BYTEFLAGS_P_HPP__ */
