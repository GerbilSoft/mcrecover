/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * SADXMissionFlags.hpp: Sonic Adventure DX - Mission flags.               *
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

#ifndef __MCRECOVER_EDIT_SONICADVENTURE_SADXMISSIONFLAGS_HPP__
#define __MCRECOVER_EDIT_SONICADVENTURE_SADXMISSIONFLAGS_HPP__

#include "../models/ByteFlags.hpp"
class SADXMissionFlagsPrivate;
class SADXMissionFlags : public ByteFlags
{
	// TODO: Should this actually inherit from QObject?
	Q_OBJECT

	public:
		SADXMissionFlags(QObject *parent = 0);
		virtual ~SADXMissionFlags();

	protected:
		SADXMissionFlagsPrivate *d_ptr;
		Q_DECLARE_PRIVATE(SADXMissionFlags)
	private:
		Q_DISABLE_COPY(SADXMissionFlags)

	public:
		/**
		 * Get a description of the type of object that is represented by the class.
		 * @return Flag type, e.g. "Mission".
		 */
		virtual QString objectType(void) const override;

		/**
		 * Get a description of the type of flag represented by a given bit.
		 * @param bit Bit index. (LSB == 0)
		 * @return Flag type, e.g. "Completed". (If bit is unused, empty QString is returned.)
		 */
		virtual QString flagType(int bit) const override;

		/**
		 * Get a character icon representing a flag.
		 * TODO: Make this more generic?
		 * @param id Object ID.
		 * @return Character icon.
		 */
		virtual QPixmap icon(int id) const;
};

#endif /* __MCRECOVER_EDIT_SONICADVENTURE_SADXMISSIONFLAGS_HPP__ */
