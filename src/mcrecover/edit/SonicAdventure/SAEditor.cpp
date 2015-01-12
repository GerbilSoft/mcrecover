/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * SAEditor.cpp: Sonic Adventure - save file editor.           *
 *                                                                         *
 * Copyright (c) 2015 by David Korth.                                      *
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

#include "SAEditor.hpp"

// Qt includes.
#include <QtCore/QEvent>

/** SAEditorPrivate **/

#include "ui_SAEditor.h"
class SAEditorPrivate
{
	public:
		SAEditorPrivate(SAEditor *q);

	protected:
		SAEditor *const q_ptr;
		Q_DECLARE_PUBLIC(SAEditor)
	private:
		Q_DISABLE_COPY(SAEditorPrivate)

	public:
		Ui::SAEditor ui;
};

SAEditorPrivate::SAEditorPrivate(SAEditor* q)
	: q_ptr(q)
{ }

/** SAEditor **/

/**
 * Initialize the Sonic Adventure save file editor.
 * @param parent Parent widget.
 */
SAEditor::SAEditor(QWidget *parent)
	: QDialog(parent,
		Qt::Dialog |
		Qt::CustomizeWindowHint |
		Qt::WindowTitleHint |
		Qt::WindowSystemMenuHint |
		Qt::WindowCloseButtonHint)
	, d_ptr(new SAEditorPrivate(this))
{
	Q_D(SAEditor);
	d->ui.setupUi(this);

	// NOTE: This window should NOT be deleted on close,
	// because the owner needs to retrieve the modified.
	// save file data.
	//this->setAttribute(Qt::WA_DeleteOnClose, true);

#ifdef Q_OS_MAC
	// Remove the window icon. (Mac "proxy icon")
	this->setWindowIcon(QIcon());
#endif
}

/**
 * Shut down the Sonic Adventure save file editor.
 */
SAEditor::~SAEditor()
{
	delete d_ptr;
}

/**
 * Widget state has changed.
 * @param event State change event.
 */
void SAEditor::changeEvent(QEvent *event)
{
	if (event->type() == QEvent::LanguageChange) {
		// Retranslate the UI.
		Q_D(SAEditor);
		d->ui.retranslateUi(this);
	}

	// Pass the event to the base class.
	this->QDialog::changeEvent(event);
}
