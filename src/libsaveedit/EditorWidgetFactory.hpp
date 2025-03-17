/***************************************************************************
 * GameCube Memory Card Recovery Program [libsaveedit]                     *
 * EditorWidgetFactory.hpp: EditorWidget factory class.                    *
 *                                                                         *
 * Copyright (c) 2015-2025 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#pragma once

// for Q_DISABLE_COPY()
#include <QtCore/qglobal.h>

// TODO: Namespace?
class File;

class EditorWidget;
class EditorWidgetFactory
{
// TODO: Check EmuContextFactory before committing.
private:
	EditorWidgetFactory();
	~EditorWidgetFactory();
private:
	Q_DISABLE_COPY(EditorWidgetFactory)

public:
	/**
	 * Create an EditorWidget for the specified file.
	 * @param file File to edit
	 * @return EditorWidget, or nullptr if no editors support this file.
	 */
	static EditorWidget *createWidget(File *file);

	// TODO:
	// - Get list of editors that support a given file.
	// - Add option to select a specific editor.

	/**
	 * Is an editor available for the specified file?
	 * @param file File to edit
	 * @return True if an editor is available; false if not.
	 */
	static bool isEditorAvailable(const File *file);
};
