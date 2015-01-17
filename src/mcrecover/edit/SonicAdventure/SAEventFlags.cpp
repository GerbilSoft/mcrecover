/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * SAEventFlags.cpp: Sonic Adventure event flags.                          *
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

#include "SAEventFlags.hpp"
#include "sa_event_flags.data.h"

// Qt includes.
#include <QtCore/QHash>
#include <QtCore/QString>

// TODO: Put this in a common header file somewhere.
#define NUM_ELEMENTS(x) ((int)(sizeof(x) / sizeof(x[0])))

/** SAEventFlagsPrivate **/

class SAEventFlagsPrivate
{
	public:
		SAEventFlagsPrivate();
		~SAEventFlagsPrivate();

	private:
		Q_DISABLE_COPY(SAEventFlagsPrivate)

	public:
		// TODO: Shared copy of the hash map that's only
		// deleted once all SAEventFlags are deleted?
		QHash<int, QString> eventFlags_desc;

		// Event flags.
		// TODO: Use constants from sa_defs.h.
		bool eventFlags[512];
};

SAEventFlagsPrivate::SAEventFlagsPrivate()
{
	// Initialize the eventFlags hash map.
	// NOTE: sa_event_flags is NULL-terminated.
	eventFlags_desc.clear();
	eventFlags_desc.reserve(NUM_ELEMENTS(sa_event_flags_desc)-1);
	for (int i = 0; i < NUM_ELEMENTS(sa_event_flags_desc)-1; i++) {
		eventFlags_desc.insert(sa_event_flags_desc[i].event,
			QLatin1String(sa_event_flags_desc[i].description));
	}

	// Initialize eventFlags.
	memset(eventFlags, 0, sizeof(eventFlags));
}

SAEventFlagsPrivate::~SAEventFlagsPrivate()
{ }

/** SAEventFlags **/

SAEventFlags::SAEventFlags(QObject *parent)
	: QObject(parent)
	, d_ptr(new SAEventFlagsPrivate())
{ }

SAEventFlags::~SAEventFlags()
{
	Q_D(SAEventFlags);
	delete d;
}

/**
 * Get the total number of event flags.
 * @return Total number of event flags.
 */
int SAEventFlags::count(void) const
{
	Q_D(const SAEventFlags);
	return NUM_ELEMENTS(d->eventFlags);
}

/**
 * Get an event flag's description.
 * @param event Event ID.
 * @return Description.
 */
QString SAEventFlags::description(int event) const
{
	if (event < 0 || event >= count())
		return tr("Invalid event ID");

	Q_D(const SAEventFlags);
	return d->eventFlags_desc.value(event, tr("Unknown"));
}

/**
 * Is an event flag set?
 * @param event Event ID.
 * @return True if set; false if not.
 */
bool SAEventFlags::flag(int event) const
{
	if (event < 0 || event >= count())
		return false;

	Q_D(const SAEventFlags);
	return d->eventFlags[event];
}

/**
 * Set an event flag.
 * @param event Event ID.
 * @param value New event flag value.
 */
void SAEventFlags::setFlag(int event, bool value)
{
	if (event < 0 || event >= count())
		return;

	Q_D(SAEventFlags);
	d->eventFlags[event] = value;
	// TODO: Emit a signal?
}
