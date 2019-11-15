/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * TaskbarButtonManager.cpp: Taskbar button manager base class.            *
 *                                                                         *
 * Copyright (c) 2013-2016 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#include "TaskbarButtonManager.hpp"

// Qt includes.
#include <QWidget>

/** TaskbarButtonManagerPrivate **/
#include "TaskbarButtonManager_p.hpp"

TaskbarButtonManagerPrivate::TaskbarButtonManagerPrivate(TaskbarButtonManager *const q)
	: q_ptr(q)
	, window(nullptr)
	, progressBarValue(-1)
	, progressBarMax(-1)
{ }

TaskbarButtonManagerPrivate::~TaskbarButtonManagerPrivate()
{
	// NOTE: This empty function must stay here
	// in order to prevent vtable screwups.
}

/** TaskbarButtonManager **/

TaskbarButtonManager::TaskbarButtonManager(TaskbarButtonManagerPrivate *d, QObject* parent)
	: super(parent)
	, d_ptr(d)
{ }

TaskbarButtonManager::~TaskbarButtonManager()
{
	delete d_ptr;
}

/**
 * Get the window this TaskbarButtonManager is managing.
 * @return Window.
 */
QWidget *TaskbarButtonManager::window(void) const
{
	Q_D(const TaskbarButtonManager);
	return d->window;
}

/**
 * Set the window this TaskbarButtonManager should manage.
 * This must be a top-level window in order to work properly.
 * @param window Window.
 */
void TaskbarButtonManager::setWindow(QWidget *window)
{
	Q_D(TaskbarButtonManager);

	if (d->window) {
		// Disconnect slots from the existing window.
		disconnect(d->window, &QObject::destroyed,
			   this, &TaskbarButtonManager::windowDestroyed_slot);
	}

	d->window = window;

	if (d->window) {
		// Connect slots to the new window.
		connect(d->window, &QObject::destroyed,
			this, &TaskbarButtonManager::windowDestroyed_slot);
	}
}

/**
 * Clear the progress bar.
 */
void TaskbarButtonManager::clearProgressBar(void)
{
	Q_D(TaskbarButtonManager);
	d->progressBarValue = -1;
	d->progressBarMax = -1;
	this->update();
}

/**
 * Get the progress bar value.
 * @return Value.
 */
int TaskbarButtonManager::progressBarValue(void) const
{
	Q_D(const TaskbarButtonManager);
	return d->progressBarValue;
}

/**
 * Set the progress bar value.
 * @param value Value.
 */
void TaskbarButtonManager::setProgressBarValue(int value)
{
	Q_D(TaskbarButtonManager);
	if (d->progressBarValue != value) {
		d->progressBarValue = value;
		this->update();
	}
}

/**
 * Get the progress bar's maximum value.
 * @return Maximum value.
 */
int TaskbarButtonManager::progressBarMax(void) const
{
	Q_D(const TaskbarButtonManager);
	return d->progressBarMax;
}

/**
 * Set the progress bar's maximum value.
 * @param max Maximum value.
 */
void TaskbarButtonManager::setProgressBarMax(int max)
{
	Q_D(TaskbarButtonManager);
	if (d->progressBarMax != max) {
		d->progressBarMax = max;
		this->update();
	}
}

/** Slots **/

/**
 * Window we're managing was destroyed.
 * @param obj QObject that was destroyed.
 */
void TaskbarButtonManager::windowDestroyed_slot(QObject *obj)
{
	Q_D(TaskbarButtonManager);
	if (!d->window || !obj)
		return;

	if (obj == d->window) {
		// This was our window.
		// TODO: Make a base close() function?
		this->setWindow(nullptr);
	}
}
