/***************************************************************************
 * GameCube Memory Card Recovery Program [libsaveedit]                     *
 * SAGeneral.hpp: Sonic Adventure - General settings editor.               *
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

#ifndef __LIBSAVEEDIT_SONICADVENTURE_SAGENERAL_HPP__
#define __LIBSAVEEDIT_SONICADVENTURE_SAGENERAL_HPP__

#include "SADXEditWidget.hpp"

struct _sa_save_slot;
struct _sadx_extra_save_slot;

class SAGeneralPrivate;
class SAGeneral : public SADXEditWidget
{
	Q_OBJECT

	public:
		explicit SAGeneral(QWidget *parent = 0);
		virtual ~SAGeneral();

	private:
		typedef SADXEditWidget super;
		SAGeneralPrivate *const d_ptr;
		Q_DECLARE_PRIVATE(SAGeneral)
		Q_DISABLE_COPY(SAGeneral)

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

	public:
		/**
		 * Load data from a Sonic Adventure DX extra save slot.
		 * @param sadx_extra_save Sonic Adventure DX extra save slot.
		 * The data must have already been byteswapped to host-endian.
		 * If nullptr, SADX editor components will be hidden.
		 * @return 0 on success; non-zero on error.
		 */
		int loadDX(const _sadx_extra_save_slot *sadx_extra_save) final;

		/**
		 * Save data to a Sonic Adventure DX extra save slot.
		 * @param sadx_extra_save Sonic Adventure DX extra save slot.
		 * The data will be in host-endian format.
		 * @return 0 on success; non-zero on error.
		 */
		int saveDX(_sadx_extra_save_slot *sadx_extra_save) final;
};

#endif /* __LIBSAVEEDIT_SONICADVENTURE_SAGENERAL_HPP__ */
