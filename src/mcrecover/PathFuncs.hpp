/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * PathFuncs.hpp: Path functions.                                          *
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

#ifndef __MCRECOVER_PATHFUNCS_HPP__
#define __MCRECOVER_PATHFUNCS_HPP__

// Qt includes.
#include <QtCore/qglobal.h>
#include <QtCore/QString>
#include <QtCore/QCoreApplication>
#include <QtCore/QDir>

/**
 * PathFuncs: Pathname functions.
 * NOTE: This is a static class.
 */
class PathFuncs
{
	private:
		PathFuncs();
		~PathFuncs();
	private:
		Q_DISABLE_COPY(PathFuncs);

	public:
		/**
		 * Make a path relative to a base path it's contained in.
		 * @param basePath Base path.
		 * @param filePath Path to make relative to basePath.
		 * @param prefix Prefix to use for relative paths.
		 * @return Relative path, or absolute path if filePath is not contained within basePath.
		 */
		static QString makeRelative(const QString &basePath, const QString &filePath,
					    const QString &prefix = QLatin1String("./"));

		/**
		 * Make a path relative to the application directory.
		 * @param filePath Path to make relative to the application directory. (MUST BE ABSOLUTE!)
		 * @return Relative path, or absolute path if filePath is not contained within the application directory.
		 */
		static inline QString makeRelativeToApplication(const QString &filePath)
		{
			return makeRelative(QCoreApplication::applicationDirPath(), filePath);
		}

		/**
		 * Make a path relative to the user's home directory.
		 * @param filePath Path to make relative to the user's home directory. (MUST BE ABSOLUTE!)
		 * @return Relative path, or absolute path if filePath is not contained within the user's home directory.
		 */
		static inline QString makeRelativeToHome(const QString &filePath)
		{
			return makeRelative(QDir::home().absolutePath(),
					    filePath, QLatin1String("~/"));
		}
};

#endif /* __MCRECOVER_PATHFUNCS_HPP__ */
