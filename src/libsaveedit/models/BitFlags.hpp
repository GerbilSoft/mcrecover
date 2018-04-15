/***************************************************************************
 * GameCube Memory Card Recovery Program [libsaveedit]                     *
 * BitFlags.hpp: Generic bit flags base class.                             *
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

#ifndef __LIBSAVEEDIT_MODELS_BITFLAGS_HPP__
#define __LIBSAVEEDIT_MODELS_BITFLAGS_HPP__

// Qt includes.
#include <QtCore/QObject>

// Bit flag struct.
// (Works for ByteFlags as well.)
#include "bit_flag.h"

class BitFlagsPrivate;
class BitFlags : public QObject
{
	Q_OBJECT
	typedef QObject super;

	Q_PROPERTY(int pageSize READ pageSize)

	// This class should not be used directly.
	protected:
		/**
		 * Initialize BitFlags.
		 *
		 * This should be called by subclass constructors with
		 * the appropriate values.
		 *
		 * @param total_flags Total number of flags the user can edit.
		 * @param bit_flags Bit flag descriptions.
		 * @param count Number of bit_flags entries. (must be >= total_flags)
		 * @param parent Parent object.
		 */
		BitFlags(int total_flags, const bit_flag_t *bit_flags,
			 int count, QObject *parent = 0);

		/**
		 * Initialize BitFlags.
		 *
		 * This should be called by subclass constructors with
		 * the appropriate values.
		 *
		 * @param total_flags Total number of flags the user can edit.
		 * @param tr_ctx Translation context for bit flag descriptions.
		 * @param bit_flags Bit flag descriptions.
		 * @param count Number of bit_flags entries. (must be >= total_flags)
		 * @param parent Parent object.
		 */
		BitFlags(int total_flags, const char *tr_ctx,
			 const bit_flag_t *bit_flags, int count, QObject *parent = 0);
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
		 * Get the bit flags as an array of bitfield data.
		 *
		 * If the array doesn't match the size of this BitFlags:
		 * - Too small: Array will be used for the first sz*8 flags.
		 * - Too big: Array will be used for count()*8 flags.
		 *
		 * TODO: Various bit flag encodings.
		 *
		 * @param data Bit flags.
		 * @param sz Number of bytes in data. (BYTES, not bits.)
		 * @return Number of bit flags retrieved.
		 */
		int allFlags(uint8_t *data, int sz) const;

		/**
		 * Set the bit flags from an array of bitfield data.
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

		/**
		 * Get the desired page size for the BitFlagsModel.
		 * @return Page size.
		 */
		virtual int pageSize(void) const;

		/**
		 * Get the name for a given page of data.
		 *
		 * If pagination is enabled (pageSize > 0), this function is
		 * used to determine the text for the corresponding tab.
		 *
		 * @param page Page number.
		 * @return Page name.
		 */
		virtual QString pageName(int page) const;
};

#endif /* __LIBSAVEEDIT_MODELS_BITFLAGS_HPP__ */
