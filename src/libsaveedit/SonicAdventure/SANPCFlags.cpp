/***************************************************************************
 * GameCube Memory Card Recovery Program [libsaveedit]                     *
 * SANPCFlags.cpp: Sonic Adventure - Event flags.                          *
 *                                                                         *
 * Copyright (c) 2015-2018 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
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
