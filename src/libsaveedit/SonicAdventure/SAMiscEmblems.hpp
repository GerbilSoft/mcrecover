/***************************************************************************
 * GameCube Memory Card Recovery Program [libsaveedit]                     *
 * SAMiscEmblems.hpp: Sonic Adventure - Miscellaneous Emblems editor.      *
 *                                                                         *
 * Copyright (c) 2015-2025 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#pragma once

#include "SAEditWidget.hpp"

struct _sa_save_slot;

class SAMiscEmblemsPrivate;
class SAMiscEmblems : public SAEditWidget
{
	Q_OBJECT

	public:
		explicit SAMiscEmblems(QWidget *parent = 0);
		virtual ~SAMiscEmblems();

	private:
		typedef SAEditWidget super;
		SAMiscEmblemsPrivate *const d_ptr;
		Q_DECLARE_PRIVATE(SAMiscEmblems)
		Q_DISABLE_COPY(SAMiscEmblems)

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
