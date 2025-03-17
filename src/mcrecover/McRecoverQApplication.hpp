/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * McRecoverQApplication.hpp: QApplication subclass.                       *
 *                                                                         *
 * Copyright (c) 2013-2025 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#ifndef __MCRECOVER_MCRECOVERQAPPLICATION_HPP__
#define __MCRECOVER_MCRECOVERQAPPLICATION_HPP__

#include <QApplication>
#include <QtGui/QIcon>
#include <QtGui/QPixmap>
#include <QStyle>

class McRecoverQApplicationPrivate;
class McRecoverQApplication : public QApplication
{
	Q_OBJECT
	typedef QApplication super;

	public:
		McRecoverQApplication(int &argc, char **argv);
		virtual ~McRecoverQApplication() { }

	private:
		Q_DISABLE_COPY(McRecoverQApplication)

	public:
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

		/**
		 * Get a standard icon.
		 * @param standardIcon Standard pixmap.
		 * @param option QStyleOption.
		 * @param widget QWidget.
		 * @return QIcon.
		 */
		static QIcon StandardIcon(QStyle::StandardPixmap standardIcon,
				const QStyleOption *option = 0,
				const QWidget *widget = 0);

#ifdef Q_OS_WIN
		enum class Win32Icon {
			None = 0,
			Defrag,

			Max
		};

		/**
		 * Get an icon from a Win32 module.
		 * @param module Filename of the Win32 module.
		 * @param resId Resource identifier.
		 * @param size Icon size.
		 * @return Icon, as a QPixmap.
		 */
		static QPixmap getIconFromModule(const QString &module,
					uint16_t resId, const QSize &size);

		/**
		 * Get a Win32 icon.
		 * @param icon Win32 icon.
		 * @param size Desired size.
		 * @return QIcon.
		 */
		static QIcon Win32Icon(Win32Icon icon, const QSize &size);

		// Win32 event filter.
		bool winEventFilter(MSG *msg, long *result);

		/**
		 * Set the Qt font to match the system font.
		 */
		static void SetFont_Win32(void);

		/**
		 * Get the message ID for TaskbarButtonCreated.
		 * @return Message ID, or 0 if not registered.
		 */
		static unsigned int WM_TaskbarButtonCreated(void);
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
