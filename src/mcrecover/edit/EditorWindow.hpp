/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * EditorWindow.hpp: Save file editor.                                     *
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

#ifndef __MCRECOVER_EDIT_EDITORWINDOW_HPP__
#define __MCRECOVER_EDIT_EDITORWINDOW_HPP__

#include <QtGui/QMainWindow>

class GcnFile;
class VmuFile;

class EditorWindowPrivate;
class EditorWindow : public QMainWindow
{
	Q_OBJECT
	typedef QMainWindow super;

	protected:
		EditorWindow(QWidget *parent = nullptr);
	public:
		~EditorWindow();

	protected:
		EditorWindowPrivate *const d_ptr;
		Q_DECLARE_PRIVATE(EditorWindow)
	private:
		Q_DISABLE_COPY(EditorWindow)

	protected:
		// State change event. (Used for switching the UI language at runtime.)
		void changeEvent(QEvent *event);

	/** Public functions. **/

	public:
		/**
		 * Edit a GcnFile.
		 * TODO: Combine with editVmuFile.
		 * @param gcnFile GcnFile to edit.
		 * @return Editor window for the GcnFile. (nullptr if the file cannot be edited)
		 */
		static EditorWindow *editGcnFile(GcnFile *gcnFile);

		/**
		 * Edit a VmuFile.
		 * TODO: Combine with editGcnFile.
		 * @param vmuFile VmuFile to edit.
		 * @return Editor dialog for the VmuFile. (nullptr if the file cannot be edited)
		 */
		static EditorWindow *editVmuFile(VmuFile *VmuFile);

	protected slots:
		/** Widget slots. **/

		/**
		 * A save slot button has been clicked.
		 * @param saveSlot Save slot. (-1 for "general" settings.)
		 */
		void saveSlotButton_clicked(int saveSlot);
};

#endif /* __MCRECOVER_EDIT_EDITORWINDOW_HPP__ */
