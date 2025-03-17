/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * DockManager.cpp: DockManager D-Bus implementation.                      *
 *                                                                         *
 * Copyright (c) 2013-2025 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#pragma once

#include "TaskbarButtonManager.hpp"

class DockManagerPrivate;
class DockManager : public TaskbarButtonManager
{
	Q_OBJECT

	public:
		explicit DockManager(QObject *parent = 0);

	private:
		typedef TaskbarButtonManager super;
		Q_DECLARE_PRIVATE(DockManager)
		Q_DISABLE_COPY(DockManager)

	public:
		/**
		 * Is this TaskbarButtonManager usable?
		 * @return True if usable; false if not.
		 */
		static bool IsUsable(void);

	public:
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
		void setWindow(QWidget *window) final;

	protected:
		/**
		 * Update the taskbar button.
		 */
		void update(void) final;

	private slots:
		/**
		 * HACK: Timer for window initialization.
		 * If we attempt to get the dock item before the
		 * window is fully initialized, we won't find it.
		 */
		void setWindow_timer_slot(void);
};
