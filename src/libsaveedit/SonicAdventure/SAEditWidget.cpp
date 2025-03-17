/***************************************************************************
 * GameCube Memory Card Recovery Program [libsaveedit]                     *
 * SAEditWidget.cpp: Sonic Adventure - SA1/SADX edit widget base class.    *
 *                                                                         *
 * Copyright (c) 2015-2025 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#include "SAEditWidget.hpp"

/**
 * Change the 'modified' state.
 * This function must be called instead of modifying
 * the variable directly in order to handle signals.
 * @param modified New 'modified' state.
 */
void SAEditWidget::setModified(bool modified)
{
	if (m_modified == modified)
		return;
	m_modified = modified;
	emit hasBeenModified(modified);
}

/**
 * Connect a widget's modified signal to this slot
 * to automatically set the modified state.
 */
void SAEditWidget::widgetModifiedSlot(void)
{
	if (m_suspendHasBeenModified <= 0) {
		setModified(true);
	}
}
