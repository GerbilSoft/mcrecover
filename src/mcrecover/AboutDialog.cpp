/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * AboutDialog.cpp: About Dialog.                                          *
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

#include "AboutDialog.hpp"
#include "git.h"

// C includes.
#include <string.h>

// Qt includes.
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QDir>
#include <QtCore/QCoreApplication>
#include <QtGui/QScrollArea>


class AboutDialogPrivate
{
	public:
		AboutDialogPrivate(AboutDialog *q);

	private:
		AboutDialog *const q;
		Q_DISABLE_COPY(AboutDialogPrivate);

	public:
		static AboutDialog *ms_AboutDialog;

		bool scrlAreaInit;

		// Initialize the About Dialog text.
		void initAboutDialogText(void);

		// Credits.
		static QString GetCredits(void);

		// Included libraries.
		static QString GetIncLibraries(void);

		// Debug information.
		static QString GetDebugInfo(void);
#ifdef Q_OS_WIN32
		static QString GetCodePageInfo(void);
#endif /* Q_OS_WIN32 */
};

// Static member initialization.
AboutDialog *AboutDialogPrivate::ms_AboutDialog = NULL;


AboutDialogPrivate::AboutDialogPrivate(AboutDialog* q)
	: q(q)
{ }


/**
 * Initialize the About Dialog text.
 */
void AboutDialogPrivate::initAboutDialogText(void)
{
	// Line break string.
	const QString sLineBreak = QLatin1String("<br/>\n");

	// Build the credits text.
	QString sCredits = GetCredits();

	// Set the credits text.
	q->lblCredits->setText(sCredits);
	q->lblCredits->setTextFormat(Qt::RichText);

	// Build the program title text.
	QString sPrgTitle;
	sPrgTitle.reserve(4096);
	sPrgTitle += QLatin1String("<b>") +
			QCoreApplication::applicationName() +
			QLatin1String("</b>") + sLineBreak +
			AboutDialog::tr("Version %1")
			.arg(QCoreApplication::applicationVersion()) + sLineBreak;
#ifdef MCRECOVER_GIT_VERSION
	sPrgTitle += QLatin1String(MCRECOVER_GIT_VERSION) + sLineBreak;
#endif

#ifdef Q_OS_LINUX
	// Set the "MegaCard Engine" text.
	sPrgTitle += sLineBreak +
		QApplication::translate("AboutDialog",
			"Powered by the<br/>\n<b>MegaCard Engine</b>™",
			0, QApplication::UnicodeUTF8);
#endif

	// Set the program title text.
        q->lblPrgTitle->setText(sPrgTitle);

	// Set the included libraries text.
	q->lblIncLibraries->setText(GetIncLibraries());
	q->lblIncLibraries->setTextFormat(Qt::RichText);

	// Set the debug information text.
	q->lblDebugInfo->setText(GetDebugInfo());
	q->lblDebugInfo->setTextFormat(Qt::RichText);

	if (scrlAreaInit) {
		// Create the scroll areas.
		// Qt Designer's QScrollArea implementation is horribly broken.
		// Also, this has to be done after the labels are set, because
		// QScrollArea is kinda dumb.
		QScrollArea *scrlCredits = new QScrollArea();
		scrlCredits->setFrameShape(QFrame::NoFrame);
		scrlCredits->setFrameShadow(QFrame::Plain);
		scrlCredits->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		scrlCredits->setWidget(q->lblCredits);
		scrlCredits->setWidgetResizable(true);
		q->vboxCredits->addWidget(scrlCredits);
		scrlCredits->setAutoFillBackground(false);

		QScrollArea *scrlIncLibraries = new QScrollArea();
		scrlIncLibraries->setFrameShape(QFrame::NoFrame);
		scrlIncLibraries->setFrameShadow(QFrame::Plain);
		scrlIncLibraries->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		scrlIncLibraries->setWidget(q->lblIncLibraries);
		scrlIncLibraries->setWidgetResizable(true);
		q->vboxIncLibraries->addWidget(scrlIncLibraries);
		scrlIncLibraries->setAutoFillBackground(false);

		QScrollArea *scrlDebugInfo = new QScrollArea();
		scrlDebugInfo->setFrameShape(QFrame::NoFrame);
		scrlDebugInfo->setFrameShadow(QFrame::Plain);
		scrlDebugInfo->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		scrlDebugInfo->setWidget(q->lblDebugInfo);
		scrlDebugInfo->setWidgetResizable(true);
		q->vboxDebugInfo->addWidget(scrlDebugInfo);
		scrlDebugInfo->setAutoFillBackground(false);

		// Scroll areas initialized.
		scrlAreaInit = true;
	}
}


/**
 * Credits.
 */
QString AboutDialogPrivate::GetCredits(void)
{
	static const QString sLineBreak = QLatin1String("<br/>\n");
	static const QString sIndent = QLatin1String("&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;");
	static const QChar chrBullet(0x2022);  // U+2022: BULLET

	QString credits;
	credits.reserve(4096);
	credits += QLatin1String("Copyright (c) 2012-2013 by David Korth.");

	enum CreditType_t {
		CT_CONTINUE = 0,	// Continue previous type.
		CT_MCF_CONTRIBUTORS,	// Memory Card File Contributors

		CT_MAX
	};

	struct CreditsData_t {
		CreditType_t type;
		const char *name;
	};

	// Credits data.
	static const CreditsData_t CreditsData[] = {
		{CT_MCF_CONTRIBUTORS,	"gold lightning"},
		{CT_CONTINUE,		"Henke37"},

		{CT_MAX, NULL}
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
				default:
					break;
			}

			credits += QLatin1String("</b>") + sLineBreak;
		}

		// Append the contributor's name.
		credits += sIndent + chrBullet + QChar(L' ') +
			QLatin1String(creditsData->name) + sLineBreak;
	}

	return credits;
}


/**
 * Get included libraries.
 * @return Included libraries.
 */
QString AboutDialogPrivate::GetIncLibraries(void)
{
	// Common strings.
	static const QString sLineBreak = QLatin1String("<br/>\n");
	const QString sIntCopyOf = AboutDialog::tr("Internal copy of %1.");

	// Included libraries string.
	QString sIncLibraries;

	// TODO: Statically-linked Qt.
	// TODO: Statically-linked PCRE.

	// Return the included libraries string.
	return sIncLibraries;
}


/**
 * Get debug information.
 * @return Debug information.
 */
QString AboutDialogPrivate::GetDebugInfo(void)
{
	// Line break string.
	const QString sLineBreak = QLatin1String("<br/>\n");

	// Debug information.
	QString sDebugInfo =
		AboutDialog::tr("Compiled using Qt %1.").arg(QLatin1String(QT_VERSION_STR)) + sLineBreak +
		AboutDialog::tr("Using Qt %1.").arg(QLatin1String(qVersion())) + sLineBreak + sLineBreak;

	// Reserve at least 4 KB for the debug information.
	sDebugInfo.reserve(4096);

#ifdef Q_OS_WIN32
	// Win32 code page information.
	sDebugInfo += GetCodePageInfo() + sLineBreak;
#endif /* Q_OS_WIN32 */

	// Trim whitespace at the end of sDebugInfo.
	return sDebugInfo.trimmed();
}


#ifdef Q_OS_WIN32
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
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

	cpInfo m_cpInfo[2] = {
		//: Win32: ANSI code page. (e.g. 1252 for US/English, 932 for Japanese)
		{CP_ACP,	QT_TR_NOOP("System ANSI code page:")},
		//: Win32: OEM code page. (e.g. 437 for US/English)
		{CP_OEMCP,	QT_TR_NOOP("System OEM code page:")}
	};

	// TODO: GetCPInfoExU() support?
	for (int i = 0; i < 2; i++) {
		sCodePageInfo += AboutDialog::tr(m_cpInfo[i].cpStr);

		// Get the code page information.
		CPINFOEX cpix;
		BOOL bRet = GetCPInfoExA(m_cpInfo[i].cp, 0, &cpix);
		if (!bRet) {
			//: GetCPInfoExA() call failed.
			sCodePageInfo += AboutDialog::tr("Unknown [GetCPInfoExA() failed]") + QChar(L'\n');
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
		char *parenStart = strchr(cpix.CodePageName, '(');
		if (!parenStart) {
			// No parentheses. Use the code page name as-is.
			sCodePageInfo += QString::fromLocal8Bit(cpix.CodePageName);
		} else {
			// Found starting parenthesis. Check for ending parenthesis.
			char *parenEnd = strrchr(parenStart, ')');
			if (parenEnd) {
				// Found ending parenthesis. Null it out.
				*parenEnd = 0x00;
			}

			sCodePageInfo += QString::fromLocal8Bit(parenStart + 1);
		}

		sCodePageInfo += QLatin1String(")\n");
	}

	// Is Gens/GS II using Unicode?
	if (GetModuleHandleW(NULL) != NULL) {
		//: Win32: Unicode strings are being used. (WinNT)
		sCodePageInfo += AboutDialog::tr("Using Unicode strings for Win32 API.");
	} else {
		//: Win32: ANSI strings are being used. (Win9x)
		sCodePageInfo += AboutDialog::tr("Using ANSI strings for Win32 API.");
	}
	sCodePageInfo += QChar(L'\n');

	return sCodePageInfo;
}
#endif /* Q_OS_WIN32 */


/** AboutDialog **/

/**
 * Initialize the About Dialog.
 */
AboutDialog::AboutDialog(QWidget *parent)
	: QDialog(parent,
		Qt::Dialog |
		Qt::WindowTitleHint |
		Qt::WindowSystemMenuHint |
		Qt::WindowMinimizeButtonHint |
		Qt::WindowCloseButtonHint)
	, d(new AboutDialogPrivate(this))
{
	setupUi(this);

	// Make sure the window is deleted on close.
	this->setAttribute(Qt::WA_DeleteOnClose, true);

#ifdef Q_WS_MAC
	// Remove the window icon. (Mac "proxy icon")
	this->setWindowIcon(QIcon());

	// Hide the frames.
	fraCopyrights->setFrameShape(QFrame::NoFrame);
	fraCopyrights->layout()->setContentsMargins(0, 0, 0, 0);
	fraIncLibraries->setFrameShape(QFrame::NoFrame);
	fraIncLibraries->layout()->setContentsMargins(0, 0, 0, 0);
	fraDebugInfo->setFrameShape(QFrame::NoFrame);
	fraDebugInfo->layout()->setContentsMargins(0, 0, 0, 0);
	fraCredits->setFrameShape(QFrame::NoFrame);
	fraCredits->layout()->setContentsMargins(0, 0, 0, 0);
#endif

	// Initialize the About Dialog text.
	d->initAboutDialogText();
}


/**
 * Shut down the About Dialog.
 */
AboutDialog::~AboutDialog()
{
	// Clear the m_AboutDialog pointer.
	AboutDialogPrivate::ms_AboutDialog = NULL;

	delete d;
}


/**
 * Show a single instance of the About Dialog.
 * @param parent Parent window.
 */
void AboutDialog::ShowSingle(QWidget *parent)
{
	if (AboutDialogPrivate::ms_AboutDialog != NULL) {
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
		retranslateUi(this);

		// Reinitialize the About Dialog text.
		d->initAboutDialogText();
	}

	// Pass the event to the base class.
	this->QDialog::changeEvent(event);
}
