/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * TaskbarButtonManager.hpp: Taskbar button manager base class.            *
 *                                                                         *
 * Copyright (c) 2013-2025 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#pragma once

#include <QtWidgets/QWidget>

class TaskbarButtonManagerPrivate;
class TaskbarButtonManager : public QObject
{
	Q_OBJECT

	Q_PROPERTY(QWidget* window READ window WRITE setWindow)
	Q_PROPERTY(int progressBarValue READ progressBarValue WRITE setProgressBarValue)
	Q_PROPERTY(int progressBarMax READ progressBarMax WRITE setProgressBarMax)

protected:
	TaskbarButtonManager(TaskbarButtonManagerPrivate *d, QObject *parent = 0);
public:
	virtual ~TaskbarButtonManager();

protected:
	TaskbarButtonManagerPrivate *const d_ptr;
	Q_DECLARE_PRIVATE(TaskbarButtonManager)
private:
	typedef QObject super;
	Q_DISABLE_COPY(TaskbarButtonManager)

public:
	/**
	 * Get the window this TaskbarButtonManager is managing.
	 * @return Window
	 */
	QWidget *window(void) const;

	/**
	 * Set the window this TaskbarButtonManager should manage.
	 * This must be a top-level window in order to work properly.
	 *
	 * Subclasses should reimplement this function if special
	 * initialization is required to set up the taskbar button.
	 *
	 * TODO: Make a separate protected function that setWindow() calls?
	 *
	 * @param window Window
	 */
	virtual void setWindow(QWidget *window);

	/**
	 * Clear the progress bar.
	 */
	void clearProgressBar(void);

	/**
	 * Get the progress bar value.
	 * @return Value
	 */
	int progressBarValue(void) const;

	/**
	 * Set the progress bar value.
	 * @param current Value
	 */
	void setProgressBarValue(int value);

	/**
	 * Get the progress bar's maximum value.
	 * @return Maximum value
	 */
	int progressBarMax(void) const;

	/**
	 * Set the progress bar's maximum value.
	 * @param max Maximum value
	 */
	void setProgressBarMax(int max);

protected:
	/**
	 * Update the taskbar button.
	 */
	virtual void update(void) = 0;

private slots:
	/**
	 * Window we're managing was destroyed.
	 * @param obj QObject that was destroyed
	 */
	void windowDestroyed_slot(QObject *obj);
};
