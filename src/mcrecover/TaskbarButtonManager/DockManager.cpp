/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * DockManager.cpp: DockManager D-Bus implementation.                       *
 *                                                                         *
 * Copyright (c) 2013 by David Korth.                                      *
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

#include "DockManager.hpp"

// Qt includes.
#include <QtCore/QTimer>
#include <QtGui/QWidget>

// QtDBus includes.
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusReply>

// QtDBus metatypes.
#include "dbus/DBusMetatypes.hpp"

// DockManager interfaces.
#include "dockmanagerinterface.h"
#include "dockiteminterface.h"

// C includes.
#include <sys/types.h>
#include <unistd.h>

class DockManagerPrivate
{
	public:
		DockManagerPrivate(DockManager *const q);
		~DockManagerPrivate();

		/**
		 * Close all DockManager connections.
		 */
		void close(void);

		/**
		 * Connect to the DockManager.
		 * @return 0 on success; non-zero on error.
		 */
		int connectToDockManager(void);

		/**
		 * Update the DockItem status.
		 * DockItem must be connected.
		 */
		void update(void);

	private:
		DockManager *const q;
		Q_DISABLE_COPY(DockManagerPrivate);

	private:
		// DockManager.
		NetLaunchpadDockManagerInterface *ifDockManager;
		NetLaunchpadDockItemInterface *ifDockItem;

	public:
		// Window.
		QWidget *window;

		// Status elements.
		int progressCurrent;	// Current progress. (-1 for no bar)
		int progressMax;	// Maximum progress.
};


DockManagerPrivate::DockManagerPrivate(DockManager *const q)
	: q(q)
	, ifDockManager(nullptr)
	, ifDockItem(nullptr)
	, window(nullptr)
	, progressCurrent(0)
	, progressMax(100)
{
	// Make sure the DBus metatypes are registered.
	registerDBusMetatypes();
}


/**
 * Close all DockManager connections.
 */
void DockManagerPrivate::close(void)
{
	delete ifDockItem;
	ifDockItem = nullptr;
	delete ifDockManager;
	ifDockManager = nullptr;
}


/**
 * Connect to the DockManager.
 * @return 0 on success; non-zero on error.
 */
int DockManagerPrivate::connectToDockManager(void)
{
	// Close existing connections first.
	close();

	// If we don't have a window specified, don't do anything.
	if (!window)
		return -1;

	// Get the session bus.
	QDBusConnection bus = QDBusConnection::sessionBus();

	// Connect to the DockManager over D-Bus.
	ifDockManager = new NetLaunchpadDockManagerInterface(
				QLatin1String("net.launchpad.DockManager"),
				QLatin1String("/net/launchpad/DockManager"),
				bus, q);
	if (!ifDockManager->isValid()) {
		// Cannot connect to the DockManager.
		delete ifDockManager;
		ifDockManager = nullptr;
		return 1;
	}

	// DockManager connected.
	// Get the DockItem for the window.
	// NOTE: GetItemByXid() is broken when using KDE's IconTasks.
	// Use GetItemsByPid() instead, and take the first entry.
	// TODO: Implement hacks from Quassel?
	// https://github.com/agaida/quassel/blob/master/src/qtui/dockmanagernotificationbackend.cpp
	// TODO: How do we handle multiple windows per application?
	pid_t pid = getpid();
	QDBusPendingReply<QList<QDBusObjectPath> > reply = ifDockManager->GetItemsByPid(pid);
	reply.waitForFinished();
	if (!reply.isValid()) {
		// Cannot find the DockItem for this window.
		close();
		return 2;
	}

	QList<QDBusObjectPath> paths = reply.value();
	// TODO: Find the correct path.
	if (paths.isEmpty()) {
		// No paths.
		return 3;
	}

	ifDockItem = new NetLaunchpadDockItemInterface(
			QLatin1String("net.launchpad.DockManager"),
			paths.at(0).path(), bus, q);
	if (!ifDockItem->isValid()) {
		// Cannot connect to DockItem.
		close();
		return 4;
	}

	// DockItem connected.
	// Update the status.
	update();
	return 0;
}


/**
 * Update the DockItem status.
 * DockItem must be connected.
 */
void DockManagerPrivate::update(void)
{
	if (!ifDockItem)
		return;

	// Update the DockItem.
	QVariantMap dockItemProps;

	// Progress.
	int progress;
	if (progressCurrent < 0 || progressMax <= 0) {
		progress = -1;
	} else if (progressCurrent >= progressMax) {
		progress = 100;
	} else {
		progress = (int)(((float)progressCurrent / (float)progressMax) * 100);
	}
	dockItemProps[QLatin1String("progress")] = progress;

	ifDockItem->UpdateDockItem(dockItemProps);
}


DockManagerPrivate::~DockManagerPrivate()
{
	// Close all DockManager connections.
	close();
}


/** DockManager **/


DockManager::DockManager(QObject* parent)
	: QObject(parent)
	, d(new DockManagerPrivate(this))
{

}

DockManager::~DockManager()
{
	delete d;
}


/**
 * Get the window this DockManager is managing.
 * @return Window.
 */
QWidget *DockManager::window(void)
	{ return d->window; }

/**
 * Set the window this DockManager should manage.
 * This must be a top-level window in order to work properly.
 * @param window Window.
 */
void DockManager::setWindow(QWidget *window)
{
	if (d->window) {
		// Disconnect slots from the existing window.
		disconnect(d->window, SIGNAL(destroyed(QObject*)),
			   this, SLOT(windowDestroyed_slot(QObject*)));
		d->close();
	}

	d->window = window;

	if (d->window) {
		// Connect slots to the new window.
		connect(d->window, SIGNAL(destroyed(QObject*)),
			this, SLOT(windowDestroyed_slot(QObject*)));

		// Connect to the DockManager.
		// HACK: Make sure it's after the window is initialized.
		QTimer::singleShot(100, this, SLOT(setWindow_timer_slot()));
	}
}


/**
 * Clear the progress bar.
 */
void DockManager::clearProgressBar(void)
{
	d->progressCurrent = -1;
	d->progressMax = -1;
	d->update();
}

/**
 * Set the progress bar value.
 * @param current Current progress.
 * @param max Maximum progress.
 */
void DockManager::setProgressBar(int current, int max)
{
	d->progressCurrent = current;
	d->progressMax = max;
	d->update();
}


/** Slots **/


/**
 * Window we're managing was destroyed.
 * @param obj QObject that was destroyed.
 */
void DockManager::windowDestroyed_slot(QObject *obj)
{
	if (!d->window || !obj)
		return;

	if (obj == d->window) {
		// This was our window.
		d->window = nullptr;
		d->close();
	}
}


/**
 * HACK: Timer for window initialization.
 * If we attempt to get the dock item before the
 * window is fully initialized, we won't find it.
 */
void DockManager::setWindow_timer_slot(void)
	{ d->connectToDockManager(); }
