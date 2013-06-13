/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * McRecoverWindow.cpp: Main window.                                       *
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

#include "McRecoverWindow.hpp"
#include "McRecoverQApplication.hpp"
#include "AboutDialog.hpp"

// MemCard classes.
#include "MemCard.hpp"
#include "MemCardFile.hpp"
#include "MemCardModel.hpp"

// Search classes.
#include "SearchThread.hpp"
#include "StatusBarManager.hpp"

// C includes.
#include <cstdio>

// Qt includes. (Drag & Drop)
#include <QtCore/QUrl>
#include <QtCore/QStack>
#include <QtGui/QAction>
#include <QtGui/QDragEnterEvent>
#include <QtGui/QDropEvent>
#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>
#include <QtGui/QToolBar>


/** McRecoverWindowPrivate **/

class McRecoverWindowPrivate
{
	public:
		McRecoverWindowPrivate(McRecoverWindow *q);
		~McRecoverWindowPrivate();

	private:
		McRecoverWindow *const q;
		Q_DISABLE_COPY(McRecoverWindowPrivate);

	public:
		// Memory Card instance.
		MemCard *card;

		// Memory Card model for lstFileList.
		MemCardModel *model;

		// Filename.
		QString filename;
		QString displayFilename;	// filename without subdirectories

		/**
		 * Update the memory card's QTreeView.
		 */
		void updateLstFileList(void);

		// Search thread.
		SearchThread *searchThread;

		// Memory Card toolbar.
		QToolBar *mcToolbar;

		/**
		 * Initialize the Memory Card toolbar.
		 */
		void initMcToolbar(void);

		/**
		 * Update the action enable status.
		 */
		void updateActionEnableStatus(void);

		// Status Bar Manager.
		StatusBarManager *statusBarManager;

		/**
		 * Update the window title.
		 */
		void updateWindowTitle(void);
};

McRecoverWindowPrivate::McRecoverWindowPrivate(McRecoverWindow *q)
	: q(q)
	, card(NULL)
	, model(new MemCardModel(q))
	, searchThread(new SearchThread(q))
	, mcToolbar(NULL)
	, statusBarManager(NULL)
{
	// Show icon, description, size, mtime, permission, and gamecode by default.
	// TODO: Allow the user to customize the columns, and save the 
	// customized columns somewhere.
	model->setColumnVisible(MemCardModel::COL_ICON, true);
	model->setColumnVisible(MemCardModel::COL_BANNER, false);
	model->setColumnVisible(MemCardModel::COL_DESCRIPTION, true);
	model->setColumnVisible(MemCardModel::COL_SIZE, true);
	model->setColumnVisible(MemCardModel::COL_MTIME, true);
	model->setColumnVisible(MemCardModel::COL_PERMISSION, true);
	model->setColumnVisible(MemCardModel::COL_GAMECODE, true);
	model->setColumnVisible(MemCardModel::COL_FILENAME, false);

	// Connect the MemCardModel slots.
	QObject::connect(model, SIGNAL(layoutChanged()),
			 q, SLOT(memCardModel_layoutChanged()));
	QObject::connect(model, SIGNAL(rowsInserted(QModelIndex,int,int)),
			 q, SLOT(memCardModel_rowsInserted()));

	// Connect the SearchThread slots.
	QObject::connect(searchThread, SIGNAL(searchFinished(int)),
			 q, SLOT(searchThread_searchFinished_slot(int)));
}

McRecoverWindowPrivate::~McRecoverWindowPrivate()
{
	delete model;
	delete card;

	// TODO: Wait for searchThread to finish?
	delete searchThread;

	delete mcToolbar;
	delete statusBarManager;
}


/**
 * Update the memory card's QTreeView.
 */
void McRecoverWindowPrivate::updateLstFileList(void)
{
	if (!card) {
		// Set the group box's title.
		q->grpFileList->setTitle(q->tr("No memory card loaded."));
	} else {
		// Show the filename.
		q->grpFileList->setTitle(displayFilename);
	}

	// Show the QTreeView headers if a memory card is loaded.
	q->lstFileList->setHeaderHidden(!card);

	// Update the action enable status.
	updateActionEnableStatus();

	// Resize the columns to fit the contents.
	for (int i = 0; i < model->columnCount(); i++)
		q->lstFileList->resizeColumnToContents(i);
}


/**
 * Initialize the Memory Card toolbar.
 */
void McRecoverWindowPrivate::initMcToolbar(void)
{
	// Set action icons.
	// FIXME: Currently specifying fd.o theme icons in the UI file.
	// Does this work properly when compiled with Qt <4.8?
	q->actionOpen->setIcon(
		McRecoverQApplication::IconFromTheme(QLatin1String("document-open")));
	q->actionClose->setIcon(
		McRecoverQApplication::IconFromTheme(QLatin1String("document-close")));
	q->actionScan->setIcon(
		McRecoverQApplication::IconFromTheme(QLatin1String("edit-find")));
	q->actionSave->setIcon(
		McRecoverQApplication::IconFromTheme(QLatin1String("document-save")));
	q->actionSaveAll->setIcon(
		McRecoverQApplication::IconFromTheme(QLatin1String("document-save-all")));
	q->actionAbout->setIcon(
		McRecoverQApplication::IconFromTheme(QLatin1String("help-about")));

	// Disable save actions by default.
	q->actionSave->setEnabled(false);
	q->actionSaveAll->setEnabled(false);

	// Initialize the Memory Card Toolbar.
	mcToolbar = new QToolBar(q->tr("Memory Card"), q);
	mcToolbar->setToolButtonStyle(Qt::ToolButtonIconOnly);

	// Win32 style has a border. Get rid of it.
	mcToolbar->setStyleSheet(QLatin1String("QToolBar { border: none }"));

	// Add actions to the toolbar.
	mcToolbar->addAction(q->actionOpen);
	mcToolbar->addAction(q->actionScan);
	mcToolbar->addAction(q->actionSave);
	mcToolbar->addAction(q->actionSaveAll);

	// Make sure the "About" button is right-aligned.
	QWidget *spacer = new QWidget(q);
	spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	mcToolbar->addWidget(spacer);
	mcToolbar->addAction(q->actionAbout);

	// Add the toolbar to the Memory Card information box.
	q->vboxMemCardInfo->addWidget(mcToolbar);
}


/**
 * Update the "enabled" status of the QActions.
 */
void McRecoverWindowPrivate::updateActionEnableStatus(void)
{
	if (!card) {
		// No memory card image is loaded.
		q->actionClose->setEnabled(false);
		q->actionScan->setEnabled(false);
		q->actionSave->setEnabled(false);
		q->actionSaveAll->setEnabled(false);
	} else {
		// Memory card image is loaded.
		// TODO: Disable open, scan, and save (all) if we're scanning.
		q->actionClose->setEnabled(true);
		q->actionScan->setEnabled(true);
		q->actionSave->setEnabled(
			q->lstFileList->selectionModel()->hasSelection());
		q->actionSaveAll->setEnabled(card->numFiles() > 0);
	}
}


/**
 * Update the window title.
 */
void McRecoverWindowPrivate::updateWindowTitle(void)
{
	QString windowTitle;
	if (card) {
		windowTitle += displayFilename;
		windowTitle += QLatin1String(" - ");
	}
	windowTitle += QApplication::applicationName();

	q->setWindowTitle(windowTitle);
}


/** McRecoverWindow **/

McRecoverWindow::McRecoverWindow(QWidget *parent)
	: QMainWindow(parent)
	, d(new McRecoverWindowPrivate(this))
{
	setupUi(this);

	// Make sure the window is deleted on close.
	this->setAttribute(Qt::WA_DeleteOnClose, true);

#ifdef Q_WS_MAC
	// Remove the window icon. (Mac "proxy icon")
	// TODO: Use the memory card file?
	this->setWindowIcon(QIcon());
#endif /* Q_WS_MAC */

	// Set up the QSplitter sizes.
	// We want the card info panel to be 160px wide at startup.
	// TODO: Save positioning settings somewhere?
	static const int MemCardInfoPanelWidth = 256;
	QList<int> sizes;
	sizes.reserve(2);
	sizes.append(this->width() - MemCardInfoPanelWidth);
	sizes.append(MemCardInfoPanelWidth);
	splitter->setSizes(sizes);

	// Set the splitter stretch factors.
	// We want the QTreeView to stretch, but not the card info panel.
	splitter->setStretchFactor(0, 1);
	splitter->setStretchFactor(1, 0);

	// Set lstFileList's model.
	lstFileList->setModel(d->model);

	// Don't expand the last header column to fill the QTreeView.
	lstFileList->header()->setStretchLastSection(false);

	// Connect the lstFileList slots.
	connect(lstFileList->selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),
		this, SLOT(lstFileList_selectionModel_currentRowChanged(QModelIndex,QModelIndex)));

	// Initialize the UI.
	d->updateLstFileList();
	d->initMcToolbar();
	d->statusBarManager = new StatusBarManager(Ui_McRecoverWindow::statusBar, this);
	d->updateWindowTitle();
}

McRecoverWindow::~McRecoverWindow()
{
	delete d;
}


/**
 * Open a GameCube Memory Card image.
 * @param filename Filename.
 */
void McRecoverWindow::open(QString filename)
{
	if (d->card) {
		d->model->setMemCard(NULL);
		mcCardView->setCard(NULL);
		mcfFileView->setFile(NULL);
		delete d->card;
	}

	// Open the specified memory card image.
	d->card = new MemCard(filename);
	// TODO: Make sure the card is open.
	d->model->setMemCard(d->card);
	d->filename = filename;

	// Extract the filename from the path.
	d->displayFilename = filename;
	int lastSlash = d->displayFilename.lastIndexOf(QChar(L'/'));
	if (lastSlash >= 0)
		d->displayFilename.remove(0, lastSlash + 1);

	// Set the MemCardView's MemCard to the
	// selected card in the QTreeView.
	mcCardView->setCard(d->card);

	// Update the UI.
	d->updateLstFileList();
	d->statusBarManager->opened(filename);
	d->updateWindowTitle();

	// FIXME: If a file is opened from the command line,
	// QTreeView sort-of selects the first file.
	// (Signal is emitted, but nothing is highlighted.)
}


/**
 * Close the currently-opened GameCube Memory Card image.
 */
void McRecoverWindow::close(void)
{
	d->model->setMemCard(NULL);
	mcCardView->setCard(NULL);
	mcfFileView->setFile(NULL);
	delete d->card;
	d->card = NULL;

	// Clear the filenames.
	d->filename.clear();
	d->displayFilename.clear();

	// Update the UI.
	d->updateLstFileList();
	d->statusBarManager->closed();
	d->updateWindowTitle();
}


/**
 * An item is being dragged onto the window.
 * @param event QDragEnterEvent describing the item.
 */
void McRecoverWindow::dragEnterEvent(QDragEnterEvent *event)
{
	if (!event->mimeData()->hasUrls())
		return;

	// One or more URls have been dragged onto the window.
	const QList<QUrl>& lstUrls = event->mimeData()->urls();
	if (lstUrls.size() != 1) {
		// More than one file has been dragged onto the window.
		// TODO: Add support for this; open one window per file.
		return;
	}

	// One URL has been dragged onto the window.
	const QUrl& url = lstUrls.at(0);

	// Make sure the URL is file://.
	// TODO: Add support for other protocols later.
	if (url.scheme() != QLatin1String("file"))
		return;

	// Override the proposed action with Copy, and accept it.
	event->setDropAction(Qt::CopyAction);
	event->accept();
}


/**
 * An item has been dropped onto the window.
 * @param event QDropEvent describing the item.
 */
void McRecoverWindow::dropEvent(QDropEvent *event)
{
	if (!event->mimeData()->hasUrls())
		return;

	// One or more URls have been dragged onto the window.
	const QList<QUrl>& lstUrls = event->mimeData()->urls();
	if (lstUrls.size() != 1) {
		// More than one file has been dropped onto the window.
		// TODO: Add support for this; open one window per file.
		return;
	}

	// One URL has been dropped onto the window.
	const QUrl& url = lstUrls.at(0);
	
	// Make sure the URL is file://.
	// TODO: Add support for other protocols later.
	if (url.scheme() != QLatin1String("file"))
		return;

	// Get the local filename.
	// NOTE: url.toLocalFile() returns an empty string if this isn't file://,
	// but we're already checking for file:// above...
	QString filename = url.toLocalFile();
	if (filename.isEmpty())
		return;

	// Override the proposed action with Copy, and accept it.
	event->setDropAction(Qt::CopyAction);
	event->accept();

	// Open the memory card file.
	open(filename);
}


/**
 * Open a memory card image.
 */
void McRecoverWindow::on_actionOpen_triggered(void)
{
	// TODO: Set the default filename.
	QString filename = QFileDialog::getOpenFileName(this,
			tr("Open GameCube Memory Card Image"),	// Dialog title
			QString(),				// Default filename
			tr("GameCube Memory Card Image") + QLatin1String(" (*.raw);;") +
			tr("All Files") + QLatin1String(" (*)"));

	if (filename.isEmpty())
		return;

	// Open the memory card file.
	open(filename);
}


/**
 * Close the currently-opened memory card image.
 */
void McRecoverWindow::on_actionClose_triggered(void)
{
	if (!d->card)
		return;

	close();
}


/**
 * Scan for lost files.
 */
void McRecoverWindow::on_actionScan_triggered(void)
{
	if (!d->card)
		return;

	// Load the database.
	// TODO:
	// - Only if the database is not loaded,
	//   or if the database file has been changed.
	int ret = d->searchThread->loadGcnMcFileDb(QLatin1String("GcnMcFileDb.xml"));
	if (ret != 0)
		return;

	// Remove lost files from the card.
	d->card->removeLostFiles();

	// Update the status bar manager.
	d->statusBarManager->setSearchThread(d->searchThread);

	// Search blocks for lost files.
	// TODO: Handle errors.
	ret = d->searchThread->searchMemCard_async(d->card);
	if (ret < 0) {
		// Error starting the thread.
		// Use the synchronous version.
		// TODO: Handle errors.
		// NOTE: Files will be added by searchThread_searchFinished_slot().
		ret = d->searchThread->searchMemCard(d->card);
	}
}


/**
 * Exit the program.
 * TODO: Separate close/exit for Mac OS X?
 */
void McRecoverWindow::on_actionExit_triggered(void)
{
	this->close();
}


/**
 * Show the About dialog.
 */
void McRecoverWindow::on_actionAbout_triggered(void)
{
	AboutDialog::ShowSingle();
}


void McRecoverWindow::memCardModel_layoutChanged(void)
{
	// Update the QTreeView columns, etc.
	// FIXME: This doesn't work the first time a file is added...
	// (possibly needs a dataChanged() signal)
	d->updateLstFileList();
}


void McRecoverWindow::memCardModel_rowsInserted(void)
{
	// A new file entry was added to the MemCard.
	// Update the QTreeView columns.
	// FIXME: This doesn't work the first time a file is added...
	d->updateLstFileList();
}


/**
 * Search has completed.
 * @param lostFilesFound Number of "lost" files found.
 */
void McRecoverWindow::searchThread_searchFinished_slot(int lostFilesFound)
{
	Q_UNUSED(lostFilesFound)

	// Remove lost files from the card.
	d->card->removeLostFiles();

	// Get the files found list.
	QLinkedList<SearchData> filesFoundList = d->searchThread->filesFoundList();

	// Add the directory entries.
	foreach (const SearchData &searchData, filesFoundList) {
		MemCardFile *file = d->card->addLostFile(&(searchData.dirEntry), searchData.fatEntries);

		// TODO: Add ChecksumData parameter to addLostFile.
		// Alternatively, add SearchData overload?
		if (file)
			file->setChecksumDefs(searchData.checksumDefs);
	}
}


/**
 * lstFileList selectionModel: Current row selection has changed.
 * @param current Current index.
 * @param previous Previous index.
 */
void McRecoverWindow::lstFileList_selectionModel_currentRowChanged(
	const QModelIndex& current, const QModelIndex& previous)
{
	Q_UNUSED(previous)

	// If file(s) are selected, enable the Save action.
	// FIXME: Selection model is empty due to the initial selection bug
	// that happens if a file was specified on the command line.
	//actionSave->setEnabled(lstFileList->selectionModel()->hasSelection());
	actionSave->setEnabled(current.row() >= 0);

	// Set the MemCardFileView's MemCardFile to the
	// selected file in the QTreeView.
	mcfFileView->setFile(d->card->getFile(current.row()));
}
