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

#include "StatusBarManager.hpp"

// Search Thread.
#include "SearchThread.hpp"

// Qt includes.
#include <QtCore/QDir>
#include <QtGui/QApplication>
#include <QtGui/QLabel>
#include <QtGui/QStatusBar>
#include <QtGui/QProgressBar>


/** StatusBarManagerPrivate **/

class StatusBarManagerPrivate
{
	public:
		StatusBarManagerPrivate(StatusBarManager *q);
		~StatusBarManagerPrivate();

	private:
		StatusBarManager *const q;
		Q_DISABLE_COPY(StatusBarManagerPrivate);

	public:
		// Status bar.
		QStatusBar *statusBar;

		// Message label.
		QLabel *lblMessage;

		// Progress bar.
		QProgressBar *progressBar;

		// Search thread.
		// NOTE: We don't own this!
		SearchThread *searchThread;

		// Last status message.
		QString lastStatusMessage;

		/**
		 * Update the status bar.
		 */
		void updateStatusBar(void);

		// Are we currently scanning a memory card?
		bool scanning;

		// Search status from last SearchThread update.
		int currentPhysBlock;
		int totalPhysBlocks;
		int currentSearchBlock;
		int totalSearchBlocks;
		int lostFilesFound;
};

StatusBarManagerPrivate::StatusBarManagerPrivate(StatusBarManager *q)
	: q(q)
	, statusBar(NULL)
	, lblMessage(NULL)
	, progressBar(NULL)
	, searchThread(NULL)
	, scanning(false)
	, currentPhysBlock(0)
	, totalPhysBlocks(0)
	, currentSearchBlock(0)
	, totalSearchBlocks(0)
	, lostFilesFound(0)
{
	// Default message.
	lastStatusMessage = q->tr("Ready.");
}

StatusBarManagerPrivate::~StatusBarManagerPrivate()
{
	delete lblMessage;
	delete progressBar;
	delete statusBar;
}


/**
 * Update the status bar.
 */
void StatusBarManagerPrivate::updateStatusBar(void)
{
	if (scanning) {
		// We're scanning for files.
		lastStatusMessage = q->tr("Scanning block #%L1 (%L2 scanned, %L3 remaining)")
					.arg(currentPhysBlock)
					.arg(currentSearchBlock)
					.arg(totalSearchBlocks - currentSearchBlock);

		// TODO: Show number of files found?
		/*
		QString filesFoundText = q->tr("%n lost file(s) found.", NULL, lostFilesFound);
		q->lblFilesFound->setText(filesFoundText);
		*/

		// Update the progress bar.
		if (progressBar)
			progressBar->setVisible(true);
	}

	// Set the status bar message.
	if (lblMessage)
		lblMessage->setText(lastStatusMessage);

	// Set the progress bar values.
	// TODO: Hide the progress bar ~5 seconds after scan is complete?
	if (progressBar) {
		progressBar->setMaximum(totalSearchBlocks);
		progressBar->setValue(currentSearchBlock);
	}

	// Make sure the label updates are processed.
	QApplication::processEvents();
}


/** StatusBarManager **/

StatusBarManager::StatusBarManager(QObject *parent)
	: QObject(parent)
	, d(new StatusBarManagerPrivate(this))
{ }

StatusBarManager::StatusBarManager(QStatusBar *statusBar, QObject *parent)
	: QObject(parent)
	, d(new StatusBarManagerPrivate(this))
{
	// Initialize the status bar.
	setStatusBar(statusBar);
}

StatusBarManager::~StatusBarManager()
{
	delete d;
}


/**
 * Get the QStatusBar.
 * @return QStatusBar.
 */
QStatusBar *StatusBarManager::statusBar(void) const
	{ return d->statusBar; }

/**
 * Set the QStatusBar.
 * @param statusBar QStatusBar.
 */
void StatusBarManager::setStatusBar(QStatusBar *statusBar)
{
	if (d->statusBar) {
		// Disconnect signals from the current statusBar.
		disconnect(d->statusBar, SIGNAL(destroyed(QObject*)),
			   this, SLOT(object_destroyed_slot(QObject*)));
		disconnect(d->lblMessage, SIGNAL(destroyed(QObject*)),
			   this, SLOT(object_destroyed_slot(QObject*)));
		disconnect(d->progressBar, SIGNAL(destroyed(QObject*)),
			   this, SLOT(object_destroyed_slot(QObject*)));

		// Delete the progress bar.
		delete d->progressBar;
		d->progressBar = NULL;
	}

	d->statusBar = statusBar;

	if (d->statusBar) {
		// Connect signals to the new statusBar.
		connect(d->statusBar, SIGNAL(destroyed(QObject*)),
			this, SLOT(object_destroyed_slot(QObject*)));

		// Create a new message label.
		d->lblMessage = new QLabel();
		d->lblMessage->setTextFormat(Qt::PlainText);
		connect(d->lblMessage, SIGNAL(destroyed(QObject*)),
			this, SLOT(object_destroyed_slot(QObject*)));
		d->statusBar->addWidget(d->lblMessage);

		// Create a new progress bar.
		d->progressBar = new QProgressBar();
		d->progressBar->setVisible(false);
		connect(d->progressBar, SIGNAL(destroyed(QObject*)),
			this, SLOT(object_destroyed_slot(QObject*)));
		d->statusBar->addPermanentWidget(d->progressBar, 1);

		// Set the progress bar's size so it doesn't randomly resize.
		d->progressBar->setMinimumWidth(320);
		d->progressBar->setMaximumWidth(320);

		// Update the status bar.
		d->updateStatusBar();
	}
}


/**
 * Get the SearchThread.
 * @return SearchThread.
 */
SearchThread *StatusBarManager::searchThread(void) const
	{ return d->searchThread; }

/**
 * Set the SearchThread.
 * @param searchThread SearchThread.
 */
void StatusBarManager::setSearchThread(SearchThread *searchThread)
{
	if (d->searchThread) {
		// Disconnect signals from the current searchThread.
		disconnect(d->searchThread, SIGNAL(destroyed(QObject*)),
			   this, SLOT(object_destroyed_slot(QObject*)));
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
		connect(d->searchThread, SIGNAL(destroyed(QObject*)),
			this, SLOT(object_destroyed_slot(QObject*)));
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

	// TODO: Get current status from the new searchThread.
	// For now, just clear everything.
	d->scanning = false;
	d->currentPhysBlock = 0;
	d->totalPhysBlocks = 0;
	d->currentPhysBlock = 0;
	d->totalSearchBlocks = 0;
	d->lostFilesFound = 0;
	d->updateStatusBar();
}


/** Public Slots. **/


/**
 * A GameCube Memory Card image was opened.
 * @param filename Filename.
 */
void StatusBarManager::opened(QString filename)
{
	// Extract the filename from the path.
	int lastSlash = filename.lastIndexOf(QChar(L'/'));
	if (lastSlash >= 0)
		filename.remove(0, lastSlash + 1);

	d->scanning = false;
	d->progressBar->setVisible(false);
	d->lastStatusMessage = tr("Loaded GameCube Memory Card image %1").arg(filename);
	d->updateStatusBar();
}


/**
 * The current GameCube Memory Card image was closed.
 */
void StatusBarManager::closed(void)
{
	d->scanning = false;
	d->progressBar->setVisible(false);
	d->lastStatusMessage = tr("GameCube Memory Card image closed.");
	d->updateStatusBar();
}


/**
 * Files were saved.
 * @param n Number of files saved.
 * @param path Path files were saved to.
 */
void StatusBarManager::filesSaved(int n, QString path)
{
	d->scanning = false;
	d->progressBar->setVisible(false);
	d->lastStatusMessage = tr("%Ln file(s) saved to %1.", NULL, n)
				.arg(QDir::toNativeSeparators(path));
	d->updateStatusBar();
}


/** Private Slots. **/


/**
 * An object has been destroyed.
 * @param obj QObject that was destroyed.
 */
void StatusBarManager::object_destroyed_slot(QObject *obj)
{
	if (obj == d->statusBar)
		d->statusBar = NULL;
	else if (obj == d->lblMessage)
		d->lblMessage = NULL;
	else if (obj == d->progressBar)
		d->progressBar = NULL;
	else if (obj == d->searchThread)
		d->searchThread = NULL;
}


/**
 * Search has started.
 * @param totalPhysBlocks Total number of blocks in the card.
 * @param totalSearchBlocks Number of blocks being searched.
 * @param firstPhysBlock First block being searched.
 */
void StatusBarManager::searchStarted_slot(int totalPhysBlocks, int totalSearchBlocks, int firstPhysBlock)
{
	// Initialize the search status.
	d->scanning = true;
	// NOTE: When scanning, lastStatusMessage is set by updateStatusBar().
	d->currentPhysBlock = firstPhysBlock;
	d->totalPhysBlocks = totalPhysBlocks;
	d->currentSearchBlock = 0;
	d->totalSearchBlocks = totalSearchBlocks;
	d->lostFilesFound = 0;
	d->updateStatusBar();
}

/**
 * Search has been cancelled.
 */
void StatusBarManager::searchCancelled_slot(void)
{
	// TODO
	d->scanning = false;
	d->lastStatusMessage = tr("Scan cancelled.");
	d->updateStatusBar();
}

/**
 * Search has completed.
 * @param lostFilesFound Number of "lost" files found.
 */
void StatusBarManager::searchFinished_slot(int lostFilesFound)
{
	// Update the search status.
	d->scanning = false;
	d->lostFilesFound = lostFilesFound;
	d->currentSearchBlock = d->totalSearchBlocks;
	d->lastStatusMessage = tr("Scan complete. %Ln lost file(s) found.", NULL, lostFilesFound);
	d->updateStatusBar();
}

/**
 * Update search status.
 * @param currentPhysBlock Current physical block number being searched.
 * @param currentSearchBlock Number of blocks searched so far.
 * @param lostFilesFound Number of "lost" files found.
 */
void StatusBarManager::searchUpdate_slot(int currentPhysBlock, int currentSearchBlock, int lostFilesFound)
{
	// Update the search status.
	// NOTE: When scanning, lastStatusMessage is set by updateStatusBar().
	d->currentPhysBlock = currentPhysBlock;
	d->currentSearchBlock = currentSearchBlock;
	d->lostFilesFound = lostFilesFound;
	d->updateStatusBar();
}

/**
 * An error has occurred during the search.
 * @param errorString Error string.
 */
void StatusBarManager::searchError_slot(QString errorString)
{
	d->scanning = false;
	d->lastStatusMessage = tr("An error occurred while scanning: %1")
				.arg(errorString);
}
