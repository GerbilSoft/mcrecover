/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * DockManager.hpp: DockManager D-Bus implementation.                      *
 *                                                                         *
 * Copyright (c) 2013-2016 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#include "DockManager.hpp"

// Qt includes.
#include <QtCore/QTimer>
#include <QWidget>

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

/** DockManagerPrivate **/

#include "TaskbarButtonManager_p.hpp"
class DockManagerPrivate : public TaskbarButtonManagerPrivate
{
	public:
		explicit DockManagerPrivate(DockManager *const q);
		virtual ~DockManagerPrivate();

	private:
		typedef TaskbarButtonManagerPrivate super;
		Q_DECLARE_PUBLIC(DockManager)
		Q_DISABLE_COPY(DockManagerPrivate)

	public:
		/**
		 * Close all DockManager connections.
		 */
		void close(void);

		/**
		 * Get the DockManager interface.
		 * @param parent Parent for the DockManager interface object.
		 * @return DockManager interface, or nullptr on error.
		 */
		static net::launchpad::DockManager *GetDockManagerInterface(QObject *parent = 0);

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
		// DockManager.
		net::launchpad::DockManager *ifDockManager;
		net::launchpad::DockItem *ifDockItem;
};

DockManagerPrivate::DockManagerPrivate(DockManager *const q)
	: super(q)
	, ifDockManager(nullptr)
	, ifDockItem(nullptr)
{
	// Make sure the DBus metatypes are registered.
	registerDBusMetatypes();
}


DockManagerPrivate::~DockManagerPrivate()
{
	// Close all DockManager connections.
	close();
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
 * Get the DockManager interface.
 * @param parent Parent for the DockManager interface object.
 * @return DockManager interface, or nullptr on error.
 */
net::launchpad::DockManager *DockManagerPrivate::GetDockManagerInterface(QObject *parent)
{
	// Get the session bus.
	QDBusConnection bus = QDBusConnection::sessionBus();

	// Connect to the DockManager over D-Bus.
	net::launchpad::DockManager *ifDockManager;
	ifDockManager = new net::launchpad::DockManager(
				QLatin1String("net.launchpad.DockManager"),
				QLatin1String("/net/launchpad/DockManager"),
				bus, parent);
	if (!ifDockManager->isValid()) {
		// Cannot connect to the DockManager.
		delete ifDockManager;
		ifDockManager = nullptr;
	}

	return ifDockManager;
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
	if (!this->window)
		return -1;

	// Get the session bus.
	QDBusConnection bus = QDBusConnection::sessionBus();

	// Connect to the DockManager over D-Bus.
	Q_Q(DockManager);
	ifDockManager = GetDockManagerInterface(q);
	if (!ifDockManager)
		return 1;

	// DockManager connected.
	// Get the DockItem for the window.
	// NOTE: GetItemByXid() is broken when using KDE's IconTasks.
	// Use GetItemsByPid() instead, and take the first entry.
	// TODO: Implement hacks from Quassel?
	// https://github.com/quassel/quassel/blob/master/src/qtui/dockmanagernotificationbackend.cpp
	// TODO: How do we handle multiple windows per application?
	QDBusPendingReply<QList<QDBusObjectPath> > reply =
		ifDockManager->GetItemsByPid(QCoreApplication::applicationPid());
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

	ifDockItem = new net::launchpad::DockItem(
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
	if (!ifDockItem->isValid()) {
		close();
		return;
	}

	// Update the DockItem.
	QVariantMap dockItemProps;

	// Progress.
	int progress;
	int curVal = this->progressBarValue;
	int curMax = this->progressBarMax;
	if (curVal < 0 || curMax <= 0) {
		progress = -1;
	} else if (curVal >= curMax) {
		progress = 100;
	} else {
		progress = (int)(((float)curVal / (float)curMax) * 100);
	}
	dockItemProps[QLatin1String("progress")] = progress;

	ifDockItem->UpdateDockItem(dockItemProps);
}

/** DockManager **/

DockManager::DockManager(QObject* parent)
	: super(new DockManagerPrivate(this), parent)
{ }

/**
 * Is this TaskbarButtonManager usable?
 * @return True if usable; false if not.
 */
bool DockManager::IsUsable(void)
{
	net::launchpad::DockManager *ifDockManager =
		DockManagerPrivate::GetDockManagerInterface();
	bool isUsable = !!ifDockManager;
	delete ifDockManager;
	return isUsable;
}

/**
 * Set the window this TaskbarButtonManager should manage.
 * This must be a top-level window in order to work properly.
 *
 * Subclasses should reimplement this function if special
 * initialization is required to set up the taskbar button.
 *
 * TODO: Make a separate protected function that setWindow() calls?
 *
 * @param window Window.
 */
void DockManager::setWindow(QWidget *window)
{
	Q_D(DockManager);

	// Disconnect any existing connections.
	d->close();

	// Set the new window.
	super::setWindow(window);

	if (window != nullptr) {
		// Connect to the DockManager.
		// HACK: Make sure it's after the window is initialized.
		QTimer::singleShot(100, this, SLOT(setWindow_timer_slot()));
	}
}

/**
 * Update the taskbar button.
 */
void DockManager::update(void)
{
	Q_D(DockManager);
	d->update();
}

/** Slots **/

/**
 * HACK: Timer for window initialization.
 * If we attempt to get the dock item before the
 * window is fully initialized, we won't find it.
 */
void DockManager::setWindow_timer_slot(void)
{
	Q_D(DockManager);
	d->connectToDockManager();
}
