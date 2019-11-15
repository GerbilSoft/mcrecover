/***************************************************************************
 * GameCube Memory Card Recovery Program [libsaveedit]                     *
 * SASubGames.hpp: Sonic Adventure - Sub Games editor.                     *
 *                                                                         *
 * Copyright (c) 2015-2018 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#ifndef __LIBSAVEEDIT_SONICADVENTURE_SASUBGAMES_HPP__
#define __LIBSAVEEDIT_SONICADVENTURE_SASUBGAMES_HPP__

#include "SADXEditWidget.hpp"

struct _sa_save_slot;
struct _sadx_extra_save_slot;

class SASubGamesPrivate;
class SASubGames : public SADXEditWidget
{
	Q_OBJECT

	public:
		explicit SASubGames(QWidget *parent = 0);
		virtual ~SASubGames();

	private:
		typedef SADXEditWidget super;
		SASubGamesPrivate *const d_ptr;
		Q_DECLARE_PRIVATE(SASubGames)
		Q_DISABLE_COPY(SASubGames)

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

	protected slots:
		/**
		 * The selected character was changed.
		 * @param index New character ID.
		 */
		void on_cboCharacter_currentIndexChanged(int index);
};

#endif /* __LIBSAVEEDIT_SONICADVENTURE_SASUBGAMES_HPP__ */
