/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * EditorWidget.hpp: Editor widget base class.                             *
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

#ifndef __MCRECOVER_EDIT_EDITORWIDGET_HPP__
#define __MCRECOVER_EDIT_EDITORWIDGET_HPP__

// C includes. (C++ namespace)
#include <cassert>

// Qt includes.
#include <QWidget>

// TODO: Namespace?
class File;

class EditorWidgetPrivate;
class EditorWidget : public QWidget
{
	Q_OBJECT
	typedef QWidget super;

	// TODO: NOTIFY signal for file?
	Q_PROPERTY(File* file READ file WRITE setFile /*NOTIFY fileChanged*/)
	Q_PROPERTY(int currentSaveSlot READ currentSaveSlot WRITE setCurrentSaveSlot NOTIFY currentSaveSlotChanged);
	Q_PROPERTY(int saveSlots READ saveSlots NOTIFY saveSlotsChanged)
	Q_PROPERTY(bool generalSettings READ hasGeneralSettings NOTIFY generalSettingsChanged)

	protected:
		/**
		 * Create an EditorWidget.
		 * This EditorWidget is NOT valid by itself, and must be
		 * subclassed in order to implement certain functions.
		 * @param d EditorWidgetPrivate-derived private class.
		 * @param parent Parent widget.
		 */
		EditorWidget(EditorWidgetPrivate *d, QWidget *parent = nullptr);
	public:
		virtual ~EditorWidget();

	protected:
		EditorWidgetPrivate *const d_ptr;
		Q_DECLARE_PRIVATE(EditorWidget)
	private:
		Q_DISABLE_COPY(EditorWidget)

	public:
		/** Public functions. **/

		/**
		 * Get the file currently being edited.
		 * @return File being edited, or nullptr if none.
		 */
		File *file(void) const;

		/**
		 * Set the File to edit.
		 * This function MUST be overridden by subclasses.
		 *
		 * @param file File to edit.
		 * If the file isn't valid, it won't be set;
		 * check file() afterwards to verify.
		 *
		 * @return 0 on success; non-zero on error (and file will not be set).
		 * TODO: Error code constants?
		 */
		virtual int setFile(File *file) = 0;

		// TODO: "Save" functions.

		/**
		 * Get the number of save slots available in this editor.
		 * @return Number of save slots.
		 */
		int saveSlots(void) const;

		/**
		 * Does this editor have a "general" settings section in addition to slots?
		 * @return True if this editor has a "general" settings section.
		 */
		bool hasGeneralSettings(void) const;

		/**
		 * Get the current save slot.
		 * @return Current save slot. (-1 for "general" settings)
		 */
		int currentSaveSlot(void) const;

	public slots:
		/**
		 * Set the current save slot.
		 * @param saveSlot New save slot. (-1 for "general" settings)
		 * TODO: Return the selected save slot?
		 */
		void setCurrentSaveSlot(int saveSlot);

	protected:
		/**
		 * Set the current save slot. [INTERNAL FUNCTION]
		 *
		 * This is called by the base class when the
		 * setCurrentSaveSlot() function is called.
		 * Subclasses should load the appropriate data
		 * from the specified save slot.
		 *
		 * If an error occurs, the save slot will not be changed.
		 *
		 * @return 0 on success; non-zero on error.
		 */
		virtual int setCurrentSaveSlot_int(int saveSlot) = 0;

	signals:
		/**
		 * Number of save slots has changed.
		 * @param saveSlots New number of save slots.
		 */
		void saveSlotsChanged(int saveSlots);

		/**
		 * Status of the "general" settings section has changed.
		 * @param hasGeneralSave True if this editor has a "general" settings section.
		 */
		void generalSettingsChanged(bool generalSettings);

		/**
		 * Current save slot has changed.
		 * @param saveSlot New save slot. (-1 for "general" settings)
		 */
		void currentSaveSlotChanged(int saveSlot);
};

#endif /* __MCRECOVER_EDIT_EDITORWIDGET_HPP__ */
