/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * McRecoverQApplication.cpp: QApplication subclass.                       *
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

#include "McRecoverQApplication.hpp"

// Qt includes.
#include <QtCore/QTranslator>
#include <QtCore/QLocale>
#include <QtCore/QLibraryInfo>
#include <QtCore/QDir>
#include <QtGui/QIcon>
#include <QtGui/QStyle>


class McRecoverQApplicationPrivate
{
	public:
		McRecoverQApplicationPrivate(McRecoverQApplication *q);

	private:
		McRecoverQApplication *const q;
		Q_DISABLE_COPY(McRecoverQApplicationPrivate)

	public:
		void init(void);

		// Initialize McRecoverQApplication.
		void mcrqaInit(void);

		/**
		 * Set the McRecover translation.
		 * @param locale Locale name, e.g. "en_US".
		 */
		void setMcrTranslation(QString locale);

	private:
		// Qt translators.
		QTranslator *qtTranslator;
		QTranslator *mcrTranslator;
};


/**************************************
 * McRecoverQApplicationPrivate functions. *
 **************************************/

McRecoverQApplicationPrivate::McRecoverQApplicationPrivate(McRecoverQApplication *q)
	: q(q)
	, qtTranslator(NULL)
	, mcrTranslator(NULL)
{ }

/**
 * McRecoverQApplication initialization function.
 * The same code is used in all three McRecoverQApplication() constructors.
 */
void McRecoverQApplicationPrivate::mcrqaInit(void)
{
	// Set application information.
	QCoreApplication::setOrganizationName(QLatin1String("GerbilSoft"));
	QCoreApplication::setApplicationName(QLatin1String("GameCube Memory Card Recovery Program"));

	// Version number. (TODO: Get from CMake.)
#if 0
	const QString sVersion = QString::fromLatin1("%1.%2.%3")
					.arg((LibGens::version >> 24) & 0xFF)
					.arg((LibGens::version >> 16) & 0xFF)
					.arg(LibGens::version & 0xFFFF);
	QCoreApplication::setApplicationVersion(sVersion);
#endif

	// Set the application icon. (TODO)
#if 0
	QIcon iconApp;
	iconApp.addFile(QLatin1String(":/gens/gensgs_48x48.png"), QSize(48, 48));
	iconApp.addFile(QLatin1String(":/gens/gensgs_32x32.png"), QSize(32, 32));
	iconApp.addFile(QLatin1String(":/gens/gensgs_16x16.png"), QSize(16, 16));
	q->setWindowIcon(iconApp);
#endif

#if QT_VERSION >= 0x040600
	// Check if an icon theme is available.
	if (!QIcon::hasThemeIcon(QLatin1String("application-exit"))) {
		// Icon theme is not available.
		// Use built-in Oxygen icon theme.
		// Reference: http://tkrotoff.blogspot.com/2010/02/qiconfromtheme-under-windows.html
		QIcon::setThemeName(QLatin1String("oxygen"));
	}
#endif

#ifdef Q_OS_WIN32
	// Set the application font.
	q->SetFont_Win32();
#endif /* Q_OS_WIN32 */

	// Initialize Qt translators.
	qtTranslator = new QTranslator(q);
	q->installTranslator(qtTranslator);
	mcrTranslator = new QTranslator(q);
	q->installTranslator(mcrTranslator);

	// Initialize the mcrecover translation.
	setMcrTranslation(QLocale::system().name());

	// Connect the crash handler.
#ifdef HAVE_SIGACTION
	QObject::connect(q, SIGNAL(signalCrash(int,siginfo_t*,void*)),
			 q, SLOT(slotCrash(int,siginfo_t*,void*)));
#else /* !HAVE_SIGACTION */
	QObject::connect(q, SIGNAL(signalCrash(int)),
			 q, SLOT(slotCrash(int)));
#endif /* HAVE_SIGACTION */
}


/**
 * Set the McRecover translation.
 * @param locale Locale name, e.g. "en_US".
 */
void McRecoverQApplicationPrivate::setMcrTranslation(QString locale)
{
	// Initialize the Qt translation system.
	// TODO: Allow switching languages on the fly?
	// TODO: Check in the following directories:
	// * Qt library directory
	// * Application/translations/
	// * Application/
	// * config/
	qtTranslator->load(
		QLatin1String("qt_") + locale,
		QLibraryInfo::location(QLibraryInfo::TranslationsPath));

	// Initialize the McRecover translator.
	// TODO: Check in the following directories:
	// * Application/translations/
	// * Application/
	// * config/
	QDir appDir(QApplication::applicationDirPath());
	mcrTranslator->load(
		QLatin1String("mcrecover_") + locale,
		appDir.absoluteFilePath(QLatin1String("translations/")));

	/** Translation file information. **/

	//: Translation file author. Put your name here.
	QString tsAuthor = McRecoverQApplication::tr("David Korth", "ts-author");
	Q_UNUSED(tsAuthor)

	// TODO: Allow the program to access the translation file information.
}


/*******************************
 * McRecoverQApplication functions. *
 *******************************/

McRecoverQApplication::McRecoverQApplication(int &argc, char **argv)
	: QApplication(argc, argv)
	, d(new McRecoverQApplicationPrivate(this))
{
	d->mcrqaInit();
}

McRecoverQApplication::McRecoverQApplication(int &argc, char **argv, bool GUIenabled)
	: QApplication(argc, argv, GUIenabled)
	, d(new McRecoverQApplicationPrivate(this))
{
	d->mcrqaInit();
}

McRecoverQApplication::McRecoverQApplication(int &argc, char **argv, Type type)
	: QApplication(argc, argv, type)
	, d(new McRecoverQApplicationPrivate(this))
{
	d->mcrqaInit();
}

McRecoverQApplication::~McRecoverQApplication()
	{ delete d; }


/**
 * Get an icon from the system theme.
 * @param name Icon name.
 * @return QIcon.
 */
QIcon McRecoverQApplication::IconFromTheme(QString name)
{
#ifndef Q_WS_X11
	// Check if a system icon exists.
	// TODO: Add standardPixmap parameter to reduce string comparisons?
	// TODO: Native Win32 icons for everything else.
	QStyle *style = McRecoverQApplication::style();
	if (name == QLatin1String("document-open"))
		return style->standardPixmap(QStyle::SP_DirOpenIcon);
#endif

#if QT_VERSION >= 0x040600
	if (QIcon::hasThemeIcon(name))
		return QIcon::fromTheme(name);
#endif

	// System icon doesn't exist.
	// Get the fallback icon.
	QIcon icon;
	icon.addFile(QLatin1String(":/oxygen/48x48/") + name + QLatin1String(".png"), QSize(48, 48));
	icon.addFile(QLatin1String(":/oxygen/32x32/") + name + QLatin1String(".png"), QSize(32, 32));
	icon.addFile(QLatin1String(":/oxygen/22x22/") + name + QLatin1String(".png"), QSize(22, 22));
	icon.addFile(QLatin1String(":/oxygen/16x16/") + name + QLatin1String(".png"), QSize(16, 16));
	return icon;
}
