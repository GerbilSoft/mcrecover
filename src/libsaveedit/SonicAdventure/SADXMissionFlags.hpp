/***************************************************************************
 * GameCube Memory Card Recovery Program [libsaveedit]                     *
 * SADXMissionFlags.hpp: Sonic Adventure DX - Mission flags.               *
 *                                                                         *
 * Copyright (c) 2015-2025 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#pragma once

#include "../models/ByteFlags.hpp"
class SADXMissionFlagsPrivate;
class SADXMissionFlags : public ByteFlags
{
	// TODO: Should this actually inherit from QObject?
	Q_OBJECT

	public:
		explicit SADXMissionFlags(QObject *parent = 0);
		virtual ~SADXMissionFlags();

	private:
		typedef ByteFlags super;
		SADXMissionFlagsPrivate *d_ptr;
		Q_DECLARE_PRIVATE(SADXMissionFlags)
		Q_DISABLE_COPY(SADXMissionFlags)

	public:
		/**
		 * Get a description of the type of object that is represented by the class.
		 * @return Flag type, e.g. "Mission".
		 */
		QString objectType(void) const final;

		/**
		 * Get a description of the type of flag represented by a given bit.
		 * @param bit Bit index. (LSB == 0)
		 * @return Flag type, e.g. "Completed". (If bit is unused, empty QString is returned.)
		 */
		QString flagType(int bit) const final;

		/**
		 * Get a character icon representing a flag.
		 * TODO: Make this more generic?
		 * @param id Object ID.
		 * @return Character icon.
		 */
		QPixmap icon(int id) const final;
};
