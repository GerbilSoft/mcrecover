/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * TaskbarButtonManagerFactory.hpp: TaskbarButtonManager factory class.    *
 *                                                                         *
 * Copyright (c) 2015-2025 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#pragma once

// for Q_DISABLE_COPY()
#include <QtCore/qglobal.h>
class QObject;

class TaskbarButtonManager;
class TaskbarButtonManagerFactory
{
	private:
		TaskbarButtonManagerFactory();
		~TaskbarButtonManagerFactory();
	private:
		Q_DISABLE_COPY(TaskbarButtonManagerFactory)

	public:
		/**
		 * Create a TaskbarButtonManager.
		 * @param parent Parent object.
		 * @return System-specific TaskbarButtonManager, or nullptr on error.
		 */
		static TaskbarButtonManager *createManager(QObject *parent = 0);
};
