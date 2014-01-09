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
#include <QtCore/QTimer>
#include <QtGui/QApplication>
#include <QtGui/QLabel>
#include <QtGui/QStatusBar>
#include <QtGui/QProgressBar>

// taskbarButtonManager.
#include "TaskbarButtonManager/TaskbarButtonManager.hpp"


/** StatusBarManagerPrivate **/

class StatusBarManagerPrivate
{
	public:
		StatusBarManagerPrivate(StatusBarManager *q);
		~StatusBarManagerPrivate();

	protected:
		StatusBarManager *const q_ptr;
		Q_DECLARE_PUBLIC(StatusBarManager)
	private:
		Q_DISABLE_COPY(StatusBarManagerPrivate)

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

		// Number of seconds to wait before hiding the
		// progress bar after the search has completed.
		static const int SECONDS_TO_HIDE_PROGRESS_BAR = 5;

		// TaskbarButtonManager.
		TaskbarButtonManager *taskbarButtonManager;

		// Timer for hiding the progress bar.
		QTimer tmrHideProgressBar;
};

StatusBarManagerPrivate::StatusBarManagerPrivate(StatusBarManager *q)
	: q_ptr(q)
	, statusBar(nullptr)
	, lblMessage(nullptr)
	, progressBar(nullptr)
	, searchThread(nullptr)
	, scanning(false)
	, currentPhysBlock(0)
	, totalPhysBlocks(0)
	, currentSearchBlock(0)
	, totalSearchBlocks(0)
	, lostFilesFound(0)
	, taskbarButtonManager(TaskbarButtonManager::Instance(q))
{
	// Default message.
	lastStatusMessage = StatusBarManager::tr("Ready.");

	// Initialize the timer.
	tmrHideProgressBar.setInterval(SECONDS_TO_HIDE_PROGRESS_BAR * 1000);
	tmrHideProgressBar.setSingleShot(true);
	QObject::connect(&tmrHideProgressBar, SIGNAL(timeout()),
			 q, SLOT(hideProgressBar_slot()));
}

StatusBarManagerPrivate::~StatusBarManagerPrivate()
{
	delete lblMessage;
	delete progressBar;
	delete statusBar;
	delete taskbarButtonManager;
}

/**
 * Update the status bar.
 */
void StatusBarManagerPrivate::updateStatusBar(void)
{
	if (scanning) {
		// We're scanning for files.
		lastStatusMessage = StatusBarManager::tr("Scanning block #%L1 (%L2 scanned, %L3 remaining)...")
					.arg(currentPhysBlock)
					.arg(currentSearchBlock)
					.arg(totalSearchBlocks - currentSearchBlock);

		/* TODO: Show number of files found?
		QString filesFoundText = StatusBarManager::tr("%n lost file(s) found.", nullptr, lostFilesFound);
		q->lblFilesFound->setText(filesFoundText);
		*/

		// Update the progress bar.
		if (progressBar)
			progressBar->setVisible(true);
	}

	// Set the status bar message.
	if (lblMessage) {
		lblMessage->setText(lastStatusMessage);

		// HACK: Workaround for the label sometimes
		// not resizing fully on Qt/Linux, resulting
		// in truncated text.
		int w = statusBar->width();
		if (progressBar)
			w -= progressBar->width();
		lblMessage->resize(w, lblMessage->height());
	}

	// Set the progress bar values.
	// TODO: Hide the progress bar ~5 seconds after scan is complete?
	if (progressBar && progressBar->isVisible()) {
		progressBar->setMaximum(totalSearchBlocks);
		progressBar->setValue(currentSearchBlock);
		if (taskbarButtonManager) {
			// TODO: Set max only in initialization?
			taskbarButtonManager->setProgressBarValue(currentSearchBlock);
			taskbarButtonManager->setProgressBarMax(totalSearchBlocks);
		}
	} else {
		if (taskbarButtonManager)
			taskbarButtonManager->clearProgressBar();
	}
}

/** StatusBarManager **/

StatusBarManager::StatusBarManager(QObject *parent)
	: QObject(parent)
	, d_ptr(new StatusBarManagerPrivate(this))
{ }

StatusBarManager::StatusBarManager(QStatusBar *statusBar, QObject *parent)
	: QObject(parent)
	, d_ptr(new StatusBarManagerPrivate(this))
{
	// Initialize the status bar.
	setStatusBar(statusBar);
}

StatusBarManager::~StatusBarManager()
{
	Q_D(StatusBarManager);
	delete d;
}

/**
 * Get the QStatusBar.
 * @return QStatusBar.
 */
QStatusBar *StatusBarManager::statusBar(void) const
{
	Q_D(const StatusBarManager);
	return d->statusBar;
}

/**
 * Set the QStatusBar.
 * @param statusBar QStatusBar.
 */
void StatusBarManager::setStatusBar(QStatusBar *statusBar)
{
	Q_D(StatusBarManager);

	// Stop the Hide Progress Bar timer.
	d->tmrHideProgressBar.stop();

	if (d->statusBar) {
		// Disconnect signals from the current statusBar.
		disconnect(d->statusBar, SIGNAL(destroyed(QObject*)),
			   this, SLOT(object_destroyed_slot(QObject*)));
		disconnect(d->lblMessage, SIGNAL(destroyed(QObject*)),
			   this, SLOT(object_destroyed_slot(QObject*)));
		disconnect(d->progressBar, SIGNAL(destroyed(QObject*)),
			   this, SLOT(object_destroyed_slot(QObject*)));

		// Clear the TaskbarButtonManager window.
		if (d->taskbarButtonManager)
			d->taskbarButtonManager->setWindow(nullptr);

		// Delete the progress bar.
		delete d->progressBar;
		d->progressBar = nullptr;
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

		// Set the TaskbarButtonmanager's window to the status bar's topmost parent window.
		if (d->taskbarButtonManager)
			d->taskbarButtonManager->setWindow(d->statusBar->window());

		// Update the status bar.
		d->updateStatusBar();
	}
}

/**
 * Get the SearchThread.
 * @return SearchThread.
 */
SearchThread *StatusBarManager::searchThread(void) const
{
	Q_D(const StatusBarManager);
	return d->searchThread;
}

/**
 * Set the SearchThread.
 * @param searchThread SearchThread.
 */
void StatusBarManager::setSearchThread(SearchThread *searchThread)
{
	Q_D(StatusBarManager);

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

	Q_D(StatusBarManager);
	d->scanning = false;
	d->progressBar->setVisible(false);
	d->lastStatusMessage = tr("Loaded GameCube Memory Card image %1").arg(filename);
	d->updateStatusBar();

	// Stop the Hide Progress Bar timer.
	d->tmrHideProgressBar.stop();
}

/**
 * The current GameCube Memory Card image was closed.
 */
void StatusBarManager::closed(void)
{
	Q_D(StatusBarManager);
	d->scanning = false;
	d->progressBar->setVisible(false);
	d->lastStatusMessage = tr("GameCube Memory Card image closed.");
	d->updateStatusBar();

	// Stop the Hide Progress Bar timer.
	d->tmrHideProgressBar.stop();
}

/**
 * Files were saved.
 * @param n Number of files saved.
 * @param path Path files were saved to.
 */
void StatusBarManager::filesSaved(int n, QString path)
{
	Q_D(StatusBarManager);
	d->scanning = false;
	d->progressBar->setVisible(false);
	d->lastStatusMessage = tr("%Ln file(s) saved to %1.", "", n)
				.arg(QDir::toNativeSeparators(path));
	d->updateStatusBar();

	// Stop the Hide Progress Bar timer.
	d->tmrHideProgressBar.stop();
}

/** Private Slots. **/

/**
 * An object has been destroyed.
 * @param obj QObject that was destroyed.
 */
void StatusBarManager::object_destroyed_slot(QObject *obj)
{
	Q_D(StatusBarManager);

	if (obj == d->statusBar) {
		d->statusBar = nullptr;
	} else if (obj == d->lblMessage) {
		d->lblMessage = nullptr;
	} else if (obj == d->progressBar) {
		// Stop the Hide Progress Bar timer.
		d->tmrHideProgressBar.stop();
		d->progressBar = nullptr;
	} else if (obj == d->searchThread) {
		d->searchThread = nullptr;
	}
}

/**
 * Search has started.
 * @param totalPhysBlocks Total number of blocks in the card.
 * @param totalSearchBlocks Number of blocks being searched.
 * @param firstPhysBlock First block being searched.
 */
void StatusBarManager::searchStarted_slot(int totalPhysBlocks, int totalSearchBlocks, int firstPhysBlock)
{
	Q_D(StatusBarManager);

	// Initialize the search status.
	d->scanning = true;
	// NOTE: When scanning, lastStatusMessage is set by updateStatusBar().
	d->currentPhysBlock = firstPhysBlock;
	d->totalPhysBlocks = totalPhysBlocks;
	d->currentSearchBlock = 0;
	d->totalSearchBlocks = totalSearchBlocks;
	d->lostFilesFound = 0;
	d->updateStatusBar();

	// Stop the Hide Progress Bar timer.
	d->tmrHideProgressBar.stop();
}

/**
 * Search has been cancelled.
 */
void StatusBarManager::searchCancelled_slot(void)
{
	Q_D(StatusBarManager);

	// TODO
	d->scanning = false;
	d->lastStatusMessage = tr("Scan cancelled.");
	d->updateStatusBar();

	// Stop the Hide Progress Bar timer.
	// TODO: (or keep it running?)
	d->tmrHideProgressBar.stop();
}

/**
 * Search has completed.
 * @param lostFilesFound Number of "lost" files found.
 */
void StatusBarManager::searchFinished_slot(int lostFilesFound)
{
	Q_D(StatusBarManager);

	// Update the search status.
	d->scanning = false;
	d->lostFilesFound = lostFilesFound;
	d->currentSearchBlock = d->totalSearchBlocks;
	d->lastStatusMessage = tr("Scan complete. %Ln lost file(s) found.", "", lostFilesFound);
	d->updateStatusBar();

	// Hide the progress bar after a few seconds.
	d->tmrHideProgressBar.start();
}

/**
 * Update search status.
 * @param currentPhysBlock Current physical block number being searched.
 * @param currentSearchBlock Number of blocks searched so far.
 * @param lostFilesFound Number of "lost" files found.
 */
void StatusBarManager::searchUpdate_slot(int currentPhysBlock, int currentSearchBlock, int lostFilesFound)
{
	Q_D(StatusBarManager);

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
	Q_D(StatusBarManager);

	d->scanning = false;
	d->lastStatusMessage = tr("An error occurred while scanning: %1")
				.arg(errorString);

	// TODO: Keep the progress bar visible but indicate an error.
}

/**
 * Hide the progress bar.
 * This is usually done a few seconds after the
 * search is completed.
 */
void StatusBarManager::hideProgressBar_slot(void)
{
	Q_D(StatusBarManager);
	d->progressBar->setVisible(false);
	d->updateStatusBar();
}
