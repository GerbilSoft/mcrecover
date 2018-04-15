/***************************************************************************
 * GameCube Memory Card Recovery Program [libsaveedit]                     *
 * SANPCFlags.cpp: Sonic Adventure - Event flags.                          *
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

#include "SANPCFlags.hpp"
#include "SAData.h"

// Total number of NPC Flags.
#define SA_NPC_FLAG_COUNT 512

/** SANPCFlags **/

// TODO: NPC flag descriptions.
SANPCFlags::SANPCFlags(QObject *parent)
	: super(512,		// Total number of bit flags.
		nullptr,	// Translation context.
		nullptr,	// Bit flags with names.
		0,		// Number of named flags.
		parent)
{ }

/**
 * Get a description of the type of flag that is represented by the class.
 * @return Flag type, e.g. "Event".
 */
QString SANPCFlags::flagType(void) const
{
	return tr("NPC");
}
