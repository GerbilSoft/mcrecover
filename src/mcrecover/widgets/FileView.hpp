/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * FileView.hpp: File view widget.                                         *
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

#ifndef __MCRECOVER_WIDGETS_FILEVIEW_HPP__
#define __MCRECOVER_WIDGETS_FILEVIEW_HPP__

#include <QtGui/QWidget>

class File;

class FileViewPrivate;
class FileView : public QWidget
{
	Q_OBJECT

	Q_PROPERTY(const File* file READ file WRITE setFile)

	public:
		FileView(QWidget *parent = 0);
		~FileView();

	protected:
		FileViewPrivate *const d_ptr;
		Q_DECLARE_PRIVATE(FileView)
	private:
		Q_DISABLE_COPY(FileView)

	public:
		/**
		 * Get the File being displayed.
		 * @return File.
		 */
		const File *file(void) const;

		/**
		 * Set the File being displayed.
		 * @param file File.
		 */
		void setFile(const File *file);

	protected:
		// State change event. (Used for switching the UI language at runtime.)
		void changeEvent(QEvent *event);

	public slots:
		/**
		 * Pause animation.
		 * Should be used if e.g. the window is minimized.
		 * NOTE: This uses an internal counter; the number of resumes
		 * must match the number of pauses to resume animation.
		 */
		void pauseAnimation(void);

		/**
		 * Resume animation.
		 * Should be used if e.g. the window is un-minimized.
		 * NOTE: This uses an internal counter; the number of resumes
		 * must match the number of pauses to resume animation.
		 */
		void resumeAnimation(void);

	protected slots:
		/**
		 * File object was destroyed.
		 * @param obj QObject that was destroyed.
		 */
		void file_destroyed_slot(QObject *obj = 0);

		/**
		 * Animation timer slot.
		 */
		void animTimer_slot(void);

		/**
		 * XML button was pressed.
		 */
		void on_btnXML_clicked(void);

		/**
		 * Edit button was pressed.
		 */
		void on_btnEdit_clicked(void);
};

#endif /* __MCRECOVER_WIDGETS_FILEVIEW_HPP__ */
