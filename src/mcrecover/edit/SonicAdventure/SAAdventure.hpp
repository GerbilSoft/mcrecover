/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * SAAdventure.hpp: Sonic Adventure - Adventure Mode status editor.        *
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

#ifndef __MCRECOVER_EDIT_SONICADVENTURE_SAADVENTURE_HPP__
#define __MCRECOVER_EDIT_SONICADVENTURE_SAADVENTURE_HPP__

#include "SAEditWidget.hpp"

struct _sa_save_slot;

class SAAdventurePrivate;
class SAAdventure : public SAEditWidget
{
	Q_OBJECT

	public:
		explicit SAAdventure(QWidget *parent = 0);
		virtual ~SAAdventure();

	private:
		typedef SAEditWidget super;
		SAAdventurePrivate *const d_ptr;
		Q_DECLARE_PRIVATE(SAAdventure)
		Q_DISABLE_COPY(SAAdventure)

	protected:
		// State change event. (Used for switching the UI language at runtime.)
		void changeEvent(QEvent *event) final;

	public:
		/**
		 * Load data from a Sonic Adventure save slot.
		 * @param sa_save Sonic Adventure save slot.
		 * The data must have already been byteswapped to host-endian.
		 * @return 0 on success; non-zero on error.
		 */
		int load(const _sa_save_slot *sa_save) final;

		/**
		 * Save data to a Sonic Adventure save slot.
		 * @param sa_save Sonic Adventure save slot.
		 * The data will be in host-endian format.
		 * @return 0 on success; non-zero on error.
		 */
		int save(_sa_save_slot *sa_save) final;
};

#endif /* __MCRECOVER_EDIT_SONICADVENTURE_SAADVENTURE_HPP__ */
