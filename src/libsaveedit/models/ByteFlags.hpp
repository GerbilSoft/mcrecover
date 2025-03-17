/***************************************************************************
 * GameCube Memory Card Recovery Program [libsaveedit]                     *
 * ByteFlags.hpp: Generic byte flags base class.                           *
 * Used for things where a single object has multiple flags                *
 * stored as a byte.                                                       *
 *                                                                         *
 * Copyright (c) 2015-2025 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#pragma once

// Qt includes.
#include <QtCore/QObject>
#include <QtGui/QPixmap>

// C includes.
#include <stdint.h>

// Bit flag struct.
// (Works for ByteFlags as well.)
#include "bit_flag.h"

class ByteFlagsPrivate;
class ByteFlags : public QObject
{
	Q_OBJECT
	typedef QObject super;

	Q_PROPERTY(int pageSize READ pageSize)

	// This class should not be used directly.
	protected:
		/**
		 * Initialize ByteFlags.
		 *
		 * This should be called by subclass constructors with
		 * the appropriate values.
		 *
		 * @param total_flags Total number of flags the user can edit.
		 * @param byte_flags Byte flag descriptions.
		 * @param count Number of bit_flags entries. (must be >= total_flags)
		 * @param parent Parent object.
		 */
		ByteFlags(int total_flags, const bit_flag_t *byte_flags,
			  int count, QObject *parent = 0);

		/**
		 * Initialize ByteFlags.
		 *
		 * This should be called by subclass constructors with
		 * the appropriate values.
		 *
		 * @param total_flags Total number of flags the user can edit.
		 * @param tr_ctx Translation context for byte flag descriptions.
		 * @param byte_flags Byte flag descriptions.
		 * @param count Number of bit_flags entries. (must be >= total_flags)
		 * @param parent Parent object.
		 */
		ByteFlags(int total_flags, const char *tr_ctx,
			  const bit_flag_t *byte_flags, int count, QObject *parent = 0);
	public:
		virtual ~ByteFlags();

	protected:
		ByteFlagsPrivate *d_ptr;
		Q_DECLARE_PRIVATE(ByteFlags)
	private:
		Q_DISABLE_COPY(ByteFlags)

	signals:
		/**
		 * An object's flags have been changed.
		 *
		 * NOTE: If multiple flags are changed at once,
		 * flagsChanged() is emitted instead.
		 *
		 * @param id Object ID.
		 * @param value Byte value.
		 */
		void flagChanged(int id, uint8_t value);

		/**
		 * Multiple objects' flags have been changed.
		 *
		 * NOTE: If a single object's flags are changed,
		 * flagChanged() is emitted instead.
		 *
		 * @param firstID ID of first object whose flags have changed.
		 * @param lastID ID of last object whose flags have changed.
		 */
		void flagsChanged(int firstID, int lastID);

	public:
		/**
		 * Get the total number of objects.
		 * @return Total number of objects.
		 */
		int count(void) const;

		/**
		 * Get an object's description.
		 * @param id Object ID.
		 * @return Description.
		 */
		QString description(int id) const;

		/**
		 * Get an object's flags.
		 * @param id Object ID.
		 * @return Object's flags.
		 */
		uint8_t flag(int id) const;

		/**
		 * Set an object's flags.
		 * @param id Object ID.
		 * @param value New flag value.
		 */
		void setFlag(int id, uint8_t value);

		/**
		 * Get the object flags as an array of bytes.
		 *
		 * If the array doesn't match the size of this ByteFlags:
		 * - Too small: Array will be used for the first sz flags.
		 * - Too big: Array will be used for count() flags.
		 *
		 * TODO: Various byte flag encodings.
		 *
		 * @param data Byte flags.
		 * @param sz Number of bytes in data.
		 * @return Number of byte flags retrieved.
		 */
		int allFlags(uint8_t *data, int sz) const;

		/**
		 * Set the bit flags from an array of bytes.
		 *
		 * If the array doesn't match the size of this ByteFlags:
		 * - Too small: Array will be used for the first sz flags.
		 * - Too big: Array will be used for count() flags.
		 *
		 * TODO: Various byte flag encodings.
		 *
		 * @param data Byte flags.
		 * @param sz Number of bytes in data.
		 * @return Number of byte flags loaded.
		 */
		int setAllFlags(const uint8_t *data, int sz);

		/**
		 * Get a description of the type of object that is represented by the class.
		 * @return Flag type, e.g. "Mission".
		 */
		virtual QString objectType(void) const = 0;

		/**
		 * Get a description of the type of flag represented by a given bit.
		 * @param bit Bit index. (LSB == 0)
		 * @return Flag type, e.g. "Completed". (If bit is unused, empty QString is returned.)
		 */
		virtual QString flagType(int bit) const = 0;

		/**
		 * Get a character icon representing a flag.
		 * TODO: Make this more generic?
		 * @param id Object ID.
		 * @return Character icon.
		 */
		virtual QPixmap icon(int id) const = 0;

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
