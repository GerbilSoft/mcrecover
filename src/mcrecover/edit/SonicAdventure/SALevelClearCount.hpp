/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * SALevelClearCount.hpp: Sonic Adventure - Level Clear Count editor.      *
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

#ifndef __MCRECOVER_EDIT_SONICADVENTURE_SALEVELCLEARCOUNT_HPP__
#define __MCRECOVER_EDIT_SONICADVENTURE_SALEVELCLEARCOUNT_HPP__

#include "SAEditWidget.hpp"

struct _sa_save_slot;

class SALevelClearCountPrivate;
class SALevelClearCount : public SAEditWidget
{
	Q_OBJECT

	public:
		explicit SALevelClearCount(QWidget *parent = 0);
		virtual ~SALevelClearCount();

	private:
		typedef SAEditWidget super;
		SALevelClearCountPrivate *const d_ptr;
		Q_DECLARE_PRIVATE(SALevelClearCount)
		Q_DISABLE_COPY(SALevelClearCount)

	protected:
		// State change event. (Used for switching the UI language at runtime.)
		void changeEvent(QEvent *event) final;

	public:
		/**
		 * Clear the loaded data.
		 */
		void clear(void) final;

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

	protected slots:
		/**
		 * spnCount signal mapper.
		 * @param spnId Spinbox ID. (0xAABB; AA == level, BB == character)
		 */
		void spnCount_mapped_slot(int spnId);

	protected:
		/**
		 * QObject eventFilter.
		 * Used to handle QScrollArea resize and QScrollBar show events.
		 * @param watched Watched QObject.
		 * @param event QEvent.
		 * @return True to stop the event from being handled further; false to pass it down.
		 */
		bool eventFilter(QObject *watched, QEvent *event) final;
};

#endif /* __MCRECOVER_EDIT_SONICADVENTURE_SALEVELCLEARCOUNT_HPP__ */
