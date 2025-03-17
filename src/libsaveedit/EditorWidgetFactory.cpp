/***************************************************************************
 * GameCube Memory Card Recovery Program [libsaveedit]                     *
 * EditorWidgetFactory.hpp: EditorWidget factory class.                    *
 *                                                                         *
 * Copyright (c) 2015-2025 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#include "EditorWidgetFactory.hpp"
#include "EditorWidget.hpp"

// Files.
#include "libmemcard/File.hpp"

// Editor widgets.
#include "SonicAdventure/SAEditor.hpp"

/**
 * Create an EditorWidget for the specified file.
 * @param file File to edit
 * @return EditorWidget, or nullptr if no editors support this file.
 */
EditorWidget *EditorWidgetFactory::createWidget(File *file)
{
	// Check if the file is supported by the various editors.
	EditorWidget *widget = nullptr;
	if (SAEditor::isFileSupported(file)) {
		widget = new SAEditor();
	}

	if (widget) {
		// Found an EditorWidget that accepts this file.
		if (widget->setFile(file) != 0) {
			// Error opening the file...
			// TODO: Error code.
			delete widget;
			widget = nullptr;
		}
	}

	return widget;
}

/**
 * Is an editor available for the specified file?
 * @param file File to edit
 * @return True if an editor is available; false if not.
 */
bool EditorWidgetFactory::isEditorAvailable(const File *file)
{
	// TODO: Register all editor widgets somewhere?

	// Check if the file is supported by the various editors.
	if (SAEditor::isFileSupported(file)) {
		return true;
	}

	// No editors are available for this file.
	return false;
}
