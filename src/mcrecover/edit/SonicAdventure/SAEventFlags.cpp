/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * SAEventFlags.cpp: Sonic Adventure - Event flags.                        *
 *                                                                         *
 * Copyright (c) 201-20165 by David Korth.                                 *
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
#include "SAData.h"

// TODO: Put this in a common header file somewhere.
#define NUM_ELEMENTS(x) ((int)(sizeof(x) / sizeof(x[0])))

/** SAEventFlagsPrivate **/
#include "../models/BitFlags_p.hpp"
class SAEventFlagsPrivate : public BitFlagsPrivate
{
	public:
		SAEventFlagsPrivate();

	private:
		Q_DISABLE_COPY(SAEventFlagsPrivate)

	public:
		static const int flagCount = 512;
};

// NOTE: NUM_ELEMENTS() includes the NULL-terminator.
SAEventFlagsPrivate::SAEventFlagsPrivate()
	: BitFlagsPrivate(512, &sa_event_flags_desc[0], NUM_ELEMENTS(sa_event_flags_desc))
{ }

/** SAEventFlags **/

SAEventFlags::SAEventFlags(QObject *parent)
	: BitFlags(new SAEventFlagsPrivate(), parent)
{ }

/**
 * Get a description of the type of flag that is represented by the class.
 * @return Flag type, e.g. "Event".
 */
QString SAEventFlags::flagType(void) const
{
	return tr("Event");
}

/**
 * Get the desired page size for the BitFlagsModel.
 * @return Page size.
 */
int SAEventFlags::pageSize(void) const
{
	// 64 events per character.
	return 64;
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
QString SAEventFlags::pageName(int page) const
{
	switch (page) {
		case 0:	return tr("Unused?");
		case 1:	return tr("General");
		case 2:	return tr("Sonic");
		case 3:	return tr("Tails");
		case 4:	return tr("Knuckles");
		case 5:	return tr("Amy");
		case 6:	return tr("Gamma");
		case 7:	return tr("Big");
		default: break;
	}

	return QString();
}
