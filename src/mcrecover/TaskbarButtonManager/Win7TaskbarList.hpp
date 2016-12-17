/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * Win7TaskbarList.hpp: Windows 7 ITaskBarList3 implementation.            *
 *                                                                         *
 * Copyright (c) 2013-2016 by David Korth.                                 *
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

#ifndef __MCRECOVER_TASKBARBUTTONMANAGER_WIN7TASKBARLIST_HPP__
#define __MCRECOVER_TASKBARBUTTONMANAGER_WIN7TASKBARLIST_HPP__

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
		virtual void setWindow(QWidget *window) final;

	protected:
		/**
		 * Update the taskbar button.
		 */
		virtual void update(void) final;
};

#endif /* __MCRECOVER_TASKBARBUTTONMANAGER_WIN7TASKBARLIST_HPP__ */
