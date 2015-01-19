/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * SAEventFlags.cpp: Sonic Adventure - Event flags.                        *
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

// TODO: Put this in a common header file somewhere.
#define NUM_ELEMENTS(x) ((int)(sizeof(x) / sizeof(x[0])))

/** SAEventFlagsPrivate **/
#include "BitFlags_p.hpp"
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
