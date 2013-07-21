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

#include "config.mcrecover.h"
#include "McRecoverQApplication.hpp"

// Qt includes.
#include <QtCore/QTranslator>
#include <QtCore/QLocale>
#include <QtCore/QLibraryInfo>
#include <QtCore/QDir>
#include <QtGui/QIcon>
#include <QtGui/QStyle>

// Custom types for QVariant.
#include "FileComments.hpp"


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
	QCoreApplication::setApplicationName(QLatin1String("GCN MemCard Recover"));

	// Version number.
	const QString sVersion = QString::fromLatin1(MCRECOVER_VERSION_STRING);
	QCoreApplication::setApplicationVersion(sVersion);

	// Set the application icon. (TODO)
	QIcon mcrIcon = q->IconFromProgram(QLatin1String("mcrecover"));
	q->setWindowIcon(mcrIcon);

#if QT_VERSION >= 0x040600
	// Check if an icon theme is available.
	if (!QIcon::hasThemeIcon(QLatin1String("application-exit"))) {
		// Icon theme is not available.
		// Use the built-in Oxygen icon theme.
		// Reference: http://tkrotoff.blogspot.com/2010/02/qiconfromtheme-under-windows.html
		QIcon::setThemeName(QLatin1String("oxygen"));
	}
#endif

#ifdef Q_OS_WIN
	// Set the application font.
	q->SetFont_Win32();
#endif /* Q_OS_WIN */

	// Register custom types for QVariant.
	qRegisterMetaType<FileComments>("FileComments");

	// Initialize Qt translators.
	qtTranslator = new QTranslator(q);
	q->installTranslator(qtTranslator);
	mcrTranslator = new QTranslator(q);
	q->installTranslator(mcrTranslator);

	// Initialize the mcrecover translation.
	setMcrTranslation(QLocale::system().name());
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
#if QT_VERSION >= 0x040600
	if (QIcon::hasThemeIcon(name))
		return QIcon::fromTheme(name);
#endif

	// System icon doesn't exist.
	// Get the fallback icon.
	struct IconSz_t {
		QString path;
		int sz;
	};

	static const IconSz_t iconSz[] = {
		{QLatin1String(":/oxygen/256x256/"), 256},
		{QLatin1String(":/oxygen/128x128/"), 128},
		{QLatin1String(":/oxygen/64x64/"), 64},
		{QLatin1String(":/oxygen/48x48/"), 48},
		{QLatin1String(":/oxygen/32x32/"), 32},
		{QLatin1String(":/oxygen/24x24/"), 24},
		{QLatin1String(":/oxygen/22x22/"), 22},
		{QLatin1String(":/oxygen/16x16/"), 16}
	};
	static const QString pngExt = QLatin1String(".png");

	QIcon icon;
	for (int i = 0; i < (int)(sizeof(iconSz)/sizeof(iconSz[0])); i++) {
		QPixmap pxm(iconSz[i].path + name + pngExt);
		if (!pxm.isNull())
			icon.addPixmap(pxm);
	}

	return icon;
}


/**
 * Get an icon from the MemCard Recover icon set.
 * @param name Icon name.
 * @return QIcon.
 */
QIcon McRecoverQApplication::IconFromProgram(QString name)
{
#if QT_VERSION >= 0x040600
	if (QIcon::hasThemeIcon(name))
		return QIcon::fromTheme(name);
#endif

	// System icon doesn't exist.
	// Get the fallback icon.
	struct IconSz_t {
		QString path;
		int sz;
	};

	static const IconSz_t iconSz[] = {
		{QLatin1String(":/mcrecover/256x256/"), 256},
		{QLatin1String(":/mcrecover/128x128/"), 128},
		{QLatin1String(":/mcrecover/64x64/"), 64},
		{QLatin1String(":/mcrecover/48x48/"), 48},
		{QLatin1String(":/mcrecover/32x32/"), 32},
		{QLatin1String(":/mcrecover/24x24/"), 24},
		{QLatin1String(":/mcrecover/22x22/"), 22},
		{QLatin1String(":/mcrecover/16x16/"), 16}
	};
	static const QString pngExt = QLatin1String(".png");

	QIcon icon;
	for (int i = 0; i < (int)(sizeof(iconSz)/sizeof(iconSz[0])); i++) {
		QPixmap pxm(iconSz[i].path + name + pngExt);
		if (!pxm.isNull())
			icon.addPixmap(pxm);
	}

	return icon;
}
