/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * XmlTemplateDialog.cpp: XML template dialog.                             *
 *                                                                         *
 * Copyright (c) 2014-2015 by David Korth.                                 *
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

#include "XmlTemplateDialog.hpp"

// MemCard
#include "card/GcnFile.hpp"

// Qt includes.
#include <QtCore/QXmlStreamWriter>

// C includes. (C++ namespace)
#include <cstdio>

/** XmlTemplateDialogPrivate **/

#include "ui_XmlTemplateDialog.h"
class XmlTemplateDialogPrivate
{
	public:
		XmlTemplateDialogPrivate(XmlTemplateDialog *q, const GcnFile *file);

	protected:
		XmlTemplateDialog *const q_ptr;
		Q_DECLARE_PUBLIC(XmlTemplateDialog)
	private:
		Q_DISABLE_COPY(XmlTemplateDialogPrivate)

	public:
		Ui::XmlTemplateDialog ui;

		const GcnFile *file;

		// XML template.
		QString xmlTemplate;

		/**
		 * Update the window text.
		 */
		void updateWindowText(void);

		/**
		 * Escape a string for use as a PCRE regular expression.
		 * This also adds ^ and $ to the beginning and end of the string.
		 * @return Escaped string.
		 */
		static QString escapeString(const QString &str);

		/**
		 * Generate the XML template.
		 */
		void generateXmlTemplate(void);
};

XmlTemplateDialogPrivate::XmlTemplateDialogPrivate(XmlTemplateDialog* q, const GcnFile *file)
	: q_ptr(q)
	, file(file)
{ }

/**
 * Update the window text.
 */
void XmlTemplateDialogPrivate::updateWindowText(void)
{
	Q_Q(XmlTemplateDialog);

	QString winTitle, templateDesc;
	if (file) {
		//: Window title: %1 == game ID; %2 == internal filename.
		winTitle = XmlTemplateDialog::tr("Generated XML Template: %1/%2")
			.arg(file->id6())
			.arg(file->filename());

		//: Template description: %1 == game ID; %2 == internal filename.
		templateDesc = XmlTemplateDialog::tr(
			"Generated XML template for: %1/%2\n"
			"You will need to edit gameName and fileInfo,\n"
			"and may also need to add variable modifiers.")
			.arg(file->id6())
			.arg(file->filename());
	} else {
		//: Window title: No file loaded.
		winTitle = XmlTemplateDialog::tr("Generated XML Template: No file loaded");
		//: Template description: No file loaded.
		templateDesc = XmlTemplateDialog::tr("No file loaded.") + QChar(L'\n') + QChar(L'\n');
	}

	q->setWindowTitle(winTitle);
	ui.lblTemplateDesc->setText(templateDesc);
	ui.txtTemplate->setPlainText(xmlTemplate);
}

/**
 * Escape a string for use as a PCRE regular expression.
 * This also adds ^ and $ to the beginning and end of the string.
 * @return Escaped string.
 */
QString XmlTemplateDialogPrivate::escapeString(const QString &str)
{
	QString esc_str;
	esc_str.reserve(str.size() + 8);

	esc_str.append(QChar(L'^'));
	for (int i = 0; i < str.size(); i++) {
		QChar chr = str.at(i);
		switch (chr.unicode()) {
			case L'.': case L'^':
			case L'$': case L'*':
			case L'+': case L'?':
			case L'(': case L')':
			case L'[': case L'{':
			case L'\\': case L'|':
				// Escape this character.
				esc_str += QChar(L'\\');
				// fall-through
			default:
				esc_str += chr;
		}
	}
	esc_str.append(QChar(L'$'));

	return esc_str;
}

/**
 * Generate the XML template.
 */
void XmlTemplateDialogPrivate::generateXmlTemplate(void)
{
	xmlTemplate.clear();
	xmlTemplate.reserve(1024);

	if (!file) {
		// No file is loaded.
		return;
	}

	const card_direntry *dirEntry = file->dirEntry();
	char tmp[16];

	QXmlStreamWriter xml(&xmlTemplate);
	xml.setAutoFormatting(true);
	xml.setAutoFormattingIndent(-1);
	xml.writeStartDocument();

	// <file> block.
	xml.writeStartElement(QLatin1String("file"));
	xml.writeTextElement(QLatin1String("gameName"), file->gameDesc());
	xml.writeTextElement(QLatin1String("fileInfo"), QLatin1String("Save File"));
	// TODO: Allow combined ID6 tag?
	xml.writeTextElement(QLatin1String("gamecode"), file->id4());
	xml.writeTextElement(QLatin1String("company"), file->company());

	// <search> block.
	xml.writeStartElement(QLatin1String("search"));
	snprintf(tmp, sizeof(tmp), "0x%04X", dirEntry->commentaddr);
	xml.writeTextElement(QLatin1String("address"), QLatin1String(tmp));
	xml.writeTextElement(QLatin1String("gameDesc"), escapeString(file->gameDesc()));
	xml.writeTextElement(QLatin1String("fileDesc"), escapeString(file->fileDesc()));
	xml.writeEndElement();

	// <variables> block.
	// TODO: Autodetect certain variables?

	// <dirEntry> block.
	xml.writeStartElement(QLatin1String("dirEntry"));
	xml.writeTextElement(QLatin1String("filename"), file->filename());
	snprintf(tmp, sizeof(tmp), "0x%02X", dirEntry->bannerfmt);
	xml.writeTextElement(QLatin1String("bannerFormat"), QLatin1String(tmp));
	snprintf(tmp, sizeof(tmp), "0x%04X", dirEntry->iconaddr);
	xml.writeTextElement(QLatin1String("iconAddress"), QLatin1String(tmp));
	snprintf(tmp, sizeof(tmp), (dirEntry->iconfmt <= 0xFF ? "0x%02X" : "0x%04X"), dirEntry->iconfmt);
	xml.writeTextElement(QLatin1String("iconFormat"), QLatin1String(tmp));
	snprintf(tmp, sizeof(tmp), (dirEntry->iconspeed <= 0xFF ? "0x%02X" : "0x%04X"), dirEntry->iconspeed);
	xml.writeTextElement(QLatin1String("iconSpeed"), QLatin1String(tmp));
	snprintf(tmp, sizeof(tmp), "0x%02X", dirEntry->permission);
	xml.writeTextElement(QLatin1String("permission"), QLatin1String(tmp));
	xml.writeTextElement(QLatin1String("length"), QString::number(file->size()));
	xml.writeEndElement();

	// </file>
	xml.writeEndElement();

	// End of fragment.
	xml.writeEndDocument();

	// Remove the "<?xml" line.
	// This is an XML fragment, not a full document.
	int n = xmlTemplate.indexOf(L'\n');
	if (n >= 0)
		xmlTemplate.remove(0, n + 1);
}

/** XmlTemplateDialog **/

/**
 * Initialize the XML Template Dialog.
 * @param parent Parent widget.
 */
XmlTemplateDialog::XmlTemplateDialog(QWidget *parent)
	: QDialog(parent,
		Qt::Dialog |
		Qt::CustomizeWindowHint |
		Qt::WindowTitleHint |
		Qt::WindowSystemMenuHint |
		Qt::WindowCloseButtonHint)
	, d_ptr(new XmlTemplateDialogPrivate(this, nullptr))
{
	init();
}

/**
 * Initialize the XML Template Dialog.
 * @param file GcnFile to display.
 * @param parent Parent widget.
 */
XmlTemplateDialog::XmlTemplateDialog(const GcnFile *file, QWidget *parent)
	: QDialog(parent,
		Qt::Dialog |
		Qt::WindowTitleHint |
		Qt::WindowSystemMenuHint |
		Qt::WindowMinimizeButtonHint |
		Qt::WindowCloseButtonHint)
	, d_ptr(new XmlTemplateDialogPrivate(this, file))
{
	init();
}

/**
 * Common initialization function for all constructors.
 */
void XmlTemplateDialog::init(void)
{
	Q_D(XmlTemplateDialog);
	d->ui.setupUi(this);

	// Make sure the window is deleted on close.
	this->setAttribute(Qt::WA_DeleteOnClose, true);

#ifdef Q_OS_MAC
	// Remove the window icon. (Mac "proxy icon")
	this->setWindowIcon(QIcon());
#endif

	// Set the correct font on the QPlainTextEdit widget.
	// Qt/Windows doesn't recognize the Monospace font,
	// so we have to initialize the font family to ""
	// and set the style hint to QFont::TypeWriter.
	// FIXME: Font is a bit small on Windows...
	QFont fntMonospace(QLatin1String(""));
	fntMonospace.setStyleHint(QFont::TypeWriter);
	d->ui.txtTemplate->setFont(fntMonospace);

	// FIXME: QPlainTextEdit cursor doesn't show up when readOnly.

	// Update the window text.
	d->generateXmlTemplate();
	d->updateWindowText();
}

/**
 * Shut down the XML Template Dialog.
 */
XmlTemplateDialog::~XmlTemplateDialog()
{
	delete d_ptr;
}

/**
 * Widget state has changed.
 * @param event State change event.
 */
void XmlTemplateDialog::changeEvent(QEvent *event)
{
	if (event->type() == QEvent::LanguageChange) {
		// Retranslate the UI.
		Q_D(XmlTemplateDialog);
		d->ui.retranslateUi(this);

		// Update the window text to retranslate descriptions.
		d->generateXmlTemplate();
		d->updateWindowText();
	}

	// Pass the event to the base class.
	this->QDialog::changeEvent(event);
}
