/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * TaskbarButtonManager_p.hpp: Taskbar button manager base class. (PRIVATE)*
 *                                                                         *
 * Copyright (c) 2013-2025 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#pragma once

#include "TaskbarButtonManager.hpp"

class TaskbarButtonManagerPrivate
{
public:
	explicit TaskbarButtonManagerPrivate(TaskbarButtonManager *const q);
	virtual ~TaskbarButtonManagerPrivate();

protected:
	TaskbarButtonManager *const q_ptr;
	Q_DECLARE_PUBLIC(TaskbarButtonManager)
private:
	Q_DISABLE_COPY(TaskbarButtonManagerPrivate)

public:
	// Window
	QWidget *window;

	// Status elements
	int progressBarValue;	// Current progress. (-1 for no bar)
	int progressBarMax;	// Maximum progress.
};
