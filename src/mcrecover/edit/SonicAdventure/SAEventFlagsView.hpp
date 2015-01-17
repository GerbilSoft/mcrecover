/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * SAEventFlagsView.hpp: Sonic Adventure - Event Flags editor.             *
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

#ifndef __MCRECOVER_EDIT_SONICADVENTURE_SAEVENTFLAGSVIEW_HPP__
#define __MCRECOVER_EDIT_SONICADVENTURE_SAEVENTFLAGSVIEW_HPP__

#include <QtGui/QWidget>

struct _sa_event_flags;

class SAEventFlagsViewPrivate;
class SAEventFlagsView : public QWidget
{
	Q_OBJECT

	public:
		SAEventFlagsView(QWidget *parent = 0);
		~SAEventFlagsView();

	protected:
		SAEventFlagsViewPrivate *const d_ptr;
		Q_DECLARE_PRIVATE(SAEventFlagsView)
	private:
		Q_DISABLE_COPY(SAEventFlagsView)

	public:
		/** Data access. **/

		/**
		 * Get the event flags from the widget.
		 * Flags are copied from the widget.
		 * @param eventFlags sa_event_flags to store the event flags in.
		 */
		void eventFlags(_sa_event_flags *eventFlags) const;

		/**
		 * Set the event flags for the widget.
		 * Flags are copied into the widget.
		 * @param eventFlags sa_event_flags to store in the widget.
		 */
		void setEventFlags(const _sa_event_flags *eventFlags);
};

#endif /* __MCRECOVER_EDIT_SONICADVENTURE_SAEVENTFLAGSVIEW_HPP__ */
