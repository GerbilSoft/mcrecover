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

	protected:
		BitFlagsPrivate *d_ptr;
		Q_DECLARE_PRIVATE(BitFlags)
	private:
		Q_DISABLE_COPY(BitFlags)

	signals:
		/**
		 * A flag has been changed.
		 *
		 * NOTE: If multiple flags are changed at once,
		 * flagsChanged() is emitted instead.
		 *
		 * @param flag Flag ID.
		 * @param value Flag value.
		 */
		void flagChanged(int flag, bool value);

		/**
		 * Multiple flags have been changed.
		 *
		 * NOTE: If a single flag is changed,
		 * flagChanged() is emitted instead.
		 *
		 * @param firstFlag First flag that has changed.
		 * @param lastFlag Last flag that has changed.
		 */
		void flagsChanged(int firstFlag, int lastFlag);

	public:
		/**
		 * Get the total number of flags.
		 * @return Total number of flags.
		 */
		int count(void) const;

		/**
		 * Get a flag's description.
		 * @param flag Flag ID.
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

		/**
		 * Load an array of bit flags.
		 *
		 * If the array doesn't match the size of this BitFlags:
		 * - Too small: Array will be used for the first sz*8 flags.
		 * - Too big: Array will be used for count()*8 flags.
		 *
		 * TODO: Various bit flag encodings.
		 *
		 * @param data Bit flags.
		 * @param sz Number of bytes in data. (BYTES, not bits.)
		 * @return Number of bit flags loaded.
		 */
		int setAllFlags(const uint8_t *data, int sz);

		/**
		 * Get a description of the type of flag that is represented by the class.
		 * @return Flag type, e.g. "Event".
		 */
		virtual QString flagType(void) const = 0;
};

#endif /* __MCRECOVER_EDIT_SONICADVENTURE_FLAGS_HPP__ */
