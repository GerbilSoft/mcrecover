/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * SAEventFlags.hpp: Sonic Adventure event flags.                          *
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

// Qt includes.
// TODO: Should this actually inherit from QObject?
#include <QtCore/QObject>

// TODO: Use a generic sparse map interface so this
// can also be used for NPC flags?

class SAEventFlagsPrivate;
class SAEventFlags : public QObject
{
	Q_OBJECT

	public:
		SAEventFlags(QObject *parent = 0);
		virtual ~SAEventFlags();

	private:
		SAEventFlagsPrivate *d_ptr;
		Q_DECLARE_PRIVATE(SAEventFlags);

	public:
		/**
		 * Get the total number of event flags.
		 * @return Total number of event flags.
		 */
		int count(void) const;

		/**
		 * Get an event flag's description.
		 * @param event Event ID.
		 * @return Description.
		 */
		QString description(int event) const;

		/**
		 * Is an event flag set?
		 * @param event Event ID.
		 * @return True if set; false if not.
		 */
		bool flag(int event) const;

		/**
		 * Set an event flag.
		 * @param event Event ID.
		 * @param value New event flag value.
		 */
		void setFlag(int event, bool value);
};

#endif /* __MCRECOVER_EDIT_SONICADVENTURE_SAEVENTFLAGS_HPP__ */
