/***************************************************************************
 * GameCube Memory Card Recovery Program [libsaveedit]                     *
 * SAEditWidget.hpp: Sonic Adventure - SADX edit widget base class.        *
 *                                                                         *
 * Copyright (c) 2015-2025 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#pragma once

#include "SAEditWidget.hpp"

struct _sadx_extra_save_slot;

class SADXEditWidget : public SAEditWidget
{
	Q_OBJECT

	public:
		explicit SADXEditWidget(QWidget *parent = 0)
			: super(parent) { }

	private:
		typedef SAEditWidget super;
		Q_DISABLE_COPY(SADXEditWidget)

	public:
		/**
		 * Load data from a Sonic Adventure DX extra save slot.
		 * @param sadx_extra_save Sonic Adventure DX extra save slot.
		 * The data must have already been byteswapped to host-endian.
		 * If nullptr, SADX editor components will be hidden.
		 * @return 0 on success; non-zero on error.
		 */
		virtual int loadDX(const _sadx_extra_save_slot *sadx_extra_save) = 0;

		/**
		 * Save data to a Sonic Adventure DX extra save slot.
		 * @param sadx_extra_save Sonic Adventure DX extra save slot.
		 * The data will be in host-endian format.
		 * @return 0 on success; non-zero on error.
		 */
		virtual int saveDX(_sadx_extra_save_slot *sadx_extra_save) = 0;
};
