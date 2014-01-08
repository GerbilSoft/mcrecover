/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * McRecoverQApplication.hpp: QApplication subclass.                       *
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

#ifndef __MCRECOVER_MCRECOVERQAPPLICATION_HPP__
#define __MCRECOVER_MCRECOVERQAPPLICATION_HPP__

#include <QtGui/QApplication>
#include <QtGui/QIcon>

class McRecoverQApplicationPrivate;

class McRecoverQApplication : public QApplication
{
	Q_OBJECT

	public:
		McRecoverQApplication(int &argc, char **argv);
		McRecoverQApplication(int &argc, char **argv, bool GUIenabled);
		McRecoverQApplication(int &argc, char **argv, Type type);
		virtual ~McRecoverQApplication();

		/**
		 * Get an icon from the system theme.
		 * @param name Icon name.
		 * @return QIcon.
		 */
		static QIcon IconFromTheme(const QString &name);

		/**
		 * Get an icon from the MemCard Recover icon set.
		 * @param name Icon name.
		 * @return QIcon.
		 */
		static QIcon IconFromProgram(const QString &name);

#ifdef Q_OS_WIN
		// Win32 event filter.
		bool winEventFilter(MSG *msg, long *result);
#endif /* Q_OS_WIN */

	private:
		friend class McRecoverQApplicationPrivate;
		McRecoverQApplicationPrivate *const d;

		Q_DISABLE_COPY(McRecoverQApplication)

#ifdef Q_OS_WIN
		/**
		 * Set the Qt font to match the system font.
		 */
		static void SetFont_Win32(void);
#endif /* Q_OS_WIN */

	signals:
		/**
		 * The system theme has changed.
		 * Currently only triggered on Windows via WM_THEMECHANGED.
		 * TODO: Add Linux/X11 support.
		 */
		void themeChanged(void);
};

#endif /* __MCRECOVER_MCRECOVERQAPPLICATION_HPP__ */
