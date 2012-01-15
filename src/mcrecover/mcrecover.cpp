/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * mcrecover.cpp: Main program.                                            *
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
//#include "MemCard.hpp"
//#include "MemCardFile.hpp"

// C includes.
#include <stdio.h>
#include <stdlib.h>

// Qt includes.
#include <QtGui/QApplication>

int main(int argc, char *argv[])
{
	QApplication *mcApp = new QApplication(argc, argv);
	
	if (argc < 2)
	{
		printf("Usage: %s [filename]\n", argv[0]);
		return EXIT_FAILURE;
	}
	
	// Initialize the McRecoverWindow.
	McRecoverWindow *mcRecoverWindow = new McRecoverWindow();
	mcRecoverWindow->open(QString::fromLocal8Bit(argv[1]));
	mcRecoverWindow->show();
	
	// Run the Qt4 UI.
	return mcApp->exec();
	
#if 0
	// Open the memory card file.
	MemCard *card = new MemCard(QString::fromUtf8(argv[1]));
	if (!card->isOpen())
	{
		printf("Cannot open file. (TODO: Get actual error!)\n");
		return EXIT_FAILURE;
	}
	
	const int numFiles = card->numFiles();
	printf("Number of files: %d\n\n", numFiles);
	
	for (int i = 0; i < numFiles; i++)
	{
		MemCardFile *file = card->getFile(i);
		if (!file)
		{
			printf("ERROR getting file %d.\n\n", i);
			continue;
		}
		
		// Print file information.
		printf("File %d:\n", i);
		printf("- Game code: %s%s\n",
			file->gamecode().toUtf8().constData(),
			file->company().toUtf8().constData());
		printf("- Filename: %s\n", file->filename().toUtf8().constData());
		printf("- Last modified: %s\n", file->lastModified().toString(Qt::DefaultLocaleLongDate).toUtf8().constData());
		printf("- Game Description: %s\n", file->gameDesc().toUtf8().constData());
		printf("- File Description: %s\n", file->fileDesc().toUtf8().constData());
		
		uint8_t permission = file->permission();
		printf("- Permission: %c%c%c%c\n",
			((permission & CARD_ATTRIB_GLOBAL) ? 'G' : '-'),
			((permission & CARD_ATTRIB_NOMOVE) ? 'M' : '-'),
			((permission & CARD_ATTRIB_NOCOPY) ? 'C' : '-'),
			((permission & CARD_ATTRIB_PUBLIC) ? 'P' : '-'));
		
		printf("- Size: %d block(s) (%d KB)\n",
			file->size(),
			((file->size() * card->blockSize()) / 1024));
		
		printf("\n");
	}
	
	delete card;
#endif
}
