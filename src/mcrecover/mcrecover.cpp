/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * mcrecover.cpp: Main program.                                            *
 *                                                                         *
 * Copyright (c) 2011-2018 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#include "mcrecover.hpp"

#include "windows/McRecoverWindow.hpp"

// C includes.
#include <stdio.h>
#include <stdlib.h>

// Qt includes.
#include "McRecoverQApplication.hpp"
#include <QtCore/QDir>

/**
 * Main entry point.
 * @param argc Number of arguments.
 * @param argv Array of arguments.
 * @return Return value.
 */
int mcrecover_main(int argc, char *argv[])
{
	// Enable High DPI.
	McRecoverQApplication::setAttribute(Qt::AA_UseHighDpiPixmaps, true);
#if QT_VERSION >= 0x050600
	// Enable High DPI pixmaps.
	McRecoverQApplication::setAttribute(Qt::AA_EnableHighDpiScaling, true);
#else
	// Hardcode the value in case the user upgrades to Qt 5.6 later.
	// http://doc.qt.io/qt-5/qt.html#ApplicationAttribute-enum
	McRecoverQApplication::setAttribute((Qt::ApplicationAttribute)13, true);
#endif /* QT_VERSION >= 0x050600 */

	McRecoverQApplication *mcApp = new McRecoverQApplication(argc, argv);

	// Initialize the McRecoverWindow.
	McRecoverWindow *mcRecoverWindow = new McRecoverWindow();

	// If a filename was specified, open it.
	QStringList args = mcApp->arguments();
	if (args.size() >= 2) {
		mcRecoverWindow->openCard(QDir::fromNativeSeparators(args.at(1)));
	}

	// Show the window.
	mcRecoverWindow->show();

	// Run the Qt4 UI.
	return mcApp->exec();
}
