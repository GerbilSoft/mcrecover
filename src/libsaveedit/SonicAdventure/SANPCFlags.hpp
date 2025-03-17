/***************************************************************************
 * GameCube Memory Card Recovery Program [libsaveedit]                     *
 * SANPCFlags.hpp: Sonic Adventure - NPC flags.                            *
 *                                                                         *
 * Copyright (c) 2015-2025 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#pragma once

#include "../models/BitFlags.hpp"

class SANPCFlags : public BitFlags
{
	// TODO: Should this actually inherit from QObject?
	Q_OBJECT

	public:
		explicit SANPCFlags(QObject *parent = 0);

	private:
		typedef BitFlags super;
		Q_DISABLE_COPY(SANPCFlags)

	public:
		/**
		 * Get a description of the type of flag that is represented by the class.
		 * @return Flag type, e.g. "Event".
		 */
		QString flagType(void) const final;
};
