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
	
	// Initialize the McRecoverWindow.
	McRecoverWindow *mcRecoverWindow = new McRecoverWindow();
	
	// If a filename was specified, open it.
	if (argc > 1)
		mcRecoverWindow->open(QString::fromLocal8Bit(argv[1]));
	
	// Show the window.
	mcRecoverWindow->show();
	
	// Run the Qt4 UI.
	return mcApp->exec();
}
