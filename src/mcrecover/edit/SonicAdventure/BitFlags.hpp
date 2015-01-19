/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * BitFlags.hpp: Generic bit flags base class.                             *
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

#ifndef __MCRECOVER_EDIT_SONICADVENTURE_BITFLAGS_HPP__
#define __MCRECOVER_EDIT_SONICADVENTURE_BITFLAGS_HPP__

// Qt includes.
// TODO: Should this actually inherit from QObject?
#include <QtCore/QObject>

class BitFlagsPrivate;
class BitFlags : public QObject
{
	Q_OBJECT

	// This class should not be used directly.
	protected:
		BitFlags(BitFlagsPrivate *d, QObject *parent = 0);
	public:
		virtual ~BitFlags();

	private:
		BitFlagsPrivate *d_ptr;
		Q_DECLARE_PRIVATE(BitFlags);

	public:
		/**
		 * Get the total number of flags.
		 * @return Total number of flags.
		 */
		int count(void) const;

		/**
		 * Get a flag's description.
		 * @param event Event ID.
		 * @return Description.
		 */
		QString description(int flag) const;

		/**
		 * Is a flag set?
		 * @param flag Flag ID.
		 * @return True if set; false if not.
		 */
		bool flag(int flag) const;

		/**
		 * Set a flag.
		 * @param event Flag ID.
		 * @param value New flag value.
		 */
		void setFlag(int flag, bool value);
};

#endif /* __MCRECOVER_EDIT_SONICADVENTURE_FLAGS_HPP__ */
