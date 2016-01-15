/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * SAEventFlags.hpp: Sonic Adventure - Event flags.                        *
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

#ifndef __MCRECOVER_EDIT_SONICADVENTURE_SAEVENTFLAGS_HPP__
#define __MCRECOVER_EDIT_SONICADVENTURE_SAEVENTFLAGS_HPP__

#include "BitFlags.hpp"

class SAEventFlagsPrivate;
class SAEventFlags : public BitFlags
{
	// TODO: Should this actually inherit from QObject?
	Q_OBJECT

	public:
		SAEventFlags(QObject *parent = 0);

	protected:
		Q_DECLARE_PRIVATE(SAEventFlags)
	private:
		Q_DISABLE_COPY(SAEventFlags)

	public:
		/**
		 * Get a description of the type of flag that is represented by the class.
		 * @return Flag type, e.g. "Event".
		 */
		virtual QString flagType(void) const override;
};

#endif /* __MCRECOVER_EDIT_SONICADVENTURE_SAEVENTFLAGS_HPP__ */
