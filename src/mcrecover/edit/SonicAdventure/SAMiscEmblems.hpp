/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * SAMiscEmblems.hpp: Sonic Adventure - Miscellaneous Emblems editor.      *
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

#ifndef __MCRECOVER_EDIT_SONICADVENTURE_SAMISCEMBLEMS_HPP__
#define __MCRECOVER_EDIT_SONICADVENTURE_SAMISCEMBLEMS_HPP__

#include "SAEditWidget.hpp"

struct _sa_save_slot;

class SAMiscEmblemsPrivate;
class SAMiscEmblems : public SAEditWidget
{
	Q_OBJECT
	typedef SAEditWidget super;

	public:
		explicit SAMiscEmblems(QWidget *parent = 0);
		virtual ~SAMiscEmblems();

	protected:
		SAMiscEmblemsPrivate *const d_ptr;
		Q_DECLARE_PRIVATE(SAMiscEmblems)
	private:
		Q_DISABLE_COPY(SAMiscEmblems)

	protected:
		// State change event. (Used for switching the UI language at runtime.)
		virtual void changeEvent(QEvent *event) final;

	public:
		/**
		 * Load data from a Sonic Adventure save slot.
		 * @param sa_save Sonic Adventure save slot.
		 * The data must have already been byteswapped to host-endian.
		 * @return 0 on success; non-zero on error.
		 */
		virtual int load(const _sa_save_slot *sa_save) final;

		/**
		 * Save data to a Sonic Adventure save slot.
		 * @param sa_save Sonic Adventure save slot.
		 * The data will be in host-endian format.
		 * @return 0 on success; non-zero on error.
		 */
		virtual int save(_sa_save_slot *sa_save) final;
};

#endif /* __MCRECOVER_EDIT_SONICADVENTURE_SAMISCEMBLEMS_HPP__ */
