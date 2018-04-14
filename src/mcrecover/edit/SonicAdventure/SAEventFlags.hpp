/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * SAEventFlags.hpp: Sonic Adventure - Event flags.                        *
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

#ifndef __MCRECOVER_EDIT_SONICADVENTURE_SAEVENTFLAGS_HPP__
#define __MCRECOVER_EDIT_SONICADVENTURE_SAEVENTFLAGS_HPP__

#include "../models/BitFlags.hpp"

class SAEventFlags : public BitFlags
{
	// TODO: Should this actually inherit from QObject?
	Q_OBJECT

	public:
		explicit SAEventFlags(QObject *parent = 0);

	private:
		typedef BitFlags super;
		Q_DISABLE_COPY(SAEventFlags)

	public:
		/**
		 * Get a description of the type of flag that is represented by the class.
		 * @return Flag type, e.g. "Event".
		 */
		QString flagType(void) const final;

		/**
		 * Get the desired page size for the BitFlagsModel.
		 * @return Page size.
		 */
		int pageSize(void) const final;

		/**
		 * Get the name for a given page of data.
		 *
		 * If pagination is enabled (pageSize > 0), this function is
		 * used to determine the text for the corresponding tab.
		 *
		 * @param page Page number.
		 * @return Page name.
		 */
		QString pageName(int page) const final;
};

#endif /* __MCRECOVER_EDIT_SONICADVENTURE_SAEVENTFLAGS_HPP__ */
