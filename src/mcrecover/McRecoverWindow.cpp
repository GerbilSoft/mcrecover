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

// Qt includes.
#include <QtCore/QUrl>
#include <QtCore/QStack>
#include <QtCore/QVector>
#include <QtCore/QFile>
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
		 * Retranslate the Memory Card toolbar.
		 */
		void retranslateMcToolbar(void);

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

		/**
		 * Save the specified file(s).
		 * @param files List of file(s) to save.
		 * @param path If specified, save file(s) to path using default GCI filenames.
		 */
		void saveFiles(QVector<MemCardFile*> files, QString path = QString());
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

	// Initialize the Memory Card toolbar.
	mcToolbar = new QToolBar(q);
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

	// Retranslate the Memory Card toolbar.
	retranslateMcToolbar();

	// Add the toolbar to the Memory Card information box.
	q->vboxMemCardInfo->addWidget(mcToolbar);
}


/**
 * Retranslate the Memory Card toolbar.
 */
void McRecoverWindowPrivate::retranslateMcToolbar(void)
{
	if (mcToolbar)
		mcToolbar->setWindowTitle(q->tr("Memory Card"));
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


/**
 * Save the specified file(s).
 * @param files List of file(s) to save.
 * @param path If specified, save file(s) to path using default GCI filenames.
 */
void McRecoverWindowPrivate::saveFiles(const QVector<MemCardFile*> files, QString path)
{
	if (files.isEmpty()) {
		return;
	} else if (files.size() == 1 && path.isEmpty()) {
		// Single file, path not specified.
		MemCardFile *file = files.at(0);

		// Prompt the user for a save location.
		QString filename = QFileDialog::getSaveFileName(q,
				q->tr("Save GCN Save File %1")
					.arg(file->filename()),	// Dialog title
				file->defaultGciFilename(),	// Default filename
				q->tr("GameCube Save Files") + QLatin1String(" (*.gci);;") +
				q->tr("All Files") + QLatin1String(" (*)"));
		if (filename.isEmpty())
			return;

		int filesSaved = 0;

		// Save the file.
		int ret = file->saveGci(filename);
		if (ret == 0) {
			// File saved successfully.
			filesSaved++;
		} else {
			// An error occurred while saving the file.
			// TODO: Error details.
		}

		// Update the status bar.
		const QFileInfo fileInfo(filename);
		const QDir dir = fileInfo.dir();
		QString absolutePath = dir.absolutePath();

		// Make sure tha path has a trailing slash.
		if (!absolutePath.isEmpty() &&
		    absolutePath.at(absolutePath.size() - 1) != QChar(L'/'))
		{
			absolutePath += QChar(L'/');
		}

		statusBarManager->filesSaved(filesSaved, absolutePath);
		return;
	} else if (files.size() > 1 && path.isEmpty()) {
		// Multiple files, path not specified.
		// Prompt the user for a save location.
		path = QFileDialog::getExistingDirectory(q,
				q->tr("Save %Ln GCN Save File(s)", NULL, files.size()));
		if (path.isEmpty())
			return;
	}

	// Save files using default filenames to the specified path.
	enum OverwriteAllStatus {
		OVERWRITEALL_UNKNOWN	= 0,
		OVERWRITEALL_YESTOALL	= 1,
		OVERWRITEALL_NOTOALL	= 2,
	};

	int filesSaved = 0;
	OverwriteAllStatus overwriteAll = OVERWRITEALL_UNKNOWN;
	foreach (MemCardFile *file, files) {
		const QString defaultGciFilename = file->defaultGciFilename();
		QString filename = path + QChar(L'/') + defaultGciFilename;
		QFile qfile(filename);

		// Check if the file exists.
		if (qfile.exists()) {
			if (overwriteAll == OVERWRITEALL_UNKNOWN) {
				bool overwrite = false;
				int ret = QMessageBox::warning(q,
					q->tr("File Already Exists"),
					q->tr("A file named \"%1\" already exists in the specified directory.")
						.arg(filename) +
					QLatin1String("\n\n") +
					q->tr("Do you want to overwrite it?"),
					(QMessageBox::Yes | QMessageBox::No |
						QMessageBox::YesToAll | QMessageBox::NoToAll),
					QMessageBox::No);
				switch (ret) {
					case QMessageBox::Yes:
						// Overwrite this file.
						overwrite = true;
						break;

					default:
					case QMessageBox::No:
					case QMessageBox::Escape:
						// Don't overwrite this file.
						overwrite = false;
						break;

					case QMessageBox::YesToAll:
						// Overwrite this file and all other files.
						overwriteAll = OVERWRITEALL_YESTOALL;
						overwrite = true;
						break;

					case QMessageBox::NoToAll:
						// Don't overwrite this file or any other files.
						overwriteAll = OVERWRITEALL_NOTOALL;
						overwrite = false;
						break;
				}

				if (!overwrite)
					continue;
			} else if (overwriteAll == OVERWRITEALL_NOTOALL) {
				// Don't overwrite any files.
				continue;
			}
		}

		// Save the file.
		int ret = file->saveGci(filename);
		if (ret == 0) {
			// File saved successfully.
			filesSaved++;
		} else {
			// An error occurred while saving the file.
			// TODO: Error details.
		}
	}

	// Update the status bar.
	const QDir dir(path);
	QString absolutePath = dir.absolutePath();

	// Make sure tha path has a trailing slash.
	if (!absolutePath.isEmpty() &&
		absolutePath.at(absolutePath.size() - 1) != QChar(L'/'))
	{
		absolutePath += QChar(L'/');
	}

	statusBarManager->filesSaved(filesSaved, absolutePath);
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
 * Widget state has changed.
 * @param event State change event.
 */
void McRecoverWindow::changeEvent(QEvent *event)
{
	if (event->type() == QEvent::LanguageChange) {
		// Retranslate the UI.
		retranslateUi(this);
		d->updateLstFileList();
		d->retranslateMcToolbar();
		d->updateWindowTitle();
	}

	// Pass the event to the base class.
	this->QMainWindow::changeEvent(event);
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


/** Save actions. **/


/**
 * Save the selected file(s).
 */
void McRecoverWindow::on_actionSave_triggered(void)
{
	QModelIndexList selList = lstFileList->selectionModel()->selectedRows();
	if (selList.isEmpty())
		return;

	QVector<MemCardFile*> files;
	files.reserve(selList.size());

	foreach(QModelIndex idx, selList) {
		MemCardFile *file = d->card->getFile(idx.row());
		if (file != NULL)
			files.append(file);
	}

	// If there's no files to save, don't do anything.
	if (files.isEmpty())
		return;

	// Save the files.
	d->saveFiles(files);
}


/**
 * Save all files.
 */
void McRecoverWindow::on_actionSaveAll_triggered(void)
{
	if (!d->card)
		return;

	const int numFiles = d->card->numFiles();
	if (numFiles <= 0)
		return;

	QVector<MemCardFile*> files;
	files.reserve(numFiles);

	for (int i = 0; i < numFiles; i++) {
		MemCardFile *file = d->card->getFile(i);
		if (file != NULL)
			files.append(file);
	}

	// If there's no files to save, don't do anything.
	if (files.isEmpty())
		return;

	// Save the files.
	d->saveFiles(files);
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
