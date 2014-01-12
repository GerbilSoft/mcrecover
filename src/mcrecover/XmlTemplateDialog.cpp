/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * XmlTemplateDialog.cpp: XML template dialog.                             *
 *                                                                         *
 * Copyright (c) 2014 by David Korth.                                      *
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
#include "MemCardFile.hpp"

/** XmlTemplateDialogPrivate **/

class XmlTemplateDialogPrivate
{
	public:
		XmlTemplateDialogPrivate(XmlTemplateDialog *q, const MemCardFile *file);

	protected:
		XmlTemplateDialog *const q_ptr;
		Q_DECLARE_PUBLIC(XmlTemplateDialog)
	private:
		Q_DISABLE_COPY(XmlTemplateDialogPrivate)

	public:
		const MemCardFile *file;

		// XML template.
		QString xmlTemplate;

		/**
		 * Update the window text.
		 */
		void updateWindowText(void);
};

XmlTemplateDialogPrivate::XmlTemplateDialogPrivate(XmlTemplateDialog* q, const MemCardFile *file)
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
		QString gameID = file->gamecode() + file->company();

		//: Window title: %1 == game ID; %2 == internal filename.
		winTitle = XmlTemplateDialog::tr("Generated XML Template: %1/%2")
			.arg(gameID)
			.arg(file->filename());

		//: Template description: %1 == game ID; %2 == internal filename.
		templateDesc = XmlTemplateDialog::tr(
			"Generated XML template for: %1/%2\n"
			"You may need to add variable modifiers.")
			.arg(gameID)
			.arg(file->filename());
	} else {
		//: Window title: No file loaded.
		winTitle = XmlTemplateDialog::tr("Generated XML Template: No file loaded");
		//: Template description: No file loaded.
		templateDesc = XmlTemplateDialog::tr("No file loaded.") + QChar(L'\n');
	}

	q->setWindowTitle(winTitle);
	q->lblTemplateDesc->setText(templateDesc);
	q->txtTemplate->setPlainText(xmlTemplate);
}

/** XmlTemplateDialog **/

/**
 * Initialize the XML Template Dialog.
 * @param parent Parent widget.
 */
XmlTemplateDialog::XmlTemplateDialog(QWidget *parent)
	: QDialog(parent,
		Qt::Dialog |
		Qt::WindowTitleHint |
		Qt::WindowSystemMenuHint |
		Qt::WindowMinimizeButtonHint |
		Qt::WindowCloseButtonHint)
	, d_ptr(new XmlTemplateDialogPrivate(this, nullptr))
{
	init();
}

/**
 * Initialize the XML Template Dialog.
 * @param file MemCardFile to display.
 * @param parent Parent widget.
 */
XmlTemplateDialog::XmlTemplateDialog(const MemCardFile *file, QWidget *parent)
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
	setupUi(this);

	// Make sure the window is deleted on close.
	this->setAttribute(Qt::WA_DeleteOnClose, true);

#ifdef Q_OS_MAC
	// Remove the window icon. (Mac "proxy icon")
	this->setWindowIcon(QIcon());
#endif

	Q_D(XmlTemplateDialog);

	// Generate the XML.
	// TODO

	// Update the window text.
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
		retranslateUi(this);

		// Update the window text to retranslate descriptions.
		Q_D(XmlTemplateDialog);
		d->updateWindowText();
	}

	// Pass the event to the base class.
	this->QDialog::changeEvent(event);
}
