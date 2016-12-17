/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * EditorWidgetFactory.hpp: EditorWidget factory class.                    *
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

#include "EditorWidgetFactory.hpp"
#include "EditorWidget.hpp"

// Files.
#include "card/File.hpp"

// Editor widgets.
#include "SonicAdventure/SAEditor.hpp"

/**
 * Create an EditorWidget for the specified file.
 * @param file File to edit.
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
 * @param file File to edit.
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
