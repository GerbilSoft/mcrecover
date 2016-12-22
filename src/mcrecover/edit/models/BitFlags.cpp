/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * BitFlags.cpp: Generic bit flags base class.                             *
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

#include "BitFlags.hpp"

// Qt includes.
#include <QtCore/QCoreApplication>
#include <QtCore/QHash>
#include <QtCore/QString>
#include <QtCore/QVector>

// C includes. (C++ namespace)
#include <cassert>

// TODO: Put this in a common header file somewhere.
#define NUM_ELEMENTS(x) ((int)(sizeof(x) / sizeof(x[0])))

/** BitFlagsPrivate **/

class BitFlagsPrivate
{
	public:
		/**
		 * Initialize BitFlagsPrivate.
		 * @param total_flags Total number of flags the user can edit.
		 * @param tr_ctx Translation context for bit flag descriptions.
		 * @param bit_flags Bit flag descriptions.
		 * @param count Number of bit_flags entries. (must be >= total_flags)
		 */
		BitFlagsPrivate(int total_flags, const char *tr_ctx,
				const bit_flag_t *bit_flags, int count);

	private:
		Q_DISABLE_COPY(BitFlagsPrivate)

	public:
		// Flag descriptions.
		// TODO: Shared copy shared by a specific derived
		// class that's only deleted once all instances
		// of said class are deleted?
		QVector<const char*> flags_desc;

		// Flags.
		// NOTE: QVector<bool> does not have bit "optimization".
		QVector<bool> flags;

		// Translation context for bit flags.
		const char *tr_ctx;
};

/**
 * Initialize BitFlagsPrivate.
 * @param total_flags Total number of flags the user can edit.
 * @param tr_ctx Translation context for bit flag descriptions.
 * @param bit_flags Bit flag descriptions.
 * @param count Number of bit_flags entries. (must be >= total_flags)
 */
BitFlagsPrivate::BitFlagsPrivate(int total_flags, const char *tr_ctx,
				 const bit_flag_t *bit_flags, int count)
	: tr_ctx(tr_ctx)
{
	// This is initialized by a derived private class.
	assert(total_flags > 0);
	assert(total_flags >= count);
	assert(count >= 0);

	// Initialize flags.
	// QVector automatically initializes the new elements to false.
	flags.resize(total_flags);

	// Initialize flags_desc.
	// TODO: Once per derived class, rather than once per instance?
	flags_desc.clear();
	flags_desc.resize(total_flags);
	for (int i = 0; i < count; i++, bit_flags++) {
		if (bit_flags->event < 0 || !bit_flags->description) {
			// End of list.
			// NOTE: count should have been set correctly...
			break;
		}

		flags_desc[bit_flags->event] = bit_flags->description;
	}
}

/** BitFlags **/

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
BitFlags::BitFlags(int total_flags, const bit_flag_t *bit_flags,
		   int count, QObject *parent)
	: super(parent)
	, d_ptr(new BitFlagsPrivate(total_flags, nullptr, bit_flags, count))
{ }

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
BitFlags::BitFlags(int total_flags, const char *tr_ctx,
		   const bit_flag_t *bit_flags, int count, QObject *parent)
	: super(parent)
	, d_ptr(new BitFlagsPrivate(total_flags, tr_ctx, bit_flags, count))
{ }

BitFlags::~BitFlags()
{
	delete d_ptr;
}

/**
 * Get the total number of flags.
 * @return Total number of flags.
 */
int BitFlags::count(void) const
{
	Q_D(const BitFlags);
	return d->flags.size();
}

/**
 * Get a flag's description.
 * @param flag Flag ID.
 * @return Description.
 */
QString BitFlags::description(int flag) const
{
	// TODO: Translate using the subclass?
	if (flag < 0 || flag >= count())
		return tr("Invalid flag ID");

	Q_D(const BitFlags);
	const char *desc = d->flags_desc[flag];
	if (!desc) {
		// No flag description is available.
		return tr("Unknown");
	}

	if (d->tr_ctx) {
		// Translation context is available.
		return QCoreApplication::translate(d->tr_ctx, desc);
	}

	// Translation context is not available.
	// NOTE: This may end up being slower and/or
	// using more memory due to lack of implicit
	// QString sharing.
	return QLatin1String(desc);
}

/**
 * Is a flag set?
 * @param flag Flag ID.
 * @return True if set; false if not.
 */
bool BitFlags::flag(int flag) const
{
	if (flag < 0 || flag >= count())
		return false;

	Q_D(const BitFlags);
	return d->flags.at(flag);
}

/**
 * Set a flag.
 * @param flag Flag ID.
 * @param value New flag value.
 */
void BitFlags::setFlag(int flag, bool value)
{
	if (flag < 0 || flag >= count())
		return;

	Q_D(BitFlags);
	d->flags[flag] = value;
	emit flagChanged(flag, value);
}

/**
 * Get all of the bit flags.
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
int BitFlags::allFlags(uint8_t *data, int sz) const
{
	Q_D(const BitFlags);
	assert(sz > 0);
	if (sz <= 0)
		return 0;

	// Convert to bits.
	int bits = sz * 8;
	if (bits > d->flags.count())
		bits = d->flags.count();

	// TODO: Verify that this is correct.
	const bool *flagBool = d->flags.constData();
	for (int i = 0; i < bits; i++, flagBool++) {
		if (i % 8 == 0) {
			// New byte.
			// TODO: Optimize this?
			if (i > 0)
				data++;
			*data = 0;
		}

		// Get this flag.
		*data <<= 1;
		*data |= !!(*flagBool);
	}

	return bits;
}

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
 * @return Number of bit flags set.
 */
int BitFlags::setAllFlags(const uint8_t *data, int sz)
{
	Q_D(BitFlags);
	assert(sz > 0);
	if (sz <= 0)
		return 0;

	// Convert to bits.
	int bits = sz * 8;
	if (bits > d->flags.count())
		bits = d->flags.count();

	// TODO: Optimizations:
	// - *flagBool++ = (curByte & 0x01)?
	uint8_t curByte = 0;
	bool *flagBool = d->flags.data();
	for (int i = 0; i < bits; i++, flagBool++) {
		if (i % 8 == 0) {
			// New byte.
			curByte = *data++;
		}

		// Set this flag.
		*flagBool = (curByte & 0x01);
		curByte >>= 1;
	}

	emit flagsChanged(0, bits-1);
	return bits;
}

/**
 * Get the desired page size for the BitFlagsModel.
 * @return Page size.
 */
int BitFlags::pageSize(void) const
{
	// Default is no pagination.
	return 0;
}

/**
 * Get the name for a given page of data.
 *
 * If pagination is enabled (pageSize > 0), this function is
 * used to determine the text for the corresponding tab.
 *
 * @param page Page number.
 * @return Page name.
 */
QString BitFlags::pageName(int page) const
{
	// Default is no pagination.
	Q_UNUSED(page)
	return QString();
}
