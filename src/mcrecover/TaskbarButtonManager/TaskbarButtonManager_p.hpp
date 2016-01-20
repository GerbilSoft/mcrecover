/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * TaskbarButtonManager_p.hpp: Taskbar button manager base class. (PRIVATE)*
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

#ifndef __MCRECOVER_TASKBARBUTTONMANAGER_TASKBARBUTTONMANAGER_P_HPP__
#define __MCRECOVER_TASKBARBUTTONMANAGER_TASKBARBUTTONMANAGER_P_HPP__

#include "TaskbarButtonManager.hpp"

class TaskbarButtonManagerPrivate
{
	public:
		TaskbarButtonManagerPrivate(TaskbarButtonManager *const q);
		virtual ~TaskbarButtonManagerPrivate();

	protected:
		TaskbarButtonManager *const q_ptr;
		Q_DECLARE_PUBLIC(TaskbarButtonManager)
	private:
		Q_DISABLE_COPY(TaskbarButtonManagerPrivate)

	public:
		// Window.
		QWidget *window;

		// Status elements.
		int progressBarValue;	// Current progress. (-1 for no bar)
		int progressBarMax;	// Maximum progress.
};

#endif /* __MCRECOVER_TASKBARBUTTONMANAGER_TASKBARBUTTONMANAGER_P_HPP__ */
