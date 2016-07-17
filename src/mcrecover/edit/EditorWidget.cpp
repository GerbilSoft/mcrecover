/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * EditorWidget.cpp: Editor widget base class.                             *
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

/** Convenience functions for EditorWidget subclasses. **/

/**
 * Set the number of save slots.
 * @param saveSlots New number of save slots. (Must be at least 1!)
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
 * @param d EditorWidgetPrivate-derived private class.
 * @param parent Parent widget.
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
 * @return Number of save slots.
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
 * @return Current save slot. (-1 for "general" settings)
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
 * @param saveSlot New save slot. (-1 for "general" settings)
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
