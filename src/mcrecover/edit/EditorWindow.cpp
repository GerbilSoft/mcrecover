/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * EditorWindow.cpp: Save file editor window.                              *
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

#include "EditorWindow.hpp"
#include "McRecoverQApplication.hpp"

// Qt includes.
#include <QtCore/QEvent>
#include <QtCore/QSignalMapper>
#include <QtCore/QVector>
#include <QtGui/QAction>

// Files.
#include "../card/GcnFile.hpp"
#include "../card/VmuFile.hpp"

// Editors.
// TODO: EditorWidgetFactory?
#include "EditorWidget.hpp"
#include "SonicAdventure/SAEditor.hpp"

/** EditorWindowPrivate **/

#include "ui_EditorWindow.h"
class EditorWindowPrivate
{
	public:
		EditorWindowPrivate(EditorWindow *q);
		~EditorWindowPrivate();

	protected:
		EditorWindow *const q_ptr;
		Q_DECLARE_PUBLIC(EditorWindow)
	private:
		Q_DISABLE_COPY(EditorWindowPrivate)

	public:
		Ui::EditorWindow ui;

		// Editor widget.
		EditorWidget *editorWidget;

		// Save slot buttons.
		QVector<QAction*> saveSlotButtons;
		QActionGroup *actgrpSaveSlots;
		QSignalMapper *signalMapper;
		QAction *toolBarSeparator;	// CACHED; don't delete this!

		/**
		 * Set the editor widget.
		 * The editor widget will be owned by this EditorWindow.
		 * @param editorWidget Editor widget.
		 */
		void setEditorWidget(EditorWidget *editorWidget);

		/**
		 * Initialize the toolbar.
		 */
		void initToolbar(void);

		/**
		 * Update the save slot buttons.
		 */
		void updateSaveSlotButtons(void);
};

EditorWindowPrivate::EditorWindowPrivate(EditorWindow* q)
	: q_ptr(q)
	, editorWidget(nullptr)
	, actgrpSaveSlots(new QActionGroup(q))
	, signalMapper(new QSignalMapper(q))
	, toolBarSeparator(nullptr)
{
	// Connect the save slot button signal mapper.
	QObject::connect(signalMapper, SIGNAL(mapped(int)),
			 q, SLOT(toolBar_saveSlotButton_clicked(int)));
}

EditorWindowPrivate::~EditorWindowPrivate()
{
	// TODO: Do we really need to delete these here?
	// Qt should handle it automatically...
	delete actgrpSaveSlots;
	delete signalMapper;
	qDeleteAll(saveSlotButtons);
	saveSlotButtons.clear();
}

/**
 * Set the editor widget.
 * This EditorWindow will take ownership of the EditorWidget.
 * @param editor Editor widget.
 */
void EditorWindowPrivate::setEditorWidget(EditorWidget *editorWidget)
{
	if (this->editorWidget == editorWidget)
		return;

	// Delete the existing editor widget.
	delete this->editorWidget;

	// Add the new editor widget.
	this->editorWidget = editorWidget;
	if (editorWidget != nullptr) {
		// Connect various signals.
		Q_Q(EditorWindow);
		QObject::connect(editorWidget, SIGNAL(saveSlotsChanged(int)),
				 q, SLOT(saveSlotsChanged_slot(int)));
		QObject::connect(editorWidget, SIGNAL(generalSettingsChanged(bool)),
				 q, SLOT(generalSettingsChanged_slot(bool)));
		QObject::connect(editorWidget, SIGNAL(currentSaveSlotChanged(int)),
				 q, SLOT(currentSaveSlotChanged_slot(int)));
		QObject::connect(editorWidget, SIGNAL(destroyed(QObject*)),
				 q, SLOT(editorWidget_destroyed_slot(QObject*)));

		// Set the central widget.
		q->setCentralWidget(editorWidget);

		// Update the save slot buttons.
		updateSaveSlotButtons();
	}
}

/**
 * Initialize the toolbar.
 */
void EditorWindowPrivate::initToolbar(void)
{
	// Set action icons.
	ui.actionSave->setIcon(
		McRecoverQApplication::IconFromTheme(QLatin1String("document-save")));
	// TODO: Actual "reload" icon.
	ui.actionReload->setIcon(
		McRecoverQApplication::IconFromTheme(QLatin1String("edit-undo")));

	// Disable save actions by default.
	ui.actionSave->setEnabled(false);

	// Cache the separator action.
	QList<QAction*> actions = ui.toolBar->actions();
	if (!actions.isEmpty()) {
		// TODO: Connect the destroyed() signal?
		toolBarSeparator = actions.at(actions.size() - 1);
	}

	// Connect the "General" settings button to the signal mapper.
	signalMapper->setMapping(ui.actionGeneralSettings, -1);
	QObject::connect(ui.actionGeneralSettings, SIGNAL(triggered()),
			 signalMapper, SLOT(map()));

	// Update the save slot buttons.
	updateSaveSlotButtons();
}

/**
 * Update the save slot buttons.
 */
void EditorWindowPrivate::updateSaveSlotButtons(void)
{
	// Add/remove save slot buttons, if necessary.
	const int saveSlots = (editorWidget ? editorWidget->saveSlots() : 0);
	if (!editorWidget ||
		(saveSlots == 1 && !editorWidget->hasGeneralSettings()))
	{
		// Special case: Single slot, no general settings.
		// (Or, we don't have an EditorWidget.)

		// Remove all buttons and hide the separator.
		// NOTE: For save files that are "only" general settings,
		// the editor should indicate 1 slot and no general settings.
		// TODO: Test this!
		ui.actionGeneralSettings->setVisible(false);
		qDeleteAll(saveSlotButtons);
		saveSlotButtons.clear();

		// Hide the separator.
		toolBarSeparator->setVisible(false);
		return;
	}

	// Make sure the separator is visible.
	toolBarSeparator->setVisible(true);

	if (saveSlots > saveSlotButtons.size()) {
		// Add buttons.
		Q_Q(EditorWindow);
		for (int i = saveSlotButtons.size(); i < saveSlots; i++) {
			QAction *action = new QAction(QString::number(i+1), q);
			action->setToolTip(EditorWidget::tr("Save Slot %1").arg(i+1));
			action->setCheckable(true);
			saveSlotButtons.append(action);
			actgrpSaveSlots->addAction(action);
			ui.toolBar->addAction(action);
			// Connect the signal mapper.
			signalMapper->setMapping(action, i);
			QObject::connect(action, SIGNAL(triggered()),
						signalMapper, SLOT(map()));
		}
	} else if (saveSlots < saveSlotButtons.size()) {
		// Remove buttons.
		for (int i = saveSlotButtons.size() - 1; i >= saveSlots; i--) {
			delete saveSlotButtons.at(i);
		}
		saveSlotButtons.resize(saveSlots);
	}

	// NOTE: We can't easily insert a QAction by index, so we'll
	// just hide/show the action when necessary.
	const bool generalSettings = editorWidget->hasGeneralSettings();
	ui.actionGeneralSettings->setVisible(generalSettings);

	// Check if the current save slot is invalid.
	const int currentSaveSlot = editorWidget->currentSaveSlot();
	if (currentSaveSlot >= saveSlotButtons.size()) {
		// Previous save slot no longer exists.
		// Select the next highest save slot.
		editorWidget->setCurrentSaveSlot(saveSlotButtons.size() - 1);
	} else if (currentSaveSlot < 0 && !generalSettings) {
		// Previous save slot was "General" settings,
		// but it was disabled. Select slot 0.
		editorWidget->setCurrentSaveSlot(0);
	} else {
		// Save slot is valid. Simply update the UI.
		if (currentSaveSlot < 0) {
			ui.actionGeneralSettings->setChecked(true);
		} else {
			saveSlotButtons.at(currentSaveSlot)->setChecked(true);
		}
	}
}

/** EditorWindow **/

/**
 * Initialize the save file editor.
 * @param parent Parent widget.
 */
EditorWindow::EditorWindow(QWidget *parent)
	: QMainWindow(parent,
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

	// Initialize the UI.
	d->initToolbar();

	// FIXME: setFile() needs to update the status of the "Save" button,
	// depending on whether or not the file is read-only.
	// Also, connect the readOnlyChanged() signal.
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
	super::changeEvent(event);
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

/** Widget slots. **/

/**
 * Number of save slots has changed.
 * @param saveSlots New number of save slots.
 */
void EditorWindow::saveSlotsChanged_slot(int saveSlots)
{
	Q_UNUSED(saveSlots)

	// TODO: Just update save slot buttons instead of the whole thing?
	Q_D(EditorWindow);
	d->updateSaveSlotButtons();
}

/**
 * Status of the "general" settings section has changed.
 * @param hasGeneralSave True if this editor has a "general" settings section.
 */
void EditorWindow::generalSettingsChanged_slot(bool generalSettings)
{
	Q_UNUSED(generalSettings)

	// TODO: Just update actionGeneralSettings instead of the whole thing?
	Q_D(EditorWindow);
	d->updateSaveSlotButtons();
}

/**
 * Current save slot has changed.
 * @param saveSlot New save slot. (-1 for "general" settings)
 */
void EditorWindow::currentSaveSlotChanged_slot(int saveSlot)
{
	Q_UNUSED(saveSlot)

	// Update the UI.
	// TODO: Validate the save slot number?
	Q_D(EditorWindow);
	assert(saveSlot >= -1 && saveSlot < d->saveSlotButtons.size());
	if (saveSlot < 0) {
		d->ui.actionGeneralSettings->setChecked(true);
	} else {
		d->saveSlotButtons.at(saveSlot)->setChecked(true);
	}
}

/**
 * EditorWidget has been destroyed.
 * @param obj QObject that was destroyed.
 */
void EditorWindow::editorWidget_destroyed_slot(QObject *obj)
{
	Q_D(EditorWindow);
	if (obj == d->editorWidget) {
		// Our EditorWidget was destroyed.
		// TODO: Disable the save/reload buttons?
		d->editorWidget = nullptr;
	}
}

/**
 * A save slot button on the toolbar was clicked.
 * @param saveSlot Save slot number. (-1 for "general" settings)
 */
void EditorWindow::toolBar_saveSlotButton_clicked(int saveSlot)
{
	Q_D(EditorWindow);
	assert(d->editorWidget);
	// TODO: More error checking?
	d->editorWidget->setCurrentSaveSlot(saveSlot);
}
