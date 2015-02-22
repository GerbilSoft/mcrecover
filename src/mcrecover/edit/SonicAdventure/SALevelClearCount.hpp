/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * SALevelClearCount.hpp: Sonic Adventure - Level Clear Count editor.      *
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

#ifndef __MCRECOVER_EDIT_SONICADVENTURE_SALEVELCLEARCOUNT_HPP__
#define __MCRECOVER_EDIT_SONICADVENTURE_SALEVELCLEARCOUNT_HPP__

#include <QtGui/QWidget>

struct _sa_save_slot;

class SALevelClearCountPrivate;
class SALevelClearCount : public QWidget
{
	Q_OBJECT

	public:
		SALevelClearCount(QWidget *parent = 0);
		~SALevelClearCount();

	protected:
		SALevelClearCountPrivate *const d_ptr;
		Q_DECLARE_PRIVATE(SALevelClearCount)
	private:
		Q_DISABLE_COPY(SALevelClearCount)

	protected:
		// State change event. (Used for switching the UI language at runtime.)
		void changeEvent(QEvent *event);

	public:
		/**
		 * Load data from a Sonic Adventure save slot.
		 * @param sa_save Sonic Adventure save slot.
		 * The data must have already been byteswapped to host-endian.
		 * @return 0 on success; non-zero on error.
		 */
		int load(const _sa_save_slot *sa_save);

		/**
		 * Save data to a Sonic Adventure save slot.
		 * @param sa_save Sonic Adventure save slot.
		 * The data will be in host-endian format.
		 * @return 0 on success; non-zero on error.
		 */
		int save(_sa_save_slot *sa_save) const;

		/**
		 * Clear the loaded data.
		 */
		void clear(void);

	protected slots:
		/**
		 * spnCount signal mapper.
		 * @param spnId Spinbox ID. (0xAABB; AA == level, BB == character)
		 */
		void spnCount_mapped_slot(int spnId);
};

#endif /* __MCRECOVER_EDIT_SONICADVENTURE_SALEVELCLEARCOUNT_HPP__ */
