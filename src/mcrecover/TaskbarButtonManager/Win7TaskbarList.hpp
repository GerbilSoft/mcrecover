/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * Win7TaskbarList.hpp: Windows 7 ITaskBarList3 implementation.            *
 *                                                                         *
 * Copyright (c) 2013-2025 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#pragma once

#include "TaskbarButtonManager.hpp"

class Win7TaskbarListPrivate;
class Win7TaskbarList : public TaskbarButtonManager
{
	Q_OBJECT

	public:
		explicit Win7TaskbarList(QObject *parent = 0);

	private:
		typedef TaskbarButtonManager super;
		Q_DECLARE_PRIVATE(Win7TaskbarList)
		Q_DISABLE_COPY(Win7TaskbarList)

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
};
