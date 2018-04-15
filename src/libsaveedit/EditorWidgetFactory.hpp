/***************************************************************************
 * GameCube Memory Card Recovery Program [libsaveedit]                     *
 * EditorWidgetFactory.hpp: EditorWidget factory class.                    *
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

#ifndef __LIBSAVEEDIT_EDITORWIDGETFACTORY_HPP__
#define __LIBSAVEEDIT_EDITORWIDGETFACTORY_HPP__

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
		 * @param file File to edit.
		 * @return EditorWidget, or nullptr if no editors support this file.
		 */
		static EditorWidget *createWidget(File *file);

		// TODO:
		// - Get list of editors that support a given file.
		// - Add option to select a specific editor.

		/**
		 * Is an editor available for the specified file?
		 * @param file File to edit.
		 * @return True if an editor is available; false if not.
		 */
		static bool isEditorAvailable(const File *file);
};

#endif /* __LIBSAVEEDIT_EDITORWIDGETFACTORY_HPP__ */
