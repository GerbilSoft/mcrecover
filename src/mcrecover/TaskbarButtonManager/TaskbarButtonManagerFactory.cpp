/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * TaskbarButtonManagerFactory.hpp: TaskbarButtonManager factory class.    *
 *                                                                         *
 * Copyright (c) 2015-2016 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#include "TaskbarButtonManagerFactory.hpp"
#include "TaskbarButtonManager.hpp"

// TaskbarButtonManager subclasses.
#include <config.mcrecover.h>
#ifdef Q_OS_WIN32
#include "Win7TaskbarList.hpp"
#endif /* Q_OS_WIN32 */
#if defined(Q_OS_UNIX) && !defined(Q_OS_MAC)
#include "UnityLauncher.hpp"
#endif
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
#if defined(Q_OS_UNIX) && !defined(Q_OS_MAC)
	// Unity Launcher
	if (UnityLauncher::IsUsable()) {
		// Unity Launcher is usable.
		return new UnityLauncher(parent);
	}
#endif /* defined(Q_OS_UNIX) && !defined(Q_OS_MAC) */
#ifdef QtDBus_FOUND
	if (DockManager::IsUsable()) {
		// DockManager is usable.
		return new DockManager(parent);
	}
#endif /* QtDBus_FOUND */

	// No TaskbarButtonManager subclasses are available.
	return nullptr;
}
