/***************************************************************************
 * GameCube Memory Card Recovery Program [libsaveedit]                     *
 * SADXMissionFlags.hpp: Sonic Adventure DX - Mission flags.               *
 *                                                                         *
 * Copyright (c) 2015-2018 by David Korth.                                 *
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

#ifndef __LIBSAVEEDIT_SONICADVENTURE_SADXMISSIONFLAGS_HPP__
#define __LIBSAVEEDIT_SONICADVENTURE_SADXMISSIONFLAGS_HPP__

#include "../models/ByteFlags.hpp"
class SADXMissionFlagsPrivate;
class SADXMissionFlags : public ByteFlags
{
	// TODO: Should this actually inherit from QObject?
	Q_OBJECT

	public:
		explicit SADXMissionFlags(QObject *parent = 0);
		virtual ~SADXMissionFlags();

	private:
		typedef ByteFlags super;
		SADXMissionFlagsPrivate *d_ptr;
		Q_DECLARE_PRIVATE(SADXMissionFlags)
		Q_DISABLE_COPY(SADXMissionFlags)

	public:
		/**
		 * Get a description of the type of object that is represented by the class.
		 * @return Flag type, e.g. "Mission".
		 */
		QString objectType(void) const final;

		/**
		 * Get a description of the type of flag represented by a given bit.
		 * @param bit Bit index. (LSB == 0)
		 * @return Flag type, e.g. "Completed". (If bit is unused, empty QString is returned.)
		 */
		QString flagType(int bit) const final;

		/**
		 * Get a character icon representing a flag.
		 * TODO: Make this more generic?
		 * @param id Object ID.
		 * @return Character icon.
		 */
		QPixmap icon(int id) const final;
};

#endif /* __LIBSAVEEDIT_SONICADVENTURE_SADXMISSIONFLAGS_HPP__ */
