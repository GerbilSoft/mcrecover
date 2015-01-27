/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * SADXMissions.hpp: Sonic Adventure DX - Mission editor.                  *
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

#ifndef __MCRECOVER_EDIT_SONICADVENTURE_SADXMISSIONS_HPP__
#define __MCRECOVER_EDIT_SONICADVENTURE_SADXMISSIONS_HPP__

#include <QtGui/QWidget>

struct _sadx_extra_save_slot;

class SADXMissionsPrivate;
class SADXMissions : public QWidget
{
	Q_OBJECT

	public:
		SADXMissions(QWidget *parent = 0);
		~SADXMissions();

	protected:
		SADXMissionsPrivate *const d_ptr;
		Q_DECLARE_PRIVATE(SADXMissions)
	private:
		Q_DISABLE_COPY(SADXMissions)

	protected:
		// State change event. (Used for switching the UI language at runtime.)
		void changeEvent(QEvent *event);

	public:
		/**
		 * Load data from an SADX extra save slot.
		 * @param sadx_extra_save SADX extra save slot.
		 * The data must have already been byteswapped to host-endian.
		 * @return 0 on success; non-zero on error.
		 */
		int load(const _sadx_extra_save_slot *sadx_extra_save);

		/**
		 * Clear the loaded data.
		 */
		void clear(void);
};

#endif /* __MCRECOVER_EDIT_SONICADVENTURE_SADXMISSIONS_HPP__ */
