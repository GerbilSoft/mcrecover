/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * PathFuncs.cpp: Path functions.                                          *
 *                                                                         *
 * Copyright (c) 2014 by David Korth.                                      *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#include "PathFuncs.hpp"

// Qt includes.
#include <QtCore/QDir>

/**
 * Make a path relative to a base path it's contained in.
 * @param basePath Base path.
 * @param filePath Path to make relative to basePath.
 * @param prefix Prefix to use for relative paths.
 * @return Relative path, or absolute path if filePath is not contained within basePath.
 */
QString PathFuncs::makeRelative(const QString &basePath, const QString &filePath,
				const QString &prefix)
{
	QDir baseDir = QDir(basePath);
	QDir fileDir = QDir(filePath);

	if (baseDir.isRelative() || fileDir.isRelative()) {
		// Relative path. Don't do anything.
		return filePath;
	}

	bool fileIsInBase = false;
	do {
		if (fileDir == baseDir) {
			fileIsInBase = true;
			break;
		}

		fileDir.cdUp();
	} while (!fileDir.isRoot());

	return (fileIsInBase
		? (prefix + baseDir.relativeFilePath(filePath))
		: filePath);
}
