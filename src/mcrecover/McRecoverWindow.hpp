/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * McRecoverWindow.hpp: Main window.                                       *
 *                                                                         *
 * Copyright (c) 2012-2014 by David Korth.                                 *
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

// Qt includes.
#include <QtCore/QString>
#include <QtCore/QVector>
#include <QtCore/QModelIndex>

// MemCard Recover classes.
class MemCardFile;

class McRecoverWindowPrivate;

class McRecoverWindow : public QMainWindow
{
	Q_OBJECT
	
	public:
		McRecoverWindow(QWidget *parent = 0);
		~McRecoverWindow();

	protected:
		McRecoverWindowPrivate *const d_ptr;
		Q_DECLARE_PRIVATE(McRecoverWindow)
	private:
		Q_DISABLE_COPY(McRecoverWindow)

	public:
		/**
		 * Open a GameCube Memory Card image.
		 * @param filename Filename.
		 */
		void openCard(QString filename);

		/**
		 * Close the currently-opened GameCube Memory Card image.
		 */
		void closeCard(void);

	protected:
		// State change event. (Used for switching the UI language at runtime.)
		void changeEvent(QEvent *event);

		// QMainWindow virtual functions: drag and drop.
		void dragEnterEvent(QDragEnterEvent *event);
		void dropEvent(QDropEvent *event);

	protected slots:
		// UI busy functions.
		void markUiBusy(void);
		void markUiNotBusy(void);

	protected slots:
		// Actions.
		void on_actionOpen_triggered(void);
		void on_actionClose_triggered(void);
		void on_actionScan_triggered(void);
		void on_actionExit_triggered(void);
		void on_actionAbout_triggered(void);

		// Save actions.
		void on_actionSave_triggered(void);
		void on_actionSaveAll_triggered(void);

		/**
		 * Set the preferred region.
		 * This slot is triggered by a QSignalMapper that
		 * maps the various QActions.
		 * @param preferredRegion Preferred region. (actually char)
		 */
		void setPreferredRegion_slot(int preferredRegion);

		/**
		 * Set the animated icon format.
		 * This slot is triggered by a QSignalMapper that
		 * maps the various QActions.
		 * @param animIconFormat Animated icon format.
		 */
		void setAnimIconFormat_slot(int animIconFormat);

		// MemCardModel slots.
		void memCardModel_layoutChanged(void);
		void memCardModel_rowsInserted(void);

		// SearchThread has finished.
		void searchThread_searchFinished_slot(int lostFilesFound);

		// lstFileList slots.
		void lstFileList_selectionModel_currentRowChanged(const QModelIndex& current, const QModelIndex& previous);

		/**
		 * Set the translation.
		 * @param tsLocale Translation to use. (locale tag)
		 */
		void setTranslation_slot(const QString &tsLocale);
};

#endif /* __MCRECOVER_MCRECOVERWINDOW_HPP__ */
