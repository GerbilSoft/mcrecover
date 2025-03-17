/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * StatusBarManager.cpp: Status Bar manager                                *
 *                                                                         *
 * Copyright (c) 2013-2025 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#include "StatusBarManager.hpp"

// Search Thread.
#include "db/GcnSearchThread.hpp"

// Qt includes.
#include <QtCore/QDir>
#include <QtCore/QTimer>
#include <QApplication>
#include <QLabel>
#include <QStatusBar>
#include <QProgressBar>

// taskbarButtonManager.
#include "TaskbarButtonManager/TaskbarButtonManager.hpp"

/** StatusBarManagerPrivate **/

class StatusBarManagerPrivate
{
public:
	explicit StatusBarManagerPrivate(StatusBarManager *q);
	~StatusBarManagerPrivate();

protected:
	StatusBarManager *const q_ptr;
	Q_DECLARE_PUBLIC(StatusBarManager)
private:
	Q_DISABLE_COPY(StatusBarManagerPrivate)

public:
	QStatusBar *statusBar;		// Status bar
	QLabel *lblMessage;		// Message label
	QProgressBar *progressBar;	// Progress bar

	// Search thread
	// NOTE: We don't own this!
	GcnSearchThread *searchThread;

	// Last status message
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
	static constexpr int SECONDS_TO_HIDE_PROGRESS_BAR = 5;

	// TaskbarButtonManager
	TaskbarButtonManager *taskbarButtonManager;

	// Timer for hiding the progress bar
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
	, taskbarButtonManager(nullptr)
{
	// Default message.
	lastStatusMessage = StatusBarManager::tr("Ready.");

	// Initialize the timer.
	tmrHideProgressBar.setInterval(SECONDS_TO_HIDE_PROGRESS_BAR * 1000);
	tmrHideProgressBar.setSingleShot(true);
	QObject::connect(&tmrHideProgressBar, &QTimer::timeout,
		q, &StatusBarManager::hideProgressBar_slot);
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
		lastStatusMessage = StatusBarManager::tr("Scanning block #%L1 (%L2 scanned, %L3 remaining)...")
					.arg(currentPhysBlock)
					.arg(currentSearchBlock)
					.arg(totalSearchBlocks - currentSearchBlock);

		/* TODO: Show number of files found?
		QString filesFoundText = StatusBarManager::tr("%n lost file(s) found.", nullptr, lostFilesFound);
		q->lblFilesFound->setText(filesFoundText);
		*/
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

	// Make sure the progress bar is visible when scanning.
	if (scanning && progressBar)
		progressBar->setVisible(true);

	// Set the progress bar values.
	if (progressBar && progressBar->isVisible()) {
		progressBar->setMaximum(totalSearchBlocks);
		progressBar->setValue(currentSearchBlock);
		if (taskbarButtonManager) {
			// TODO: Set max only in initialization?
			taskbarButtonManager->setProgressBarValue(currentSearchBlock);
			taskbarButtonManager->setProgressBarMax(totalSearchBlocks);
		}
	} else {
		if (taskbarButtonManager) {
			taskbarButtonManager->clearProgressBar();
		}
	}
}

/** StatusBarManager **/

StatusBarManager::StatusBarManager(QObject *parent)
	: super(parent)
	, d_ptr(new StatusBarManagerPrivate(this))
{ }

StatusBarManager::StatusBarManager(QStatusBar *statusBar, QObject *parent)
	: super(parent)
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
 * @return QStatusBar
 */
QStatusBar *StatusBarManager::statusBar(void) const
{
	Q_D(const StatusBarManager);
	return d->statusBar;
}

/**
 * Set the QStatusBar.
 * @param statusBar QStatusBar
 */
void StatusBarManager::setStatusBar(QStatusBar *statusBar)
{
	Q_D(StatusBarManager);
	if (d->statusBar == statusBar) {
		return;
	}

	// Stop the Hide Progress Bar timer.
	d->tmrHideProgressBar.stop();

	if (d->statusBar) {
		// Disconnect signals from the current statusBar.
		disconnect(d->statusBar, &QObject::destroyed,
			   this, &StatusBarManager::object_destroyed_slot);
		disconnect(d->lblMessage, &QObject::destroyed,
			   this, &StatusBarManager::object_destroyed_slot);
		disconnect(d->progressBar, &QObject::destroyed,
			   this, &StatusBarManager::object_destroyed_slot);

		// Delete the progress bar.
		delete d->progressBar;
		d->progressBar = nullptr;
	}

	d->statusBar = statusBar;

	if (d->statusBar) {
		// Connect signals to the new statusBar.
		connect(d->statusBar, &QObject::destroyed,
			this, &StatusBarManager::object_destroyed_slot);

		// Create a new message label.
		d->lblMessage = new QLabel();
		d->lblMessage->setTextFormat(Qt::PlainText);
		connect(d->lblMessage, &QObject::destroyed,
			this, &StatusBarManager::object_destroyed_slot);
		d->statusBar->addWidget(d->lblMessage);

		// Create a new progress bar.
		d->progressBar = new QProgressBar();
		d->progressBar->setVisible(false);
		connect(d->progressBar, &QObject::destroyed,
			this, &StatusBarManager::object_destroyed_slot);
		d->statusBar->addPermanentWidget(d->progressBar, 1);

		// Set the progress bar's size so it doesn't randomly resize.
		d->progressBar->setMinimumWidth(320);
		d->progressBar->setMaximumWidth(320);

		// Update the status bar.
		d->updateStatusBar();
	}
}

/**
 * Get the GcnSearchThread.
 * @return GcnSearchThread
 */
GcnSearchThread *StatusBarManager::searchThread(void) const
{
	Q_D(const StatusBarManager);
	return d->searchThread;
}

/**
 * Set the GcnSearchThread.
 * @param searchThread GcnSearchThread
 */
void StatusBarManager::setSearchThread(GcnSearchThread *searchThread)
{
	Q_D(StatusBarManager);
	if (d->searchThread == searchThread) {
		return;
	}

	if (d->searchThread) {
		// Disconnect signals from the current searchThread.
		disconnect(d->searchThread, &QObject::destroyed,
			   this, &StatusBarManager::object_destroyed_slot);
		disconnect(d->searchThread, &GcnSearchThread::searchStarted,
			   this, &StatusBarManager::searchStarted_slot);
		disconnect(d->searchThread, &GcnSearchThread::searchCancelled,
			   this, &StatusBarManager::searchCancelled_slot);
		disconnect(d->searchThread, &GcnSearchThread::searchFinished,
			   this, &StatusBarManager::searchFinished_slot);
		disconnect(d->searchThread, &GcnSearchThread::searchUpdate,
			   this, &StatusBarManager::searchUpdate_slot);
		disconnect(d->searchThread, &GcnSearchThread::searchError,
			   this, &StatusBarManager::searchError_slot);
	}

	d->searchThread = searchThread;

	if (searchThread) {
		// Connect signals to the new searchThread.
		connect(d->searchThread, &QObject::destroyed,
			this, &StatusBarManager::object_destroyed_slot);
		connect(d->searchThread, &GcnSearchThread::searchStarted,
			this, &StatusBarManager::searchStarted_slot);
		connect(d->searchThread, &GcnSearchThread::searchCancelled,
			this, &StatusBarManager::searchCancelled_slot);
		connect(d->searchThread, &GcnSearchThread::searchFinished,
			this, &StatusBarManager::searchFinished_slot);
		connect(d->searchThread, &GcnSearchThread::searchUpdate,
			this, &StatusBarManager::searchUpdate_slot);
		connect(d->searchThread, &GcnSearchThread::searchError,
			this, &StatusBarManager::searchError_slot);
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

/**
 * Get the TaskbarButtonManager.
 * @return TaskbarButtonManager
 */
TaskbarButtonManager *StatusBarManager::taskbarButtonManager(void) const
{
	Q_D(const StatusBarManager);
	return d->taskbarButtonManager;
}

/**
 * Set the TaskbarButtonManager.
 * @param taskbarButtonManager TaskbarButtonManager
 */
void StatusBarManager::setTaskbarButtonManager(TaskbarButtonManager *taskbarButtonManager)
{
	Q_D(StatusBarManager);
	if (d->taskbarButtonManager == taskbarButtonManager) {
		return;
	}

	if (d->taskbarButtonManager) {
		// Disconnect the "destroyed" slot.
		disconnect(d->taskbarButtonManager, &QObject::destroyed,
			   this, &StatusBarManager::object_destroyed_slot);
	}

	d->taskbarButtonManager = taskbarButtonManager;

	if (taskbarButtonManager) {
		// Connect the "destroyed" slot.
		connect(taskbarButtonManager, &QObject::destroyed,
			this, &StatusBarManager::object_destroyed_slot);

		// Update the status.
		d->updateStatusBar();
	}
}

/** Public Slots **/

/**
 * A memory card image was opened.
 * @param filename Filename
 * @param productName Product name of the memory card
 */
void StatusBarManager::opened(const QString &filename, const QString &productName)
{
	// Extract the filename from the path.
	QFileInfo fileInfo(filename);
	QString filename_noPath = fileInfo.fileName();

	Q_D(StatusBarManager);
	d->scanning = false;
	d->progressBar->setVisible(false);
	d->lastStatusMessage = tr("Loaded %1 image %2")
				.arg(productName)
				.arg(filename_noPath);
	d->updateStatusBar();

	// Stop the Hide Progress Bar timer.
	d->tmrHideProgressBar.stop();
}

/**
 * The current memory card image was closed.
 * @param productName Product name of the memory card
 */
void StatusBarManager::closed(const QString &productName)
{
	Q_D(StatusBarManager);
	d->scanning = false;
	d->progressBar->setVisible(false);
	d->lastStatusMessage = tr("%1 image closed.").arg(productName);
	d->updateStatusBar();

	// Stop the Hide Progress Bar timer.
	d->tmrHideProgressBar.stop();
}

/**
 * Files were saved.
 * @param n Number of files saved
 * @param path Path files were saved to
 */
void StatusBarManager::filesSaved(int n, const QString &path)
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

/** Private Slots **/

/**
 * An object has been destroyed.
 * @param obj QObject that was destroyed
 */
void StatusBarManager::object_destroyed_slot(QObject *obj)
{
	if (!obj)
		return;

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
	} else if (obj == d->taskbarButtonManager) {
		d->taskbarButtonManager = nullptr;
	}
}

/**
 * Search has started.
 * @param totalPhysBlocks Total number of blocks in the card
 * @param totalSearchBlocks Number of blocks being searched
 * @param firstPhysBlock First block being searched
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
	d->scanning = false;
	d->lastStatusMessage = tr("Scan cancelled.");
	d->updateStatusBar();

	// Stop the Hide Progress Bar timer.
	// TODO: (or keep it running?)
	d->tmrHideProgressBar.stop();
}

/**
 * Search has completed.
 * @param lostFilesFound Number of "lost" files found
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
 * @param currentPhysBlock Current physical block number being searched
 * @param currentSearchBlock Number of blocks searched so far
 * @param lostFilesFound Number of "lost" files found
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
 * @param errorString Error string
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
