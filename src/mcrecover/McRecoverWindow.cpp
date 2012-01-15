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

// git version
#include "git.h"

McRecoverWindow::McRecoverWindow(QWidget *parent)
	: QMainWindow(parent)
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
}

McRecoverWindow::~McRecoverWindow()
{ }


/**
 * Open a GameCube Memory Card file.
 * @param filename Filename.
 */
void McRecoverWindow::open(QString filename)
{
	MemCard *card = new MemCard(filename);
	MemCardModel *model = new MemCardModel(this);
	model->setMemCard(card);
	lstMemCard->setModel(model);
	
	// Resize the columns to fit the contents.
	for (int i = 0; i < model->columnCount(); i++)
		lstMemCard->resizeColumnToContents(i);
	
	// Update the group box title.
	if (!card)
		grpMemCard->setTitle(tr("No memory card loaded."));
	else
	{
		// Extract the filename from the path.
		int lastSlash = filename.lastIndexOf(QChar(L'/'));
		if (lastSlash >= 0)
			filename.remove(0, lastSlash + 1);
		
		// NOTE: 5 blocks are subtracted here in order to
		// show the user-visible space, e.g. 59 or 251 blocks
		// instead of 64 or 256 blocks.
		grpMemCard->setTitle(
			tr("%1: %2 block(s) (%3 free)")
				.arg(filename)
				.arg(card->sizeInBlocks() - 5)
				.arg(card->freeBlocks())
			);
	}
}
