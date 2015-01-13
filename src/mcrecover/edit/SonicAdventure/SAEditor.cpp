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

// C includes. (C++ namespace)
#include <cstdlib>

// Qt includes.
#include <QtCore/QEvent>

// Files.
#include "card/File.hpp"

#include "util/byteswap.h"
#include "sa_defs.h"

// TODO: Put this in a common header file somewhere.
#define NUM_ELEMENTS(x) ((int)(sizeof(x) / sizeof(x[0])))

/** SAEditorPrivate **/

#include "ui_SAEditor.h"
class SAEditorPrivate
{
	public:
		SAEditorPrivate(SAEditor *q);
		~SAEditorPrivate();

	protected:
		SAEditor *const q_ptr;
		Q_DECLARE_PUBLIC(SAEditor)
	private:
		Q_DISABLE_COPY(SAEditorPrivate)

	public:
		Ui::SAEditor ui;

		// File being edited.
		// TODO: EditorManager to handle File being destroyed.
		File *file;

		// sa_save_slot structs.
		QVector<sa_save_slot*> data;
		int slot; // active slot (-1 for none)

		/**
		 * Clear the sa_save_slot structs.
		 */
		void clearData(void);

		/**
		 * Update the display.
		 */
		void updateDisplay(void);
};

SAEditorPrivate::SAEditorPrivate(SAEditor* q)
	: q_ptr(q)
	, slot(-1)
{ }

SAEditorPrivate::~SAEditorPrivate()
{
	clearData();
}

/**
 * Clear the sa_save_slot structs.
 */
void SAEditorPrivate::clearData(void)
{
	// Delete all loaded sa_save structs.
	// NOTE: sa_save_slot is a C struct, so use free().
	foreach (sa_save_slot *sa_save, data) {
		free(sa_save);
	}
	data.clear();
}

/**
 * Update the display.
 */
void SAEditorPrivate::updateDisplay(void)
{
	const int slotCount = data.size();
	if (slot < 0)
		slot = 0;
	if (slot >= slotCount)
		slot = (slotCount - 1);

	// Show the slot selector if we have more than one slot.
	ui.slotSelector->setVisible(slotCount > 1);

	if (slot < 0 || slot >= slotCount) {
		// Invalid slot number.
		ui.saLevelStats->clear();
		return;
	}

	// Display the data.
	sa_save_slot *sa_save = data.at(slot);
	ui.saLevelStats->load(sa_save);
}

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
 * Get the file currently being edited.
 * @return File being edited, or nullptr if none.
 */
File *SAEditor::file(void) const
{
	Q_D(const SAEditor);
	return d->file;
}

/**
 * Set the File to edit.
 * @param file File to edit.
 * @return 0 on success; non-zero on error (and file will not be set).
 * TODO: Error code constants?
 */
int SAEditor::setFile(File *file)
{
	Q_D(SAEditor);
	d->clearData();
	d->file = file;
	int ret = -1;

	QByteArray data = file->loadFileData();
	if (file->filename() == QLatin1String("SONICADV_SYS") ||
	    file->filename() == QLatin1String("SONICADV_INT"))
	{
		// DC version.
		// TODO: Verify that this is an SA1 file.
		// TODO: Show a slot selector.
		if (data.size() < (SA_SAVE_ADDRESS_DC_0 + (SA_SAVE_SLOT_LEN * 3))) {
			ret = -1;
			goto end;
		}

		// Three, count 'em, *three* save slots!
		const char *src = (data.data() + SA_SAVE_ADDRESS_DC_0);
		for (int i = 0; i < 3; i++, src += SA_SAVE_SLOT_LEN) {
			sa_save_slot *sa_save = (sa_save_slot*)malloc(sizeof(*sa_save));
			memcpy(sa_save, src, SA_SAVE_SLOT_LEN);
			d->data.append(sa_save);

#if MCRECOVER_BYTEORDER == MCRECOVER_BIG_ENDIAN
			// Byteswap the data.
			// Dreamcast's SH-4 is little-endian.
			for (int i = 0; i < NUM_ELEMENTS(sa_save->scores.all); i++) {
				sa_save->scores.all[i] = le32_to_cpu(sa_save->scores.all[i]);
			}
			for (int i = 0; i < NUM_ELEMENTS(sa_save->weights.all); i++) {
				sa_save->weights.all[i] = le16_to_cpu(sa_save->weights.all[i]);
			}
			for (int i = 0; i < NUM_ELEMENTS(sa_save->rings.all); i++) {
				sa_save->rings.all[i] = le16_to_cpu(sa_save->rings.all[i]);
			}
#endif

			// Loaded successfully.
			ret = 0;
		}
	} else if (file->filename().startsWith(QLatin1String("SONICADVENTURE_DX_PLAYRECORD_"))) {
		// GameCube verison.
		// TODO: Verify that this is an SADX file.
		if (data.size() < (SA_SAVE_ADDRESS_GCN + SA_SAVE_SLOT_LEN)) {
			ret = -1;
			goto end;
		}

		// Only one save slot.
		sa_save_slot *sa_save = (sa_save_slot*)malloc(sizeof(*sa_save));
		memcpy(sa_save, (data.data() + SA_SAVE_ADDRESS_GCN), SA_SAVE_SLOT_LEN);
		d->data.append(sa_save);

#if MCRECOVER_BYTEORDER == MCRECOVER_LIL_ENDIAN
		// Byteswap the data.
		// GameCube's PowerPC 750CL is big-endian.
		for (int i = 0; i < NUM_ELEMENTS(sa_save->scores.all); i++) {
			sa_save->scores.all[i] = be32_to_cpu(sa_save->scores.all[i]);
		}
		for (int i = 0; i < NUM_ELEMENTS(sa_save->weights.all); i++) {
			sa_save->weights.all[i] = be16_to_cpu(sa_save->weights.all[i]);
		}
		for (int i = 0; i < NUM_ELEMENTS(sa_save->rings.all); i++) {
			sa_save->rings.all[i] = be16_to_cpu(sa_save->rings.all[i]);
		}
#endif

		// Loaded successfully.
		ret = 0;
	} else {
		// Unsupported file.
		ret = -2;
		d->file = nullptr;
		goto end;
	}

end:
	// Update the display.
	// TODO: Connect slotCountChanged()?
	const int slotCount = d->data.size();
	d->ui.slotSelector->setSlotCount(slotCount);
	d->ui.slotSelector->setSlot(0);
	d->updateDisplay();
	return ret;
}

/** Widget slots. **/

/**
 * Slot selector's slot has changed.
 * @param slot New slot.
 */
void SAEditor::on_slotSelector_slotChanged(int slot)
{
	Q_D(SAEditor);
	const int slotCount = d->data.size();
	if (slot < 0)
		slot = 0;
	if (slot >= slotCount)
		slot = (slotCount - 1);

	if (slot < 0 || slot >= slotCount) {
		// Invalid slot number.
		return;
	}

	// TODO: Save data for the current slot.
	d->slot = slot;
	d->updateDisplay();
}
