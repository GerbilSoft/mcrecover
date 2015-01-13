/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * SAEditor.hpp: Sonic Adventure - save file editor.           *
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

#ifndef __MCRECOVER_EDIT_SONICADVENTURE_SONICADVENTUREEDITOR_HPP__
#define __MCRECOVER_EDIT_SONICADVENTURE_SONICADVENTUREEDITOR_HPP__

#include <QtGui/QDialog>

class File;

class SAEditorPrivate;
class SAEditor : public QWidget
{
	Q_OBJECT

	Q_PROPERTY(File* file READ file WRITE setFile)

	public:
		SAEditor(QWidget *parent = nullptr);
		~SAEditor();

	protected:
		SAEditorPrivate *const d_ptr;
		Q_DECLARE_PRIVATE(SAEditor)
	private:
		Q_DISABLE_COPY(SAEditor)

	protected:
		// State change event. (Used for switching the UI language at runtime.)
		void changeEvent(QEvent *event);

	/** Public functions. **/

	public:
		/**
		 * Get the file currently being edited.
		 * @return File being edited, or nullptr if none.
		 */
		File *file(void) const;

		/**
		 * Set the File to edit.
		 * @param file File to edit.
		 * If the file isn't valid, it won't be set;
		 * check file() afterwards to verify.
		 */
		int setFile(File *file);
};

#endif /* __MCRECOVER_EDIT_SONICADVENTURE_SONICADVENTUREEDITOR_HPP__ */
