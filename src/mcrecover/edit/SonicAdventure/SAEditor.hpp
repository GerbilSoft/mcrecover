/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * SAEditor.hpp: Sonic Adventure - save file editor.                       *
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

#ifndef __MCRECOVER_EDIT_SONICADVENTURE_SONICADVENTUREEDITOR_HPP__
#define __MCRECOVER_EDIT_SONICADVENTURE_SONICADVENTUREEDITOR_HPP__

#include "../EditorWidget.hpp"

class File;

class SAEditorPrivate;
class SAEditor : public EditorWidget
{
	Q_OBJECT
	typedef EditorWidget super;

	public:
		explicit SAEditor(QWidget *parent = nullptr);
		virtual ~SAEditor();

	protected:
		Q_DECLARE_PRIVATE(SAEditor)
	private:
		Q_DISABLE_COPY(SAEditor)

	protected:
		// State change event. (Used for switching the UI language at runtime.)
		void changeEvent(QEvent *event);

	public:
		/** Public static functions. **/

		/**
		 * Is the specified file supported by this editor?
		 * @return True if supported; false if not.
		 */
		static bool isFileSupported(const File *file);

	public:
		/** Public functions. **/

		/**
		 * Set the File to edit.
		 *
		 * @param file File to edit.
		 * If the file isn't valid, it won't be set;
		 * check file() afterwards to verify.
		 *
		 * @return 0 on success; non-zero on error (and file will not be set).
		 * TODO: Error code constants?
		 */
		virtual int setFile(File *file) final;

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
		virtual int setCurrentSaveSlot_int(int saveSlot) final;
};

#endif /* __MCRECOVER_EDIT_SONICADVENTURE_SONICADVENTUREEDITOR_HPP__ */
