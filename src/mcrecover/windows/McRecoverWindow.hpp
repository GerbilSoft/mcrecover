/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * McRecoverWindow.hpp: Main window.                                       *
 *                                                                         *
 * Copyright (c) 2012-2016 by David Korth.                                 *
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

#include <QMainWindow>

// Qt includes.
#include <QtCore/QString>
#include <QItemSelection>

// MemCard Recover classes.
class MemCardFile;

class McRecoverWindowPrivate;
class McRecoverWindow : public QMainWindow
{
	Q_OBJECT
	typedef QMainWindow super;
	
	public:
		McRecoverWindow(QWidget *parent = 0);
		virtual ~McRecoverWindow();

	protected:
		McRecoverWindowPrivate *const d_ptr;
		Q_DECLARE_PRIVATE(McRecoverWindow)
	private:
		Q_DISABLE_COPY(McRecoverWindow)

	public:
		/**
		 * Open a GameCube Memory Card image.
		 * @param filename Filename.
		 * @param type Type hint from the Open dialog. (0 == GCN, 1 == VMS, other == unknown)
		 */
		void openCard(const QString &filename, int type = -1);

		/**
		 * Close the currently-opened GameCube Memory Card image.
		 * @param noMsg If true, don't show a message in the status bar.
		 */
		void closeCard(bool noMsg = false);

	protected:
		// State change event. (Used for switching the UI language at runtime.)
		virtual void changeEvent(QEvent *event) final;

		// QMainWindow virtual functions: drag and drop.
		virtual void dragEnterEvent(QDragEnterEvent *event) final;
		virtual void dropEvent(QDropEvent *event) final;

#ifdef Q_OS_WIN
		// Windows message handler. Used for TaskbarButtonManager.
#if QT_VERSION >= 0x050000
		virtual bool nativeEvent(const QByteArray &eventType, void *message, long *result) override;
#else /* QT_VERSION < 0x050000 */
		virtual bool winEvent(MSG *message, long *result) override;
#endif
#endif /* Q_OS_WIN */

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
		 * Set the preferred region.
		 * This version is used by ConfigDefaults.
		 * @param preferredRegion Preferred region. (actually char)
		 */
		void setPreferredRegion_slot(const QVariant &preferredRegion);

		// MemCardModel slots.
		void memCardModel_layoutChanged(void);
		void memCardModel_rowsInserted(void);

		// SearchThread has finished.
		void searchThread_searchFinished_slot(int lostFilesFound);

		// lstFileList slots.
		void lstFileList_selectionModel_selectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

		/**
		 * Set the animated icon format.
		 * This slot is triggered by a QSignalMapper that
		 * maps the various QActions.
		 * @param animIconFormat Animated icon format.
		 */
		void setAnimIconFormat_slot(int animIconFormat);

		/**
		 * UI language was changed by the configuration.
		 * @param animIconFormat Animated icon format.
		 */
		void setAnimIconFormat_cfg_slot(const QVariant &animIconFormat);

		/**
		 * UI language was changed by the user.
		 * @param tsLocale Translation to use. (locale tag)
		 */
		void setTranslation_slot(const QString &tsLocale);

		/**
		 * UI language was changed by the configuration.
		 * @param tsLocale Translation to use. (locale tag)
		 */
		void setTranslation_cfg_slot(const QVariant &tsLocale);

		/**
		 * "Search Used Blocks" was changed by the user.
		 * @param checked True if checked; false if not.
		 */
		void on_actionSearchUsedBlocks_triggered(bool checked);

		/**
		 * "Search Used Blocks" was changed by the configuration.
		 * @param checked True if checked; false if not.
		 */
		void searchUsedBlocks_cfg_slot(const QVariant &checked);
};

#endif /* __MCRECOVER_MCRECOVERWINDOW_HPP__ */
