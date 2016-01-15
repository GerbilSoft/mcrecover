/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * EditorWidget_p.hpp: Editor widget base class. (PRIVATE)                 *
 *                                                                         *
 * Copyright (c) 2012-2015 by David Korth.                                 *
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

#ifndef __MCRECOVER_EDIT_EDITORWIDGET_P_HPP__
#define __MCRECOVER_EDIT_EDITORWIDGET_P_HPP__

#include "EditorWidget.hpp"

// C includes. (C++ namespace)
#include <cassert>

class EditorWidgetPrivate
{
	public:
		EditorWidgetPrivate(EditorWidget *q);
		virtual ~EditorWidgetPrivate();

	protected:
		EditorWidget *const q_ptr;
		Q_DECLARE_PUBLIC(EditorWidget)
	private:
		Q_DISABLE_COPY(EditorWidgetPrivate)

	public:
		// Opened file.
		File *file;

		// NOTE: Subclasses must emit the appropriate signals
		// when changing these. Use the helper functions.
		int saveSlots;
		bool generalSettings;
		int currentSaveSlot;

		/** Convenience functions for subclasses. **/

		/**
		 * Set the number of save slots.
		 * @param saveSlots New number of save slots. (Must be at least 1!)
		 */
		void setSaveSlots(int saveSlots);

		/**
		 * Set the "general" settings status.
		 * @param hasGeneralSettings True if this editor has a "general" settings section.
		 */
		void setGeneralSettings(bool generalSettings);
};

#endif /* __MCRECOVER_EDIT_EDITORWIDGET_P_HPP__ */
