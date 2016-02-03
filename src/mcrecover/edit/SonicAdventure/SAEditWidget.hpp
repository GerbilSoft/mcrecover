/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * SAEditWidget.hpp: Sonic Adventure - SA1/SADX edit widget base class.    *
 *                                                                         *
 * Copyright (c) 2015-2016 by David Korth.                                 *
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

#ifndef __MCRECOVER_EDIT_SONICADVENTURE_SAEDITWIDGET_HPP__
#define __MCRECOVER_EDIT_SONICADVENTURE_SAEDITWIDGET_HPP__

#include <QtGui/QWidget>

struct _sa_save_slot;

class SAEditWidget : public QWidget
{
	Q_OBJECT
	typedef QWidget super;

	public:
		SAEditWidget(QWidget *parent = 0)
			: super(parent) { }

	private:
		Q_DISABLE_COPY(SAEditWidget)

	public:
		/**
		 * Clear the loaded data.
		 * TODO: Make this function pure virtual.
		 */
		virtual void clear(void) { };

	public:
		/**
		 * Load data from a Sonic Adventure save slot.
		 * @param sa_save Sonic Adventure save slot.
		 * The data must have already been byteswapped to host-endian.
		 * @return 0 on success; non-zero on error.
		 */
		virtual int load(const _sa_save_slot *sa_save) = 0;

		/**
		 * Save data to a Sonic Adventure save slot.
		 * @param sa_save Sonic Adventure save slot.
		 * The data will be in host-endian format.
		 * @return 0 on success; non-zero on error.
		 */
		virtual int save(_sa_save_slot *sa_save) const = 0;
};

#endif /* __MCRECOVER_EDIT_SONICADVENTURE_SAEDITWIDGET_HPP__ */
