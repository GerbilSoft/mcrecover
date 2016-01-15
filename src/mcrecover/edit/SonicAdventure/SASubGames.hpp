/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * SASubGames.hpp: Sonic Adventure - Sub Games editor.                     *
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

#ifndef __MCRECOVER_EDIT_SONICADVENTURE_SASUBGAMES_HPP__
#define __MCRECOVER_EDIT_SONICADVENTURE_SASUBGAMES_HPP__

#include <QtGui/QWidget>

struct _sa_save_slot;
struct _sadx_extra_save_slot;

class SASubGamesPrivate;
class SASubGames : public QWidget
{
	Q_OBJECT

	public:
		SASubGames(QWidget *parent = 0);
		~SASubGames();

	protected:
		SASubGamesPrivate *const d_ptr;
		Q_DECLARE_PRIVATE(SASubGames)
	private:
		Q_DISABLE_COPY(SASubGames)

	protected:
		// State change event. (Used for switching the UI language at runtime.)
		void changeEvent(QEvent *event);

	protected slots:
		/**
		 * The selected character was changed.
		 * @param index New character ID.
		 */
		void on_cboCharacter_currentIndexChanged(int index);

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
		int save(_sa_save_slot *sa_save);

		/**
		 * Load data from a Sonic Adventure DX extra save slot.
		 * @param sadx_extra_save Sonic Adventure DX extra save slot.
		 * The data must have already been byteswapped to host-endian.
		 * If nullptr, SADX editor components will be hidden.
		 * @return 0 on success; non-zero on error.
		 */
		int loadDX(const _sadx_extra_save_slot *sadx_extra_save);

		/**
		 * Save data to a Sonic Adventure DX extra save slot.
		 * @param sadx_extra_save Sonic Adventure DX extra save slot.
		 * The data will be in host-endian format.
		 * @return 0 on success; non-zero on error.
		 */
		int saveDX(_sadx_extra_save_slot *sadx_extra_save);

		/**
		 * Clear the loaded data.
		 */
		void clear(void);
};

#endif /* __MCRECOVER_EDIT_SONICADVENTURE_SASUBGAMES_HPP__ */
