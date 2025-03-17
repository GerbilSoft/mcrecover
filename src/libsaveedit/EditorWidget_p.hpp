/***************************************************************************
 * GameCube Memory Card Recovery Program [libsaveedit]                     *
 * EditorWidget_p.hpp: Editor widget base class. (PRIVATE)                 *
 *                                                                         *
 * Copyright (c) 2012-2025 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#pragma once

#include "EditorWidget.hpp"

// C includes. (C++ namespace)
#include <cassert>

class EditorWidgetPrivate
{
public:
	explicit EditorWidgetPrivate(EditorWidget *q);
	virtual ~EditorWidgetPrivate();

protected:
	EditorWidget *const q_ptr;
	Q_DECLARE_PUBLIC(EditorWidget)
private:
	Q_DISABLE_COPY(EditorWidgetPrivate)

public:
	// Opened file.
	File *file;

	// NOTE: Subclasses must emit the appropriate signals
	// when changing these. Use the helper functions.
	int saveSlots;
	bool generalSettings;
	int currentSaveSlot;

	/** Convenience functions for EditorWidget subclasses **/

	/**
	 * Set the number of save slots.
	 * @param saveSlots New number of save slots (Must be at least 1!)
	 */
	void setSaveSlots(int saveSlots);

	/**
	 * Set the "general" settings status.
	 * @param hasGeneralSettings True if this editor has a "general" settings section.
	 */
	void setGeneralSettings(bool generalSettings);
};
