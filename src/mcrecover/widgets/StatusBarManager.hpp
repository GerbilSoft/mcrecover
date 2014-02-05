/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * StatusBarManager.hpp: Status Bar manager                                *
 *                                                                         *
 * Copyright (c) 2013 by David Korth.                                      *
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

#ifndef __MCRECOVER_STATUSBARMANAGER_HPP__
#define __MCRECOVER_STATUSBARMANAGER_HPP__

// Qt includes and classes.
#include <QtCore/QObject>
class QStatusBar;

// Card definitions.
#include "card.h"

// SearchThread.
class SearchThread;

// StatusBarManager private class.
class StatusBarManagerPrivate;

class StatusBarManager : public QObject
{
	Q_OBJECT

	Q_PROPERTY(QStatusBar* statusBar READ statusBar WRITE setStatusBar)
	Q_PROPERTY(SearchThread* searchThread READ searchThread WRITE setSearchThread)

	public:
		StatusBarManager(QObject *parent = 0);
		explicit StatusBarManager(QStatusBar *statusBar, QObject *parent = 0);
		~StatusBarManager();

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
		 * Get the SearchThread.
		 * @return SearchThread.
		 */
		SearchThread *searchThread(void) const;

		/**
		 * Set the SearchThread.
		 * @param searchThread SearchThread.
		 */
		void setSearchThread(SearchThread *searchThread);

	public slots:
		/**
		 * A GameCube Memory Card image was opened.
		 * @param filename Filename.
		 */
		void opened(const QString &filename);

		/**
		 * The current GameCube Memory Card image was closed.
		 */
		void closed(void);

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

#endif /* __MCRECOVER_STATUSBARMANAGER_HPP__ */
