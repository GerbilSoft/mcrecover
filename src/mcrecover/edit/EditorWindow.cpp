/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * EditorWindow.cpp: Save file editor window.                              *
 *                                                                         *
 * Copyright (c) 2015 by David Korth.                                      *
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

#include "EditorWindow.hpp"

// Qt includes.
#include <QtCore/QEvent>

// Files.
#include "../card/GcnFile.hpp"
#include "../card/VmuFile.hpp"

// Editors.
#include "SonicAdventure/SAEditor.hpp"

/** EditorWindowPrivate **/

#include "ui_EditorWindow.h"
class EditorWindowPrivate
{
	public:
		EditorWindowPrivate(EditorWindow *q);

	protected:
		EditorWindow *const q_ptr;
		Q_DECLARE_PUBLIC(EditorWindow)
	private:
		Q_DISABLE_COPY(EditorWindowPrivate)

	public:
		Ui::EditorWindow ui;

		// Editor widget.
		// TODO: EditorWidget base class?
		QWidget *editorWidget;

		/**
		 * Set the editor widget.
		 * The editor widget will be owned by this EditorWindow.
		 * @param editorWidget Editor widget.
		 */
		void setEditorWidget(QWidget *editorWidget);
};

EditorWindowPrivate::EditorWindowPrivate(EditorWindow* q)
	: q_ptr(q)
	, editorWidget(nullptr)
{ }

/**
 * Set the editor widget.
 * The editor widget will be owned by this EditorWindow.
 * @param editor Editor widget.
 */
void EditorWindowPrivate::setEditorWidget(QWidget *editorWidget)
{
	if (this->editorWidget == editorWidget)
		return;

	// Delete the existing editor widget.
	delete this->editorWidget;

	// Add the new editor widget.
	this->editorWidget = editorWidget;
	if (editorWidget != nullptr) {
		// TODO: Connect the destroyed signal.
		ui.vboxMain->insertWidget(0, editorWidget, 0, Qt::AlignTop);
	}
}

/** EditorWindow **/

/**
 * Initialize the save file editor.
 * @param parent Parent widget.
 */
EditorWindow::EditorWindow(QWidget *parent)
	: QDialog(parent,
		Qt::Dialog |
		Qt::CustomizeWindowHint |
		Qt::WindowTitleHint |
		Qt::WindowSystemMenuHint |
		Qt::WindowCloseButtonHint)
	, d_ptr(new EditorWindowPrivate(this))
{
	Q_D(EditorWindow);
	d->ui.setupUi(this);

	// NOTE: This window should NOT be deleted on close,
	// because the owner needs to retrieve the modified.
	// save file data.
	//this->setAttribute(Qt::WA_DeleteOnClose, true);

#ifdef Q_OS_MAC
	// Remove the window icon. (Mac "proxy icon")
	this->setWindowIcon(QIcon());
#endif
}

/**
 * Shut down the save file editor.
 */
EditorWindow::~EditorWindow()
{
	delete d_ptr;
}

/**
 * Widget state has changed.
 * @param event State change event.
 */
void EditorWindow::changeEvent(QEvent *event)
{
	if (event->type() == QEvent::LanguageChange) {
		// Retranslate the UI.
		Q_D(EditorWindow);
		d->ui.retranslateUi(this);
	}

	// Pass the event to the base class.
	this->QDialog::changeEvent(event);
}

/** Public functions. **/

/**
 * Edit a GcnFile.
 * TODO: Combine with editVmuFile.
 * @param gcnFile GcnFile to edit.
 * @return Editor dialog for the GcnFile. (nullptr if the file cannot be edited)
 */
EditorWindow *EditorWindow::editGcnFile(GcnFile *gcnFile)
{
	// TODO: Connect the 'destroyed' signal and
	// close the editor if the file is destroyed.

	// TODO: Check for editors that support this file.
	// For now, only allow SADX.
	if (gcnFile->gameID().left(3) != QLatin1String("GXS")) {
		// Not SADX.
		return nullptr;
	}

	// Create an SAEditor.
	EditorWindow *editor = new EditorWindow();
	SAEditor *saEditor = new SAEditor();
	int ret = saEditor->setFile(gcnFile);
	if (ret != 0) {
		// Editor does not support this file.
		delete saEditor;
		return 0;
	}
	editor->d_func()->setEditorWidget(saEditor);
	return editor;
}

/**
 * Edit a VmuFile.
 * TODO: Combine with editGcnFile.
 * @param vmuFile VmuFile to edit.
 * @return Editor dialog for the VmuFile. (nullptr if the file cannot be edited)
 */
EditorWindow *EditorWindow::editVmuFile(VmuFile *vmuFile)
{
	// TODO: Connect the 'destroyed' signal and
	// close the editor if the file is destroyed.

	// TODO: Check for editors that support this file.
	// For now, only allow SA1.
	if (vmuFile->filename() != QLatin1String("SONICADV_SYS") &&
	    vmuFile->filename() != QLatin1String("SONICADV_INT"))
	{
		// Not SA1.
		return nullptr;
	}

	// Create an SAEditor.
	EditorWindow *editor = new EditorWindow();
	SAEditor *saEditor = new SAEditor();
	int ret = saEditor->setFile(vmuFile);
	if (ret != 0) {
		// Editor does not support this file.
		delete saEditor;
		return 0;
	}
	editor->d_func()->setEditorWidget(saEditor);
	return editor;
}
