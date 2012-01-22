/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * McRecoverWindow.cpp: Main window.                                       *
 *                                                                         *
 * Copyright (c) 2011 by David Korth.                                      *
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

// MemCard classes.
#include "MemCard.hpp"
#include "MemCardFile.hpp"

// MemCard list model.
#include "MemCardModel.hpp"

// Qt includes. (Drag & Drop)
#include <QtGui/QDragEnterEvent>
#include <QtGui/QDropEvent>
#include <QtCore/QUrl>

// git version
#include "git.h"


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
		
		// Memory Card model for QListView.
		MemCardModel *model;
		
		// Filename.
		QString filename;
		
		/**
		 * Update the memory card group box's title.
		 */
		void updateGrpMemCardTitle(void);
};

McRecoverWindowPrivate::McRecoverWindowPrivate(McRecoverWindow *q)
	: q(q)
	, card(NULL)
	, model(new MemCardModel(q))
{
	// Only show icon, description, and size in the MemCardModel.
	model->setColumnVisible(MemCardModel::COL_ICON, true);
	model->setColumnVisible(MemCardModel::COL_BANNER, false);
	model->setColumnVisible(MemCardModel::COL_DESCRIPTION, true);
	model->setColumnVisible(MemCardModel::COL_SIZE, true);
	model->setColumnVisible(MemCardModel::COL_MTIME, false);
	model->setColumnVisible(MemCardModel::COL_PERMISSION, false);
	model->setColumnVisible(MemCardModel::COL_GAMECODE, false);
	model->setColumnVisible(MemCardModel::COL_FILENAME, false);
}

McRecoverWindowPrivate::~McRecoverWindowPrivate()
{
	delete model;
	delete card;
}


/**
 * Update the memory card group box's title.
 */
void McRecoverWindowPrivate::updateGrpMemCardTitle(void)
{
	if (!card)
		q->grpMemCard->setTitle(q->tr("No memory card loaded."));
	else
	{
		// Extract the filename from the path.
		QString displayFilename = filename;
		int lastSlash = displayFilename.lastIndexOf(QChar(L'/'));
		if (lastSlash >= 0)
			displayFilename.remove(0, lastSlash + 1);
		
		// NOTE: 5 blocks are subtracted here in order to
		// show the user-visible space, e.g. 59 or 251 blocks
		// instead of 64 or 256 blocks.
		q->grpMemCard->setTitle(
			q->tr("%1: %2 block(s) (%3 free)")
				.arg(displayFilename)
				.arg(card->sizeInBlocks() - 5)
				.arg(card->freeBlocks())
			);
	}
}


/** McRecoverWindow **/

McRecoverWindow::McRecoverWindow(QWidget *parent)
	: QMainWindow(parent)
	, d(new McRecoverWindowPrivate(this))
{
	setupUi(this);
	
#ifdef MCRECOVER_GIT_VERSION
	this->setWindowTitle(this->windowTitle() +
			QLatin1String(" (") +
			QLatin1String(MCRECOVER_GIT_VERSION) +
			QChar(L')'));
#endif
	
	// Make sure the window is deleted on close.
	this->setAttribute(Qt::WA_DeleteOnClose, true);
	
#ifdef Q_WS_MAC
	// Remove the window icon. (Mac "proxy icon")
	// TODO: Use the memory card file?
	this->setWindowIcon(QIcon());
#endif /* Q_WS_MAC */
	
	// Set lstMemCard's model.
	lstMemCard->setModel(d->model);
	
	// Initialize the memory card's group box title.
	d->updateGrpMemCardTitle();
}

McRecoverWindow::~McRecoverWindow()
{
	delete d;
}


/**
 * Open a GameCube Memory Card file.
 * @param filename Filename.
 */
void McRecoverWindow::open(QString filename)
{
	if (d->card)
	{
		d->model->setMemCard(NULL);
		delete d->card;
	}
	
	// Open the specified memory card file.
	d->card = new MemCard(filename);
	// TODO: Make sure the card is open.
	d->model->setMemCard(d->card);
	d->filename = filename;
	
	// Resize the columns to fit the contents.
	for (int i = 0; i < d->model->columnCount(); i++)
		lstMemCard->resizeColumnToContents(i);
	
	// Update the memory card's group box title.
	d->updateGrpMemCardTitle();
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
	if (lstUrls.size() != 1)
	{
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
	if (lstUrls.size() != 1)
	{
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
