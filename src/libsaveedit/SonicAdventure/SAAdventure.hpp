/***************************************************************************
 * GameCube Memory Card Recovery Program [libsaveedit]                     *
 * SAAdventure.hpp: Sonic Adventure - Adventure Mode status editor.        *
 *                                                                         *
 * Copyright (c) 2015-2025 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#pragma once

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
