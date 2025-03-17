/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * AboutDialog.cpp: About Dialog.                                          *
 *                                                                         *
 * Copyright (c) 2013-2018 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

/** TODO: Synchronize with rom-properties. **/

#include "config.mcrecover.h"
#include "AboutDialog.hpp"
#include "util/git.h"

#include "db/GcnMcFileDb.hpp"

// C includes.
#include <string.h>

// Qt includes.
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QDir>
#include <QtCore/QCoreApplication>
#include <QScrollArea>

// Path functions.
#include "PathFuncs.hpp"

#ifdef HAVE_ZLIB
#include <zlib.h>
#endif /* HAVE_ZLIB */

#ifdef HAVE_PNG
#include <png.h>
#include "APNG_dlopen.h"
#endif /* HAVE_PNG */

// giflib
#include "GIF_dlopen.h"

/** AboutDialogPrivate **/

#include "ui_AboutDialog.h"
class AboutDialogPrivate
{
	public:
		explicit AboutDialogPrivate(AboutDialog *q);

	protected:
		AboutDialog *const q_ptr;
		Q_DECLARE_PUBLIC(AboutDialog)
	private:
		Q_DISABLE_COPY(AboutDialogPrivate)

	public:
		static AboutDialog *ms_AboutDialog;
		Ui::AboutDialog ui;

		bool scrlAreaInit;

		// Initialize the About Dialog text.
		void initAboutDialogText(void);

		// Credits.
		static QString GetCredits(void);

		// Libraries.
		static QString GetLibraries(void);

		// Debug information.
		static QString GetDebugInfo(void);
#ifdef Q_OS_WIN
		static QString GetCodePageInfo(void);
#endif /* Q_OS_WIN */

		// Support.
		static QString GetSupport(void);
};

// Static member initialization.
AboutDialog *AboutDialogPrivate::ms_AboutDialog = nullptr;


AboutDialogPrivate::AboutDialogPrivate(AboutDialog* q)
	: q_ptr(q)
	, scrlAreaInit(false)
{ }

/**
 * Initialize the About Dialog text.
 */
void AboutDialogPrivate::initAboutDialogText(void)
{
	// Line break string.
	const QLatin1String sLineBreak("<br/>\n");

	// Build the program title text.
	QString sPrgTitle;
	sPrgTitle.reserve(4096);
	sPrgTitle += QLatin1String("<b>") +
			QCoreApplication::applicationName() +
			QLatin1String("</b>") + sLineBreak +
			AboutDialog::tr("Version %1")
			.arg(QCoreApplication::applicationVersion());
#ifdef MCRECOVER_GIT_VERSION
	sPrgTitle += sLineBreak + QString::fromUtf8(MCRECOVER_GIT_VERSION);
#ifdef MCRECOVER_GIT_DESCRIBE
	sPrgTitle += sLineBreak + QString::fromUtf8(MCRECOVER_GIT_DESCRIBE);
#endif /* MCRECOVER_GIT_DESCRIBE */
#endif /* MCRECOVER_GIT_DESCRIBE */

#ifdef Q_OS_LINUX
	// Set the "MegaCard Engine" text.
	sPrgTitle += sLineBreak + sLineBreak +
		AboutDialog::tr("Powered by the<br/>\n<b>MegaCard Engine</b>\xE2\x84\xA2");
#endif

	// Set the program title text.
        ui.lblPrgTitle->setText(sPrgTitle);

	// Set the credits text.
	ui.lblCredits->setTextFormat(Qt::RichText);
	ui.lblCredits->setText(GetCredits());

	// Set the included libraries text.
	ui.lblLibraries->setTextFormat(Qt::PlainText);
	ui.lblLibraries->setText(GetLibraries());

	// Set the debug information text.
	ui.lblDebugInfo->setTextFormat(Qt::PlainText);
	ui.lblDebugInfo->setText(GetDebugInfo());

	// Set the support text.
	ui.lblSupport->setTextFormat(Qt::RichText);
	ui.lblSupport->setText(GetSupport());

	if (!scrlAreaInit) {
		// Create the scroll areas.
		// Qt Designer's QScrollArea implementation is horribly broken.
		// Also, this has to be done after the labels are set, because
		// QScrollArea is kinda dumb.
		const QString css = QLatin1String(
			"QScrollArea, QLabel { background-color: transparent; }");

		QScrollArea *scrlCredits = new QScrollArea();
		scrlCredits->setFrameShape(QFrame::NoFrame);
		scrlCredits->setFrameShadow(QFrame::Plain);
		scrlCredits->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		scrlCredits->setStyleSheet(css);
		ui.lblCredits->setStyleSheet(css);
		scrlCredits->setWidget(ui.lblCredits);
		scrlCredits->setWidgetResizable(true);
		ui.vboxCredits->addWidget(scrlCredits);

		QScrollArea *scrlLibraries = new QScrollArea();
		scrlLibraries->setFrameShape(QFrame::NoFrame);
		scrlLibraries->setFrameShadow(QFrame::Plain);
		scrlLibraries->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		scrlLibraries->setStyleSheet(css);
		ui.lblLibraries->setStyleSheet(css);
		scrlLibraries->setWidget(ui.lblLibraries);
		scrlLibraries->setWidgetResizable(true);
		ui.vboxLibraries->addWidget(scrlLibraries);

		QScrollArea *scrlDebugInfo = new QScrollArea();
		scrlDebugInfo->setFrameShape(QFrame::NoFrame);
		scrlDebugInfo->setFrameShadow(QFrame::Plain);
		scrlDebugInfo->setStyleSheet(css);
		ui.lblDebugInfo->setStyleSheet(css);
		// Don't turn off hscroll because the default db filename might be too long.
		//scrlDebugInfo->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		scrlDebugInfo->setWidget(ui.lblDebugInfo);
		scrlDebugInfo->setWidgetResizable(true);
		ui.vboxDebugInfo->addWidget(scrlDebugInfo);

		QScrollArea *scrlSupport = new QScrollArea();
		scrlSupport->setFrameShape(QFrame::NoFrame);
		scrlSupport->setFrameShadow(QFrame::Plain);
		scrlSupport->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		scrlSupport->setStyleSheet(css);
		ui.lblSupport->setStyleSheet(css);
		scrlSupport->setWidget(ui.lblSupport);
		scrlSupport->setWidgetResizable(true);
		ui.vboxSupport->addWidget(scrlSupport);

		// Scroll areas initialized.
		scrlAreaInit = true;
	}

	// Set initial focus to the tabWidget.
	ui.tabWidget->setFocus();
}

/**
 * Credits.
 */
QString AboutDialogPrivate::GetCredits(void)
{
	const QLatin1String sLineBreak("<br/>\n");
	const QLatin1String sIndent("&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;");
	static const QChar chrBullet(0x2022);  // U+2022: BULLET

	QString credits;
	credits.reserve(4096);
	credits += QLatin1String("Copyright (c) 2012-2016 by David Korth.");

	enum CreditType_t {
		CT_CONTINUE = 0,	// Continue previous type.
		CT_MCF_CONTRIBUTORS,	// Memory Card File Contributors
		CT_TRANSLATORS,		// Translators

		CT_MAX
	};

	struct CreditsData_t {
		CreditType_t type;
		const char *name;
		const char *url;
		const char *sub;
	};

	// Credits data.
	static const CreditsData_t CreditsData[] = {
		{CT_MCF_CONTRIBUTORS,	"MainMemory", nullptr, nullptr},	// NTSC-U, fragmented card
		{CT_CONTINUE,	"Carbuncle", nullptr, nullptr},			// NTSC-U (aka megamanblue)
		{CT_CONTINUE,	"Jeff Turner", nullptr, nullptr},		// NTSC-U
		{CT_CONTINUE,	"gold lightning", nullptr, nullptr},		// NTSC-U
		{CT_CONTINUE,	"Thomas Vasto", nullptr, nullptr},		// PAL
		{CT_CONTINUE,	"Henke37", nullptr, nullptr,},			// PAL
		{CT_CONTINUE,	"Gordon Griffin", nullptr, nullptr},		// NTSC-U
		{CT_CONTINUE,	"LocalH", nullptr, nullptr},			// NTSC-U
		{CT_CONTINUE,	"McLaglen", nullptr, nullptr},			// PAL (aka Mainman)
		{CT_CONTINUE,	"einstein95", nullptr, nullptr},		// Multiple regions
		{CT_CONTINUE,	"Hendricks266", nullptr, nullptr},		// NTSC-U
		{CT_CONTINUE,	"Typpex", nullptr, nullptr},			// NTSC-U, PAL
		{CT_CONTINUE,	"JaxTH", nullptr, nullptr},			// NTSC-U
		{CT_CONTINUE,	"SonicFreak94", nullptr, nullptr},		// NTSC-U
		{CT_CONTINUE,	"gukid", nullptr, nullptr},			// NTSC-U

		{CT_TRANSLATORS,	"Overlord", nullptr, "en_GB"},
		{CT_CONTINUE,	"Kevin L\xC3\xB3pez", "https://kelopez.cl/", "es_CL"},
		{CT_CONTINUE,	"Egor305", nullptr, "ru_RU"},

		{CT_MAX, nullptr, nullptr, nullptr}
	};
	
	CreditType_t lastCreditType = CT_CONTINUE;
	for (const CreditsData_t *creditsData = &CreditsData[0];
	     creditsData->type < CT_MAX; creditsData++)
	{
		if (creditsData->type != CT_CONTINUE &&
		    creditsData->type != lastCreditType)
		{
			// New credit type.
			credits += sLineBreak + sLineBreak;
			credits += QLatin1String("<b>");

			switch (creditsData->type) {
				case CT_MCF_CONTRIBUTORS:
					credits += AboutDialog::tr("Memory Card File Contributors:");
					break;
				case CT_TRANSLATORS:
					credits += AboutDialog::tr("UI Translators:");
				default:
					break;
			}

			credits += QLatin1String("</b>");
		}

		// Append the contributor's name.
		credits += sLineBreak + sIndent +
			chrBullet + QChar(L' ');
		if (creditsData->url) {
			credits += QLatin1String("<a href='") +
				QLatin1String(creditsData->url) +
				QLatin1String("'>");
		}
		credits += QString::fromUtf8(creditsData->name);
		if (creditsData->url) {
			credits += QLatin1String("</a>");
		}
		if (creditsData->sub) {
			credits += QLatin1String(" (") +
				QLatin1String(creditsData->sub) +
				QChar(L')');
		}
	}

	return credits;
}

/**
 * Get libraries used by GCN MemCard Recover.
 * @return Libraries used by GCN MemCard Recover.
 */
QString AboutDialogPrivate::GetLibraries(void)
{
	// Double linebreak.
	const QLatin1String sDLineBreak("\n\n");

	// NOTE: These strings can NOT be static.
	// Otherwise, they won't be retranslated if the UI language
	// is changed at runtime.

	//: Using an internal copy of a library.
	const QString sIntCopyOf = AboutDialog::tr("Internal copy of %1.");
	//: Compiled with a specific version of an external library.
	const QString sCompiledWith = AboutDialog::tr("Compiled with %1.");
	//: Using an external library, e.g. libpcre.so
	const QString sUsingDll = AboutDialog::tr("Using %1.");
	//: License: (libraries with only a single license)
	const QString sLicense = AboutDialog::tr("License: %1");
	//: Licenses: (libraries with multiple licenses)
	const QString sLicenses = AboutDialog::tr("Licenses: %1");

	// Included libraries string.
	QString sLibraries;
	sLibraries.reserve(4096);

	// Icon set.
	sLibraries = QLatin1String(
		"Icon set is based on KDE's Oxygen icons. (5.46.0)\n"
		"Copyright (C) 2005-2018 by David Vignoni.\n");
	sLibraries += sLicenses.arg(QLatin1String("CC BY-SA 3.0, GNU LGPL v2.1+"));

	// EU flag icon.
	sLibraries += sDLineBreak;
	sLibraries += QLatin1String(
		"EU flag icon is based on Dolphin Emulator 4.0's Flag_Europe.png icon.");
	sLibraries += sLicenses.arg(QLatin1String("GNU GPL v2+"));

	// TODO: Don't show compiled-with version if the same as in-use version?

	/** Qt **/
	sLibraries += sDLineBreak;
	QString qtVersion = QLatin1String("Qt ") + QLatin1String(qVersion());
#ifdef QT_IS_STATIC
	sLibraries += sIntCopyOf.arg(qtVersion);
#else
	QString qtVersionCompiled = QLatin1String("Qt " QT_VERSION_STR);
	sLibraries += sCompiledWith.arg(qtVersionCompiled) + QChar(L'\n');
	sLibraries += sUsingDll.arg(qtVersion);
#endif /* QT_IS_STATIC */
	sLibraries += QChar(L'\n') +
		QLatin1String("Copyright (C) 1995-2016 The Qt Company Ltd. and/or its subsidiaries.");
	sLibraries += QChar(L'\n') + sLicenses.arg(QLatin1String("GNU LGPL v2.1+, GNU GPL v2+"));

	/** zlib **/
#ifdef HAVE_ZLIB
	sLibraries += sDLineBreak;
	QString sZlibVersion = QLatin1String("zlib %1");
	sZlibVersion = sZlibVersion.arg(QLatin1String(zlibVersion()));

#ifdef USE_INTERNAL_ZLIB
	sLibraries += sIntCopyOf.arg(sZlibVersion) + QChar(L'\n');
#else /* !USE_INTERNAL_ZLIB */
	QString sZlibVersionCompiled = QLatin1String("zlib " ZLIB_VERSION);
	sLibraries += sCompiledWith.arg(sZlibVersionCompiled) + QChar(L'\n');
	sLibraries += sUsingDll.arg(sZlibVersion) + QChar(L'\n');
#endif /* USE_INTERNAL_PNG */
	// TODO: Use richtext instead of plaintext?
	sLibraries += QLatin1String(
			"Copyright (C) 1995-2013 Jean-loup Gailly and Mark Adler.\n"
			"https://www.zlib.net/\n");
	sLibraries += sLicense.arg(QLatin1String("zlib license"));
#endif /* HAVE_ZLIB */

	/** libpng **/
#ifdef HAVE_PNG
	// APNG suffix.
	const QString pngAPngSuffix = (APNG_is_supported()
			? QLatin1String(" + APNG")
			: AboutDialog::tr(" (No APNG support)"));

	sLibraries += sDLineBreak;
	const uint32_t png_version_number = png_access_version_number();
	QString pngVersion = QString::fromLatin1("libpng %1.%2.%3")
			.arg(png_version_number / 10000)
			.arg((png_version_number / 100) % 100)
			.arg(png_version_number % 100);
	pngVersion += pngAPngSuffix;

#ifdef USE_INTERNAL_PNG
	sLibraries += sIntCopyOf.arg(pngVersion);
#else /* !USE_INTERNAL_PNG */
	QString pngVersionCompiled = QLatin1String("libpng " PNG_LIBPNG_VER_STRING);
	pngVersionCompiled += pngAPngSuffix;
	sLibraries += sCompiledWith.arg(pngVersionCompiled) + QChar(L'\n');
	sLibraries += sUsingDll.arg(pngVersion);
#endif /* USE_INTERNAL_PNG */

	/**
	 * NOTE: MSVC does not define __STDC__ by default.
	 * If __STDC__ is not defined, the libpng copyright
	 * will not have a leading newline, and all newlines
	 * will be replaced with groups of 6 spaces.
	 */
	QString png_copyright = QLatin1String(png_get_copyright(nullptr));
	if (png_copyright.indexOf(QChar(L'\n')) < 0) {
		// Convert spaces to newlines.
		// TODO: QString::simplified() to remove other patterns,
		// or just assume all versions of libpng have the same
		// number of spaces?
		png_copyright.replace(QLatin1String("      "), QLatin1String("\n"));
		png_copyright.prepend(QChar(L'\n'));
		png_copyright.append(QChar(L'\n'));
	}
	sLibraries += png_copyright;
	sLibraries += sLicense.arg(QLatin1String("libpng license"));
#endif /* HAVE_PNG */

#ifdef USE_GIF
	/** giflib **/
	// TODO: Constant string instead of .arg()?
#ifdef USE_INTERNAL_GIF
	sLibraries += sDLineBreak;
	QString gifVersion = QString::fromLatin1("giflib %1.%2.%3")
			.arg(GIFLIB_MAJOR)
			.arg(GIFLIB_MINOR)
			.arg(GIFLIB_RELEASE);
	sLibraries += sIntCopyOf.arg(gifVersion) + QChar(L'\n');
	sLibraries += QLatin1String("Copyright (c) 1989-2016 giflib developers.") + QChar(L'\n');
	sLibraries += sLicense.arg(QLatin1String("MIT/X license"));
#else /* !USE_INTERNAL_GIF */
	int giflib_version = GifDlVersion();
	if (giflib_version > 0) {
		// TODO: Get the revision number somehow?
		sLibraries += sDLineBreak;
		QString gifVersion = QString::fromLatin1("giflib %1.%2")
				.arg(giflib_version / 10)
				.arg(giflib_version % 10);
		sLibraries += sUsingDll.arg(gifVersion) + QChar(L'\n');
		sLibraries += QLatin1String("Copyright (c) 1989-2016 giflib developers.") + QChar(L'\n');
		sLibraries += sLicense.arg(QLatin1String("MIT/X license"));
	}
#endif /* USE_INTERNAL_GIF */
#endif /* USE_GIF */

	// Return the included libraries string.
	return sLibraries;
}

/**
 * Get debug information.
 * @return Debug information.
 */
QString AboutDialogPrivate::GetDebugInfo(void)
{
	static const QChar chrBullet(0x2022);  // U+2022: BULLET

	// Debug information.
	QString sDebugInfo;
	sDebugInfo.reserve(4096);

#ifdef Q_OS_WIN
	// Win32 code page information.
	sDebugInfo += GetCodePageInfo() + QChar(L'\n') + QChar(L'\n');
#endif /* Q_OS_WIN */

	// Database filenames.
	QVector<QString> dbFilenames = GcnMcFileDb::GetDbFilenames();
	sDebugInfo += AboutDialog::tr("Available databases:") + QChar(L'\n');
	if (dbFilenames.isEmpty()) {
		sDebugInfo += chrBullet + QChar(L' ') + AboutDialog::tr("(none found)");
	} else {
		for (int i = 0; i < dbFilenames.size(); i++) {
			if (i != 0)
				sDebugInfo += QChar(L'\n');
			sDebugInfo += chrBullet + QChar(L' ');

			QString filename = dbFilenames.at(i);

			// Check if the filename is relative to the application path.
			filename = PathFuncs::makeRelativeToApplication(filename);
#ifndef Q_OS_WIN
			// Non-Windows systems: Check if relative to the user's home directory.
			filename = PathFuncs::makeRelativeToHome(filename);
#endif /* !Q_OS_WIN */

			sDebugInfo += QDir::toNativeSeparators(filename);
		}
	}

	return sDebugInfo;
}

#ifdef Q_OS_WIN
#include <windows.h>

/**
 * Get information about the system code pages.
 * @return System code page information.
 */
QString AboutDialogPrivate::GetCodePageInfo(void)
{
	QString sCodePageInfo;

	// Get the ANSI and OEM code pages.
	struct cpInfo {
		unsigned int cp;
		const char *cpStr;
	};

	static const cpInfo m_cpInfo[2] = {
		//: Win32: ANSI code page. (e.g. 1252 for US/English, 932 for Japanese)
		{CP_ACP,	QT_TRANSLATE_NOOP(AboutDialog, "System ANSI code page:")},
		//: Win32: OEM code page. (e.g. 437 for US/English)
		{CP_OEMCP,	QT_TRANSLATE_NOOP(AboutDialog, "System OEM code page:")}
	};

	for (int i = 0; i < 2; i++) {
		sCodePageInfo += AboutDialog::tr(m_cpInfo[i].cpStr) + QChar(L' ');

		// Get the code page information.
		CPINFOEX cpix;
		BOOL bRet = GetCPInfoEx(m_cpInfo[i].cp, 0, &cpix);
		if (!bRet) {
			//: GetCPInfoEx() call failed.
			sCodePageInfo += AboutDialog::tr("Unknown [GetCPInfoEx() failed]") + QChar(L'\n');
			continue;
		}

		sCodePageInfo += QString::number(cpix.CodePage);

		// if the code page name is blank, don't add extra parentheses.
		if (cpix.CodePageName[0] == 0x00) {
			sCodePageInfo += QChar(L'\n');
			continue;
		}

		// Add the code page name.
		sCodePageInfo += QLatin1String(" (");

		// Windows XP has the code page number in cpix.CodePageName,
		// followed by two spaces, and then the code page name in parentheses.
		// FIXME: Qt4 uses /Zc:wchar_t-, so we can't use fromWCharArray().
		wchar_t *parenStart = wcschr(cpix.CodePageName, '(');
		if (!parenStart) {
			// No parentheses. Use the code page name as-is.
			sCodePageInfo += QString(reinterpret_cast<const QChar*>(cpix.CodePageName));
		} else {
			// Found starting parenthesis. Check for ending parenthesis.
			wchar_t *parenEnd = wcsrchr(parenStart, ')');
			if (parenEnd) {
				// Found ending parenthesis. Null it out.
				*parenEnd = 0x00;
			}

			sCodePageInfo += QString(reinterpret_cast<const QChar*>(parenStart + 1));
		}

		sCodePageInfo += QLatin1String(")\n");
	}

	// Is GCN MemCard Recover using Unicode?
	// FIXME: This may be incorrect...
	if (GetModuleHandleW(nullptr) != nullptr) {
		//: Win32: Unicode strings are being used. (WinNT)
		sCodePageInfo += AboutDialog::tr("Using Unicode strings for Win32 API.");
	} else {
		//: Win32: ANSI strings are being used. (Win9x)
		sCodePageInfo += AboutDialog::tr("Using ANSI strings for Win32 API.");
	}

	return sCodePageInfo;
}
#endif /* Q_OS_WIN */

/**
 * Support.
 */
QString AboutDialogPrivate::GetSupport(void)
{
	const QLatin1String sLineBreak("<br/>\n");
	static const QChar chrBullet(0x2022);  // U+2022: BULLET

	QString strSupport;
	strSupport.reserve(4096);
	strSupport = AboutDialog::tr(
			"For technical support, you can visit the following websites:") +
			sLineBreak;

	struct supportSite_t {
		const char *name;
		const char *url;
	};

	// Support sites.
	static const supportSite_t supportSites[] = {
		{QT_TRANSLATE_NOOP(AboutDialog, "Sonic Retro"), "https://forums.sonicretro.org/index.php?showtopic=32621"},
		{QT_TRANSLATE_NOOP(AboutDialog, "GBAtemp"), "https://gbatemp.net/threads/gcn-memcard-recover.349406/"},
		{nullptr, nullptr}
	};

	for (const supportSite_t *supportSite = &supportSites[0];
	     supportSite->name != nullptr; supportSite++)
	{
		QString siteUrl = QLatin1String(supportSite->url);
		QString siteName = QLatin1String(supportSite->name);
		QString siteUrlHtml = QLatin1String("<a href=\"") +
					siteUrl +
					QLatin1String("\">") +
					siteName +
					QLatin1String("</a>");

		strSupport += chrBullet + QChar(L' ') + siteUrlHtml + sLineBreak;
	}

	// Email the author.
	strSupport += sLineBreak +
			AboutDialog::tr(
				"You can also email the developer directly:") +
			sLineBreak + chrBullet + QChar(L' ') +
			QLatin1String(
				"<a href=\"mailto:gerbilsoft@gerbilsoft.com\">"
				"gerbilsoft@gerbilsoft.com"
				"</a>");

	return strSupport;
}

/** AboutDialog **/

/**
 * Initialize the About Dialog.
 */
AboutDialog::AboutDialog(QWidget *parent)
	: super(parent,
		Qt::Dialog |
		Qt::CustomizeWindowHint |
		Qt::WindowTitleHint |
		Qt::WindowSystemMenuHint |
		Qt::WindowCloseButtonHint)
	, d_ptr(new AboutDialogPrivate(this))
{
	Q_D(AboutDialog);
	d->ui.setupUi(this);

	// Make sure the window is deleted on close.
	this->setAttribute(Qt::WA_DeleteOnClose, true);

#ifdef Q_OS_MAC
	// Remove the window icon. (Mac "proxy icon")
	this->setWindowIcon(QIcon());

	// Hide the frames.
	d->ui.fraCopyrights->setFrameShape(QFrame::NoFrame);
	d->ui.fraCopyrights->layout()->setContentsMargins(0, 0, 0, 0);
	d->ui.fraLibraries->setFrameShape(QFrame::NoFrame);
	d->ui.fraLibraries->layout()->setContentsMargins(0, 0, 0, 0);
	d->ui.fraDebugInfo->setFrameShape(QFrame::NoFrame);
	d->ui.fraDebugInfo->layout()->setContentsMargins(0, 0, 0, 0);
	d->ui.fraCredits->setFrameShape(QFrame::NoFrame);
	d->ui.fraCredits->layout()->setContentsMargins(0, 0, 0, 0);
#endif

	// Initialize the About Dialog text.
	d->initAboutDialogText();
}

/**
 * Shut down the About Dialog.
 */
AboutDialog::~AboutDialog()
{
	AboutDialogPrivate::ms_AboutDialog = nullptr;
	delete d_ptr;
}

/**
 * Show a single instance of the About Dialog.
 * @param parent Parent window.
 */
void AboutDialog::ShowSingle(QWidget *parent)
{
	if (AboutDialogPrivate::ms_AboutDialog != nullptr) {
		// About Dialog is already displayed.
		// Activate the dialog.
		AboutDialogPrivate::ms_AboutDialog->activateWindow();
	} else {
		// About Dialog is not displayed.
		// Display it.
		AboutDialogPrivate::ms_AboutDialog = new AboutDialog(parent);
		AboutDialogPrivate::ms_AboutDialog->show();
	}
}

/**
 * Widget state has changed.
 * @param event State change event.
 */
void AboutDialog::changeEvent(QEvent *event)
{
	if (event->type() == QEvent::LanguageChange) {
		// Retranslate the UI.
		Q_D(AboutDialog);
		d->ui.retranslateUi(this);

		// Reinitialize the About Dialog text.
		d->initAboutDialogText();
	}

	// Pass the event to the base class.
	super::changeEvent(event);
}
