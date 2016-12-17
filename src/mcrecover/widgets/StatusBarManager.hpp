/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * StatusBarManager.hpp: Status Bar manager                                *
 *                                                                         *
 * Copyright (c) 2013-2016 by David Korth.                                 *
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

#ifndef __MCRECOVER_WIDGETS_STATUSBARMANAGER_HPP__
#define __MCRECOVER_WIDGETS_STATUSBARMANAGER_HPP__

// Qt includes and classes.
#include <QtCore/QObject>
class QStatusBar;

// Card definitions.
#include "card.h"

class GcnSearchThread;
class TaskbarButtonManager;

class StatusBarManagerPrivate;
class StatusBarManager : public QObject
{
	Q_OBJECT
	typedef QObject super;

	Q_PROPERTY(QStatusBar* statusBar READ statusBar WRITE setStatusBar)
	Q_PROPERTY(GcnSearchThread* searchThread READ searchThread WRITE setSearchThread)

	public:
		explicit StatusBarManager(QObject *parent = 0);
		explicit StatusBarManager(QStatusBar *statusBar, QObject *parent = 0);
		virtual ~StatusBarManager();

	protected:
		StatusBarManagerPrivate *const d_ptr;
		Q_DECLARE_PRIVATE(StatusBarManager)
	private:
		Q_DISABLE_COPY(StatusBarManager)

	public:
		/**
		 * Get the QStatusBar.
		 * @return QStatusBar.
		 */
		QStatusBar *statusBar(void) const;

		/**
		 * Set the QStatusBar.
		 * @param statusBar QStatusBar.
		 */
		void setStatusBar(QStatusBar *statusBar);

		/**
		 * Get the GcnSearchThread.
		 * @return GcnSearchThread.
		 */
		GcnSearchThread *searchThread(void) const;

		/**
		 * Set the GcnSearchThread.
		 * @param searchThread GcnSearchThread.
		 */
		void setSearchThread(GcnSearchThread *searchThread);

		/**
		 * Get the TaskbarButtonManager.
		 * @return TaskbarButtonManager.
		 */
		TaskbarButtonManager *taskbarButtonManager(void) const;

		/**
		 * Set the TaskbarButtonManager.
		 * @param taskbarButtonManager TaskbarButtonManager.
		 */
		void setTaskbarButtonManager(TaskbarButtonManager *taskbarButtonManager);

	public slots:
		/**
		 * A memory card image was opened.
		 * @param filename Filename.
		 * @param productName Product name of the memory card.
		 */
		void opened(const QString &filename, const QString &productName);

		/**
		 * The current memory card image was closed.
		 * @param productName Product name of the memory card.
		 */
		void closed(const QString &productName);

		/**
		 * Files were saved.
		 * @param n Number of files saved.
		 * @param path Path files were saved to.
		 */
		void filesSaved(int n, const QString &path);

	private slots:
		/**
		 * An object has been destroyed.
		 * @param obj QObject that was destroyed.
		 */
		void object_destroyed_slot(QObject *obj = 0);

		/**
		 * Search has started.
		 * @param totalPhysBlocks Total number of blocks in the card.
		 * @param totalSearchBlocks Number of blocks being searched.
		 * @param firstPhysBlock First block being searched.
		 */
		void searchStarted_slot(int totalPhysBlocks, int totalSearchBlocks, int firstPhysBlock);

		/**
		 * Search has been cancelled.
		 */
		void searchCancelled_slot(void);

		/**
		 * Search has completed.
		 * @param lostFilesFound Number of "lost" files found.
		 */
		void searchFinished_slot(int lostFilesFound);

		/**
		 * Update search status.
		 * @param currentPhysBlock Current physical block number being searched.
		 * @param currentSearchBlock Number of blocks searched so far.
		 * @param lostFilesFound Number of "lost" files found.
		 */
		void searchUpdate_slot(int currentPhysBlock, int currentSearchBlock, int lostFilesFound);

		/**
		 * An error has occurred during the search.
		 * @param errorString Error string.
		 */
		void searchError_slot(QString errorString);

		/**
		 * Hide the progress bar.
		 * This is usually done a few seconds after the
		 * search is completed.
		 */
		void hideProgressBar_slot(void);
};

#endif /* __MCRECOVER_WIDGETS_STATUSBARMANAGER_HPP__ */
