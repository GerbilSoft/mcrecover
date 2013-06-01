/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * SearchDialog.cpp: Search dialog.                                        *
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

#include "SearchDialog.hpp"

// Search Thread.
#include "SearchThread.hpp"

// Qt includes.
#include <QtGui/QCloseEvent>


/** SearchDialogPrivate **/

class SearchDialogPrivate
{
	public:
		SearchDialogPrivate(SearchDialog *q);
		~SearchDialogPrivate();

	private:
		SearchDialog *const q;
		Q_DISABLE_COPY(SearchDialogPrivate);

	public:
		// Search thread.
		SearchThread *searchThread;

		/**
		 * Update the search status.
		 */
		void updateSearchStatus(void);

		// Search status from last SearchThread update.
		int currentPhysBlock;
		int totalPhysBlocks;
		int currentSearchBlock;
		int totalSearchBlocks;
		int lostFilesFound;
};

SearchDialogPrivate::SearchDialogPrivate(SearchDialog *q)
	: q(q)
	, searchThread(NULL)
	, currentPhysBlock(0)
	, totalPhysBlocks(0)
	, currentSearchBlock(0)
	, totalSearchBlocks(0)
	, lostFilesFound(0)
{ }

SearchDialogPrivate::~SearchDialogPrivate()
{ }


/**
 * Update the search status.
 */
void SearchDialogPrivate::updateSearchStatus(void)
{
	// Update the "scanning" label.
	QString scanText = q->tr("Scanning block #%L1 (%L2 scanned, %L3 remaining)")
				.arg(currentPhysBlock)
				.arg(currentSearchBlock)
				.arg(totalSearchBlocks - currentSearchBlock);
	q->lblScanningBlocks->setText(scanText);

	// Update the "files found" label.
	QString filesFoundText = q->tr("%n file(s) found.", NULL, lostFilesFound);
	q->lblFilesFound->setText(filesFoundText);

	// Update the progress bar.
	q->pbProgress->setMaximum(totalSearchBlocks);
	q->pbProgress->setValue(currentSearchBlock);
}


/** SearchDialog **/

SearchDialog::SearchDialog(QWidget *parent)
	: QDialog(parent,
		Qt::CustomizeWindowHint |
		Qt::WindowTitleHint |
		Qt::WindowCloseButtonHint)
	, d(new SearchDialogPrivate(this))
{
	setupUi(this);

#ifdef Q_WS_MAC
	// Remove the window icon. (Mac "proxy icon")
	// TODO: Use the memory card file?
	this->setWindowIcon(QIcon());
#endif /* Q_WS_MAC */

	// Update the search status.
	d->updateSearchStatus();
}

SearchDialog::~SearchDialog()
{
	delete d;
}


/**
 * Get the SearchThread.
 * @return SearchThread.
 */
SearchThread *SearchDialog::searchThread(void)
	{ return d->searchThread; }

/**
 * Set the SearchThread.
 * @param searchThread SearchThread.
 */
void SearchDialog::setSearchThread(SearchThread *searchThread)
{
	// TODO: Get current status from the new searchThread.

	if (d->searchThread) {
		// Disconnect signals from the current searchThread.
		disconnect(d->searchThread, SIGNAL(searchStarted(int,int,int)),
			   this, SLOT(searchStarted_slot(int,int,int)));
		disconnect(d->searchThread, SIGNAL(searchCancelled()),
			   this, SLOT(searchCancelled_slot()));
		disconnect(d->searchThread, SIGNAL(searchFinished(int)),
			   this, SLOT(searchFinished_slot(int)));
		disconnect(d->searchThread, SIGNAL(searchUpdate(int,int,int)),
			   this, SLOT(searchUpdate_slot(int,int,int)));
		disconnect(d->searchThread, SIGNAL(searchError(QString)),
			   this, SLOT(searchError_slot(QString)));
	}

	d->searchThread = searchThread;

	if (d->searchThread) {
		// Connect signals to the new searchThread.
		connect(d->searchThread, SIGNAL(searchStarted(int,int,int)),
			this, SLOT(searchStarted_slot(int,int,int)));
		connect(d->searchThread, SIGNAL(searchCancelled()),
			this, SLOT(searchCancelled_slot()));
		connect(d->searchThread, SIGNAL(searchFinished(int)),
			this, SLOT(searchFinished_slot(int)));
		connect(d->searchThread, SIGNAL(searchUpdate(int,int,int)),
			this, SLOT(searchUpdate_slot(int,int,int)));
		connect(d->searchThread, SIGNAL(searchError(QString)),
			this, SLOT(searchError_slot(QString)));
	}
}


/** Slots. **/

/**
 * User closed the window.
 * @param event QCloseEvent.
 */
void SearchDialog::closeEvent(QCloseEvent *event)
{
	event->ignore();
	reject();
}

/**
 * User clicked cancel.
 */
void SearchDialog::reject(void)
{
	// TODO: Cancel the search.
	QDialog::reject();
}


/**
 * Search has started.
 * @param totalPhysBlocks Total number of blocks in the card.
 * @param totalSearchBlocks Number of blocks being searched.
 * @param firstPhysBlock First block being searched.
 */
void SearchDialog::searchStarted_slot(int totalPhysBlocks, int totalSearchBlocks, int firstPhysBlock)
{
	// Initialize the search status.
	d->currentPhysBlock = firstPhysBlock;
	d->totalPhysBlocks = totalPhysBlocks;
	d->currentSearchBlock = 0;
	d->totalSearchBlocks = totalSearchBlocks;
	d->lostFilesFound = 0;
	d->updateSearchStatus();
}

/**
 * Search has been cancelled.
 */
void SearchDialog::searchCancelled_slot(void)
{
	// TODO
}

/**
 * Search has completed.
 * @param lostFilesFound Number of "lost" files found.
 */
void SearchDialog::searchFinished_slot(int lostFilesFound)
{
	// Update the search status.
	d->lostFilesFound = lostFilesFound;
	d->currentSearchBlock = d->totalSearchBlocks;
	d->updateSearchStatus();

	// TODO: Hide the window?
}

/**
 * Update search status.
 * @param currentPhysBlock Current physical block number being searched.
 * @param currentSearchBlock Number of blocks searched so far.
 * @param lostFilesFound Number of "lost" files found.
 */
void SearchDialog::searchUpdate_slot(int currentPhysBlock, int currentSearchBlock, int lostFilesFound)
{
	// Update the search status.
	d->currentPhysBlock = currentPhysBlock;
	d->currentSearchBlock = currentSearchBlock;
	d->lostFilesFound = lostFilesFound;
	d->updateSearchStatus();
}

/**
 * An error has occurred during the search.
 * @param errorString Error string.
 */
void SearchDialog::searchError_slot(QString errorString)
{
	// TODO
}
