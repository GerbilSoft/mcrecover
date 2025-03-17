/***************************************************************************
 * GameCube Memory Card Recovery Program [libsaveedit]                     *
 * EditorWindow.cpp: Save file editor window.                              *
 *                                                                         *
 * Copyright (c) 2015-2025 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#include "EditorWindow.hpp"

// Qt includes
#include <QtCore/QEvent>
#include <QtCore/QSignalMapper>
#include <QtCore/QVector>
#include <QAction>
#include <QActionGroup>

// Files
#include "libmemcard/File.hpp"

// EditorWidget
#include "EditorWidget.hpp"
#include "EditorWidgetFactory.hpp"

/** EditorWindowPrivate **/

#include "ui_EditorWindow.h"
class EditorWindowPrivate
{
public:
	explicit EditorWindowPrivate(EditorWindow *q);
	~EditorWindowPrivate();

private:
	EditorWindow *const q_ptr;
	Q_DECLARE_PUBLIC(EditorWindow)
	Q_DISABLE_COPY(EditorWindowPrivate)

public:
	Ui::EditorWindow ui;

	// Editor widget
	EditorWidget *editorWidget;

	// Save slot buttons
	QVector<QAction*> saveSlotButtons;
	QActionGroup *actgrpSaveSlots;
	QSignalMapper *signalMapper;
	QAction *toolBarSeparator;	// CACHED; don't delete this!

	/**
	 * Set the editor widget.
	 * The editor widget will be owned by this EditorWindow.
	 * @param editorWidget Editor widget
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
 * @param editor Editor widget
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
		QObject::connect(editorWidget, &EditorWidget::saveSlotsChanged,
				 q, &EditorWindow::saveSlotsChanged_slot);
		QObject::connect(editorWidget, &EditorWidget::generalSettingsChanged,
				 q, &EditorWindow::generalSettingsChanged_slot);
		QObject::connect(editorWidget, &EditorWidget::currentSaveSlotChanged,
				 q, &EditorWindow::currentSaveSlotChanged_slot);
		QObject::connect(editorWidget, &EditorWidget::destroyed,
				 q, &EditorWindow::editorWidget_destroyed_slot);
		QObject::connect(editorWidget, &EditorWidget::hasBeenModified,
				 q, &EditorWindow::editorWidget_hasBeenModified);

#ifndef NDEBUG
		// DEBUGGING: Make sure we initialize the modified state.
		// Ideally, this should be false on initial load, but due
		// to various widgets emitting signals, this might be true.
		q->editorWidget_hasBeenModified(editorWidget->isModified());
#endif /* NDEBUG */

		// Set the central widget.
		// QMainWindow takes ownership of the widget.
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
	// TODO: Move McRecoverQApplication::IconFromTheme() to a separate library?

	// Set action icons.
	ui.actionSave->setIcon(QIcon::fromTheme(QLatin1String("document-save")));
	// TODO: Actual "reload" icon.
	ui.actionReload->setIcon(QIcon::fromTheme(QLatin1String("edit-undo")));

	// Disable save and reload actions by default.
	// They're enabled if the file is modified.
	ui.actionSave->setEnabled(false);
	ui.actionReload->setEnabled(false);

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
	: super(parent,
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
 * Edit a File.
 * @param file File to edit.
 * @return Editor dialog for the ile. (nullptr if the file cannot be edited)
 */
EditorWindow *EditorWindow::editFile(File *file)
{
	// TODO: Connect the 'destroyed' signal and
	// close the editor if the file is destroyed.

	EditorWidget *editorWidget = EditorWidgetFactory::createWidget(file);
	if (!editorWidget) {
		// No EditorWidget is available for this File.
		return nullptr;
	}

	// Create an EditorWindow for this EditorWidget.
	EditorWindow *editor = new EditorWindow();
	editor->d_func()->setEditorWidget(editorWidget);
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
 * EditorWidget has been modified.
 * @param modified New modified status.
 */
void EditorWindow::editorWidget_hasBeenModified(bool modified)
{
	// Save and Reload buttons are enabled if modified,
	// and disabled if not modified.
	Q_D(EditorWindow);
	d->ui.actionSave->setEnabled(modified);
	d->ui.actionReload->setEnabled(modified);
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

/**
 * "Save" button was clicked.
 */
void EditorWindow::on_actionSave_triggered(void)
{
	Q_D(EditorWindow);
	assert(d->editorWidget);
	// TODO: Show errors?
	d->editorWidget->save();
}

/**
 * "Save" button was clicked.
 */
void EditorWindow::on_actionReload_triggered(void)
{
	// TODO: Prompt if the user really wants to reload the save data.
	Q_D(EditorWindow);
	assert(d->editorWidget);
	d->editorWidget->reload();
}
