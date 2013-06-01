/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * McRecoverWindow.hpp: Main window.                                       *
 *                                                                         *
 * Copyright (c) 2012-2013 by David Korth.                                 *
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

#ifndef __MCRECOVER_MCRECOVERWINDOW_HPP__
#define __MCRECOVER_MCRECOVERWINDOW_HPP__

#include <QtGui/QMainWindow>
#include "ui_McRecoverWindow.h"

class McRecoverWindowPrivate;

class McRecoverWindow : public QMainWindow, public Ui::McRecoverWindow
{
	Q_OBJECT
	
	public:
		McRecoverWindow(QWidget *parent = 0);
		~McRecoverWindow();

	private:
		friend class McRecoverWindowPrivate;
		McRecoverWindowPrivate *const d;
		Q_DISABLE_COPY(McRecoverWindow);

	public:
		/**
		 * Open a GameCube Memory Card file.
		 * @param filename Filename.
		 */
		void open(QString filename);

	protected:
		// QMainWindow virtual functions: drag and drop.
		void dragEnterEvent(QDragEnterEvent *event);
		void dropEvent(QDropEvent *event);

	protected slots:
		// Widget slots.
		void on_btnSearchLostFiles_clicked(void);
		void on_btnLoadDatabase_clicked(void);

		// MemCardModel slots.
		void memCardModel_layoutChanged(void);
		void memCardModel_rowsInserted(void);
};

#endif /* __MCRECOVER_MCRECOVERWINDOW_HPP__ */
