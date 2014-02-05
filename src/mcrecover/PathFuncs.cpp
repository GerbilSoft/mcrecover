/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * PathFuncs.cpp: Path functions.                                          *
 *                                                                         *
 * Copyright (c) 2014 by David Korth.                                      *
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
