/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * ByteFlags.cpp: Generic byte flags base class.                           *
 * Used for things where a single object has multiple flags                *
 * stored as a byte.                                                       *
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

#include "ByteFlags.hpp"

// Qt includes.
#include <QtCore/QCoreApplication>
#include <QtCore/QHash>
#include <QtCore/QString>
#include <QtCore/QVector>

// C includes. (C++ namespace)
#include <cassert>

// TODO: Put this in a common header file somewhere.
#define NUM_ELEMENTS(x) ((int)(sizeof(x) / sizeof(x[0])))

/** ByteFlagsPrivate **/

class ByteFlagsPrivate
{
	public:
		/**
		 * Initialize ByteFlagsPrivate.
		 * @param total_flags Total number of flags the user can edit.
		 * @param tr_ctx Translation context for bit flag descriptions.
		 * @param byte_flags Byte flag descriptions.
		 * @param count Number of bit_flags entries. (must be >= total_flags)
		 */
		ByteFlagsPrivate(int total_flags, const char *tr_ctx,
				 const bit_flag_t *byte_flags, int count);

	private:
		Q_DISABLE_COPY(ByteFlagsPrivate)

	public:
		// Object descriptions.
		// TODO: Shared copy shared by a specific derived
		// class that's only deleted once all instances
		// of said class are deleted?
		QVector<const char*> objs_desc;

		// Objects, each with 8 flags.
		QVector<uint8_t> objs;

		// Translation context for bit flags.
		const char *tr_ctx;
};

/**
 * Initialize ByteFlagsPrivate.
 * @param total_flags Total number of flags the user can edit.
 * @param tr_ctx Translation context for bit flag descriptions.
 * @param byte_flags Byte flag descriptions.
 * @param count Number of bit_flags entries. (must be >= total_flags)
 */
ByteFlagsPrivate::ByteFlagsPrivate(int total_flags, const char *tr_ctx,
				   const bit_flag_t *byte_flags, int count)
	: tr_ctx(tr_ctx)
{
	// This is initialized by a derived private class.
	assert(total_flags > 0);
	assert(total_flags >= count);
	assert(count >= 0);

	// Initialize flags.
	// QVector automatically initializes the new elements to false.
	objs.resize(total_flags);

	// Initialize flags_desc.
	// TODO: Once per derived class, rather than once per instance?
	objs_desc.clear();
	objs_desc.resize(total_flags);
	for (int i = 0; i < count; i++, byte_flags++) {
		if (byte_flags->event < 0 || !byte_flags->description) {
			// End of list.
			// NOTE: count should have been set correctly...
			break;
		}

		objs_desc[byte_flags->event] = byte_flags->description;
	}
}

/** ByteFlags **/

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
ByteFlags::ByteFlags(int total_flags, const bit_flag_t *byte_flags,
		     int count, QObject *parent)
	: super(parent)
	, d_ptr(new ByteFlagsPrivate(total_flags, nullptr, byte_flags, count))
{ }

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
ByteFlags::ByteFlags(int total_flags, const char *tr_ctx,
		     const bit_flag_t *byte_flags, int count, QObject *parent)
	: QObject(parent)
	, d_ptr(new ByteFlagsPrivate(total_flags, tr_ctx, byte_flags, count))
{ }

ByteFlags::~ByteFlags()
{
	delete d_ptr;
}

/**
 * Get the total number of objects.
 * @return Total number of objects.
 */
int ByteFlags::count(void) const
{
	Q_D(const ByteFlags);
	return d->objs.size();
}

/**
 * Get an object's description.
 * @param id Object ID.
 * @return Description.
 */
QString ByteFlags::description(int id) const
{
	// TODO: Translate using the subclass?
	if (id < 0 || id >= count())
		return tr("Invalid object ID");

	Q_D(const ByteFlags);
	const char *desc = d->objs_desc[id];
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
 * Get an object's flags.
 * @param id Object ID.
 * @return Object's flags.
 */
uint8_t ByteFlags::flag(int id) const
{
	if (id < 0 || id >= count())
		return false;

	Q_D(const ByteFlags);
	return d->objs.at(id);
}

/**
 * Set an object's flags.
 * @param id Object ID.
 * @param value New flag value.
 */
void ByteFlags::setFlag(int id, uint8_t value)
{
	if (id < 0 || id >= count())
		return;

	Q_D(ByteFlags);
	d->objs[id] = value;
	emit flagChanged(id, value);
}

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
int ByteFlags::allFlags(uint8_t *data, int sz) const
{
	Q_D(const ByteFlags);
	assert(sz > 0);
	if (sz <= 0)
		return 0;
	if (sz > d->objs.count())
		sz = d->objs.count();

	memcpy(data, d->objs.constData(), sz);
	return sz;
}

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
int ByteFlags::setAllFlags(const uint8_t *data, int sz)
{
	Q_D(ByteFlags);
	assert(sz > 0);
	if (sz <= 0)
		return 0;
	if (sz > d->objs.count())
		sz = d->objs.count();

	memcpy(d->objs.data(), data, sz);
	emit flagsChanged(0, sz-1);
	return sz;
}

/**
 * Get a character icon representing a flag.
 * TODO: Make this more generic?
 * @param id Object ID.
 * @return Character icon.
 */
QPixmap ByteFlags::icon(int id) const
{
	// No icons by default...
	Q_UNUSED(id)
	return QPixmap();
}

/**
 * Get the desired page size for the ByteFlagsModel.
 * @return Page size.
 */
int ByteFlags::pageSize(void) const
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
QString ByteFlags::pageName(int page) const
{
	// Default is no pagination.
	Q_UNUSED(page)
	return QString();
}
