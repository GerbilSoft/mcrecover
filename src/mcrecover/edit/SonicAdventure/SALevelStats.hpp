/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * SALevelStats.hpp: Sonic Adventure - Level Stats editor.                 *
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

#ifndef __MCRECOVER_EDIT_SONICADVENTURE_SALEVELSTATS_HPP__
#define __MCRECOVER_EDIT_SONICADVENTURE_SALEVELSTATS_HPP__

#include "SADXEditWidget.hpp"

struct _sa_save_slot;
struct _sadx_extra_save_slot;

class SALevelStatsPrivate;
class SALevelStats : public SADXEditWidget
{
	Q_OBJECT
	typedef SADXEditWidget super;

	public:
		SALevelStats(QWidget *parent = 0);
		virtual ~SALevelStats();

	protected:
		SALevelStatsPrivate *const d_ptr;
		Q_DECLARE_PRIVATE(SALevelStats)
	private:
		Q_DISABLE_COPY(SALevelStats)

	protected:
		// State change event. (Used for switching the UI language at runtime.)
		virtual void changeEvent(QEvent *event) final;

	public:
		/**
		 * Clear the loaded data.
		 */
		virtual void clear(void) final;

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
		virtual int save(_sa_save_slot *sa_save) const final;

	public:
		/**
		 * Load data from a Sonic Adventure DX extra save slot.
		 * @param sadx_extra_save Sonic Adventure DX extra save slot.
		 * The data must have already been byteswapped to host-endian.
		 * If nullptr, SADX editor components will be hidden.
		 * @return 0 on success; non-zero on error.
		 */
		virtual int loadDX(const _sadx_extra_save_slot *sadx_extra_save) final;

		/**
		 * Save data to a Sonic Adventure DX extra save slot.
		 * @param sadx_extra_save Sonic Adventure DX extra save slot.
		 * The data will be in host-endian format.
		 * @return 0 on success; non-zero on error.
		 */
		virtual int saveDX(_sadx_extra_save_slot *sadx_extra_save) const final;

	protected slots:
		/**
		 * The selected character was changed.
		 * @param index New character ID.
		 */
		void on_cboCharacter_currentIndexChanged(int index);
};

#endif /* __MCRECOVER_EDIT_SONICADVENTURE_SALEVELSTATS_HPP__ */
