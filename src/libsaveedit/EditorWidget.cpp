/***************************************************************************
 * GameCube Memory Card Recovery Program [libsaveedit]                     *
 * EditorWidget.cpp: Editor widget base class.                             *
 *                                                                         *
 * Copyright (c) 2015-2025 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#include "EditorWidget.hpp"
#include "EditorWidget_p.hpp"

// C includes. (C++ namespace)
#include <cassert>

/** EditorWidgetPrivate **/

EditorWidgetPrivate::EditorWidgetPrivate(EditorWidget *q)
	: q_ptr(q)
	, file(nullptr)
	, saveSlots(1)
	, generalSettings(false)
	, currentSaveSlot(0)
{ }

EditorWidgetPrivate::~EditorWidgetPrivate()
{
	// Nothing to clean up here...
}

/** Convenience functions for EditorWidget subclasses **/

/**
 * Set the number of save slots.
 * @param saveSlots New number of save slots (Must be at least 1!)
 */
void EditorWidgetPrivate::setSaveSlots(int saveSlots)
{
	assert(saveSlots >= 1);
	if (this->saveSlots == saveSlots)
		return;

	Q_Q(EditorWidget);
	this->saveSlots = saveSlots;
	emit q->saveSlotsChanged(saveSlots);
}

/**
 * Set the "general" settings status.
 * @param hasGeneralSettings True if this editor has a "general" settings section.
 */
void EditorWidgetPrivate::setGeneralSettings(bool generalSettings)
{
	if (this->generalSettings == generalSettings)
		return;

	Q_Q(EditorWidget);
	if (!generalSettings && this->currentSaveSlot < 0) {
		// Currently viewing the "general" settings tab,
		// but we want to get rid of it.
		// Switch to save slot 0.
		q->setCurrentSaveSlot(0);
	}

	this->generalSettings = generalSettings;
	emit q->generalSettingsChanged(generalSettings);
}

/** EditorWidget **/

/**
 * Create an EditorWidget.
 * This EditorWidget is NOT valid by itself, and must be
 * subclassed in order to implement certain functions.
 * @param d EditorWidgetPrivate-derived private class
 * @param parent Parent widget
 */
EditorWidget::EditorWidget(EditorWidgetPrivate *d, QWidget *parent)
	: super(parent)
	, d_ptr(d)
	, m_modified(false)
{
	// No extra initialization is required here...
}

/**
 * Shut down the Editor Widget.
 */
EditorWidget::~EditorWidget()
{
	delete d_ptr;
}

/** Public functions. **/

/**
 * Get the file currently being edited.
 * @return File being edited, or nullptr if none.
 */
File *EditorWidget::file(void) const
{
	Q_D(const EditorWidget);
	return d->file;
}

/**
 * Get the number of save slots available in this editor.
 * @return Number of save slots
 */
int EditorWidget::saveSlots(void) const
{
	Q_D(const EditorWidget);
	return d->saveSlots;
}

/**
 * Does this editor have a "general" settings section in addition to slots?
 * @return True if this editor has a "general" settings section.
 */
bool EditorWidget::hasGeneralSettings(void) const
{
	Q_D(const EditorWidget);
	return d->generalSettings;
}

/**
 * Get the current save slot.
 * @return Current save slot (-1 for "general" settings)
 */
int EditorWidget::currentSaveSlot(void) const
{
	Q_D(const EditorWidget);
	return d->currentSaveSlot;
}

/**
 * Set the current save slot.
 *
 * Subclasses should save their current save slot,
 * call EditorWidget::setCurrentSaveSlot(), and then
 * load the new save slot.
 *
 * The base class function call is needed in order to
 * update internal variables and emit signals.
 *
 * NOTE: The subclass should NOT modify d->currentSaveSlot!
 *
 * @param saveSlot New save slot (-1 for "general" settings)
 * TODO: Return the selected save slot?
 */
void EditorWidget::setCurrentSaveSlot(int saveSlot)
{
       // TODO: Update setFile() to use a similar base class function setup.
       Q_D(EditorWidget);
       if (d->currentSaveSlot == saveSlot)
               return;

       // Save the current save slot and restore it in case
       // the function fails. This is needed because some
       // editors will modify currentSaveSlot in the process
       // of updating.
       int prevSaveSlot = d->currentSaveSlot;
       int ret = setCurrentSaveSlot_int(saveSlot);
       if (ret == 0) {
	       // Save slot set successfully.
		d->currentSaveSlot = saveSlot;
		emit currentSaveSlotChanged(saveSlot);
       } else {
	       // Setting the save slot failed.
	       // TODO: Return an error?
	       d->currentSaveSlot = prevSaveSlot;
       }
}
