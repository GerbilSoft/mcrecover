/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * TaskbarButtonManagerFactory.hpp: TaskbarButtonManager factory class.    *
 *                                                                         *
 * Copyright (c) 2015-2016 by David Korth.                                 *
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

#include "TaskbarButtonManagerFactory.hpp"
#include "TaskbarButtonManager.hpp"

// TaskbarButtonManager subclasses.
#include <config.mcrecover.h>
#ifdef Q_OS_WIN32
#include "Win7TaskbarList.hpp"
#endif /* Q_OS_WIN32 */
#ifdef QtDBus_FOUND
#include "DockManager.hpp"
#endif /* QtDBus_FOUND */

/**
 * Create a TaskbarButtonManager.
 * @param parent Parent object.
 * @return System-specific TaskbarButtonManager, or nullptr on error.
 */
TaskbarButtonManager *TaskbarButtonManagerFactory::createManager(QObject *parent)
{
	// Check the various implementations.
#ifdef Q_OS_WIN
	if (Win7TaskbarList::IsUsable()) {
		// Win7TaskbarList is usable.
		return new Win7TaskbarList(parent);
	}
#endif /* Q_OS_WIN */
#ifdef QtDBus_FOUND
	if (DockManager::IsUsable()) {
		// DockManager is usable.
		return new DockManager(parent);
	}
#endif /* QtDBus_FOUND */

	// No TaskbarButtonManager subclasses are available.
	return nullptr;
}
