/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * SearchDialog.hpp: Search dialog.                                        *
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

#ifndef __MCRECOVER_SEARCHDIALOG_HPP__
#define __MCRECOVER_SEARCHDIALOG_HPP__

#include <QtGui/QDialog>
#include "ui_SearchDialog.h"

// Card definitions.
#include "card.h"

// Qt classes.
class QCloseEvent;

// SearchThread.
class SearchThread;

// SearchDialog private class.
class SearchDialogPrivate;

class SearchDialog : public QDialog, public Ui::SearchDialog
{
	Q_OBJECT

	public:
		SearchDialog(QWidget *parent = 0);
		~SearchDialog();

	private:
		friend class SearchDialogPrivate;
		SearchDialogPrivate *const d;
		Q_DISABLE_COPY(SearchDialog);

	public:
		/**
		 * Get the SearchThread.
		 * @return SearchThread.
		 */
		SearchThread *searchThread(void);

		/**
		 * Set the SearchThread.
		 * @param searchThread SearchThread.
		 */
		void setSearchThread(SearchThread *searchThread);

	private slots:
		/**
		 * User closed the window.
		 * @param event QCloseEvent.
		 */
		void closeEvent(QCloseEvent *event);

		/**
		 * User clicked cancel.
		 */
		void reject(void);

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
};

#endif /* __MCRECOVER_SEARCHDIALOG_HPP__ */
