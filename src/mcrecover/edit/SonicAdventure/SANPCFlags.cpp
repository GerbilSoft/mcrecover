/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * SANPCFlags.cpp: Sonic Adventure - Event flags.                        *
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

#include "SANPCFlags.hpp"

// TODO: Put this in a common header file somewhere.
#define NUM_ELEMENTS(x) ((int)(sizeof(x) / sizeof(x[0])))

/** SANPCFlagsPrivate **/
#include "BitFlags_p.hpp"
class SANPCFlagsPrivate : public BitFlagsPrivate
{
	public:
		SANPCFlagsPrivate();

	private:
		Q_DISABLE_COPY(SANPCFlagsPrivate)

	public:
		static const int flagCount = 512;
};

// NOTE: NUM_ELEMENTS() includes the NULL-terminator.
SANPCFlagsPrivate::SANPCFlagsPrivate()
	: BitFlagsPrivate(512, nullptr, 0)
{ }

/** SANPCFlags **/

SANPCFlags::SANPCFlags(QObject *parent)
	: BitFlags(new SANPCFlagsPrivate(), parent)
{ }

/**
 * Get a description of the type of flag that is represented by the class.
 * @return Flag type, e.g. "Event".
 */
QString SANPCFlags::flagType(void) const
{
	return tr("NPC");
}
