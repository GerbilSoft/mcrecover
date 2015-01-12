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

// Files.
#include "card/File.hpp"

#include "sa_defs.h"

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
	: QWidget(parent)
	, d_ptr(new SAEditorPrivate(this))
{
	Q_D(SAEditor);
	d->ui.setupUi(this);
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
	this->QWidget::changeEvent(event);
}

/** Public functions. **/

/**
 * Set the File to edit.
 * @param file File to edit.
 */
void SAEditor::setFile(File *file)
{
	// TODO: Save the GcnFile.
	// TODO: Verify that this is an SADX file.
	QByteArray data = file->loadFileData();
	if (data.size() < (SA_SAVE_ADDRESS_GCN + SA_SAVE_FILE_LEN))
		return;

	Q_D(SAEditor);
	d->ui.saLevelStats->loadSaveData((const sa_save_file*)(data.data() + SA_SAVE_ADDRESS_GCN));
}
