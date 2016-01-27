/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * BitFlags_p.hpp: Generic bit flags base class. (PRIVATE)                 *
 *                                                                         *
 * Copyright (c) 2015-2016 by David Korth.                                 *
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

#ifndef __MCRECOVER_EDIT_MODELS_BITFLAGS_P_HPP__
#define __MCRECOVER_EDIT_MODELS_BITFLAGS_P_HPP__

// Qt includes.
#include <QtCore/QHash>
#include <QtCore/QString>
#include <QtCore/QVector>

// Bit flag struct.
#include "bit_flag.h"

class BitFlagsPrivate
{
	public:
		/**
		 * Initialize BitFlagsPrivate.
		 *
		 * Derived BitFlags classes should have their own private class
		 * that derives from BitFlagsPrivate.
		 *
		 * @param total_flags Total number of flags the user can edit.
		 * @param bit_flags Bit flag descriptions.
		 * @param count Number of bit_flags entries. (must be >= total_flags)
		 */
		BitFlagsPrivate(int total_flags, const bit_flag_t *bit_flags, int count);
		virtual ~BitFlagsPrivate();

	private:
		Q_DISABLE_COPY(BitFlagsPrivate)

	public:
		// Flag descriptions.
		// TODO: Shared copy shared by a specific derived
		// class that's only deleted once all instances
		// of said class are deleted?
		// TODO: An array might be more efficient, even if
		// it wastes some memory...
		QHash<int, QString> flags_desc;

		// Flags.
		// NOTE: QVector<bool> does not have bit "optimization".
		QVector<bool> flags;
};

#endif /* __MCRECOVER_EDIT_MODELS_BITFLAGS_P_HPP__ */
