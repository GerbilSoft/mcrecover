/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * DockManager.cpp: DockManager D-Bus implementation.                      *
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

#ifndef __MCRECOVER_DOCKMANAGER_HPP__
#define __MCRECOVER_DOCKMANAGER_HPP__

#include <QtCore/QObject>
class QWidget;

class DockManagerPrivate;

class DockManager : public QObject
{
	Q_OBJECT

	Q_PROPERTY(QWidget* window READ window WRITE setWindow)
	Q_PROPERTY(int progressBarValue READ progressBarValue WRITE setProgressBarValue)
	Q_PROPERTY(int progressBarMax READ progressBarMax WRITE setProgressBarMax)

	public:
		DockManager(QObject *parent = 0);
		~DockManager();

	private:
		friend class DockManagerPrivate;
		DockManagerPrivate *const d;
		Q_DISABLE_COPY(DockManager);

	public:
		/**
		 * Get the window this DockManager is managing.
		 * @return Window.
		 */
		QWidget *window(void);

		/**
		 * Set the window this DockManager should manage.
		 * This must be a top-level window in order to work properly.
		 * @param window Window.
		 */
		void setWindow(QWidget *window);

		/**
		 * Clear the progress bar.
		 */
		void clearProgressBar(void);

		/**
		 * Get the progress bar value.
		 * @return Value.
		 */
		int progressBarValue(void);

		/**
		 * Set the progress bar value.
		 * @param current Value.
		 */
		void setProgressBarValue(int value);

		/**
		 * Get the progress bar's maximum value.
		 * @return Maximum value.
		 */
		int progressBarMax(void);

		/**
		 * Set the progress bar's maximum value.
		 * @param max Maximum value.
		 */
		void setProgressBarMax(int max);

	private slots:
		/**
		 * Window we're managing was destroyed.
		 * @param obj QObject that was destroyed.
		 */
		void windowDestroyed_slot(QObject *obj);

		/**
		 * HACK: Timer for window initialization.
		 * If we attempt to get the dock item before the
		 * window is fully initialized, we won't find it.
		 */
		void setWindow_timer_slot(void);
};

#endif /* __MCRECOVER_DOCKMANAGER_HPP__ */
