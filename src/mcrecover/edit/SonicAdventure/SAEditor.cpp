/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * SAEditor.cpp: Sonic Adventure - save file editor.                       *
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
#include <cassert>

// Qt includes.
#include <QtCore/QEvent>

// Files.
#include "card/File.hpp"

#include "util/byteswap.h"
#include "sa_defs.h"

// BitFlags
#include "BitFlagsModel.hpp"
#include "SAEventFlags.hpp"
#include "SANPCFlags.hpp"

// ByteFlags
#include "ByteFlagsModel.hpp"
#include "SADXMissionFlags.hpp"

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
		QVector<sa_save_slot*> data_main;
		QVector<sadx_extra_save_slot*> data_sadx;
		int slot; // active slot (-1 for none)

		// BitFlagsModel objects.
		// Used for event flags, NPC flags, etc.
		// Since BitFlagsView uses a QTreeView with a list model
		// directly, we're storing the data here instead of
		// having BitFlagsView store the data.
		SAEventFlags saEventFlags;
		SANPCFlags saNPCFlags;
		BitFlagsModel *saEventFlagsModel;
		BitFlagsModel *saNPCFlagsModel;

		// ByteFlagsModel objects.
		SADXMissionFlags sadxMissionFlags;
		ByteFlagsModel *sadxMissionFlagsModel;

		/**
		 * Clear the sa_save_slot structs.
		 */
		void clearData(void);

		/**
		 * Update the display.
		 */
		void updateDisplay(void);

		/**
		 * Save data for the current slot.
		 */
		void saveCurrentSlot(void);

		/**
		 * Byteswap an sa_save_slot.
		 * @param sa_save sa_save_slot.
		 */
		static void byteswap_sa_save_slot(sa_save_slot *sa_save);
};

SAEditorPrivate::SAEditorPrivate(SAEditor* q)
	: q_ptr(q)
	, slot(-1)
	, saEventFlagsModel(nullptr)
	, saNPCFlagsModel(nullptr)
	, sadxMissionFlags(nullptr)
{
	// Make sure sa_defs.h is correct.
	static_assert(SA_SCORES_LEN == 128, "SA_SCORES_LEN is incorrect");
	static_assert(sizeof(sa_scores) == SA_SCORES_LEN, "sa_scores has the wrong size");
	static_assert(SA_TIMES_LEN == 84, "SA_TIMES_LEN is incorrect");
	static_assert(sizeof(sa_times) == SA_TIMES_LEN, "sa_times has the wrong size");
	static_assert(SA_WEIGHTS_LEN == 24, "SA_WEIGHTS_LEN is incorrect");
	static_assert(sizeof(sa_weights) == SA_WEIGHTS_LEN, "sa_weights has the wrong size");
	static_assert(SA_RINGS_LEN == 64, "SA_RINGS_LEN is incorrect");
	static_assert(sizeof(sa_rings) == SA_RINGS_LEN, "sa_rings has the wrong size");
	static_assert(SA_EVENT_FLAGS_LEN == 64, "SA_EVENT_FLAGS_LEN is incorrect");
	static_assert(sizeof(sa_event_flags) == SA_EVENT_FLAGS_LEN, "sa_event_flags has the wrong size");
	static_assert(SA_NPC_FLAGS_LEN == 64, "SA_NPC_FLAGS_LEN is incorrect");
	static_assert(sizeof(sa_npc_flags) == SA_NPC_FLAGS_LEN, "sa_npc_flags has the wrong size");
	static_assert(SA_ADVENTURE_MODE_LEN == 96, "SA_ADVENTURE_MODE_LEN is incorrect");
	static_assert(sizeof(sa_adventure_mode) == SA_ADVENTURE_MODE_LEN, "sa_adventure_mode has the wrong size");
	static_assert(SA_LEVEL_CLEAR_COUNT_LEN == 344, "SA_LEVEL_CLEAR_COUNT_LEN is incorrect");
	static_assert(sizeof(sa_level_clear_count) == SA_LEVEL_CLEAR_COUNT_LEN, "sa_level_clear_count has the wrong size");

	static_assert(SA_SAVE_SLOT_LEN == 1184, "SA_SAVE_SLOT_LEN is incorrect");
	static_assert(sizeof(sa_save_slot) == SA_SAVE_SLOT_LEN, "sa_save_file has the wrong size");

	static_assert(SADX_EXTRA_SAVE_SLOT_LEN == 208, "SADX_EXTRA_SAVE_SLOT_LEN is incorrect");
	static_assert(sizeof(sadx_extra_save_slot) == SADX_EXTRA_SAVE_SLOT_LEN, "sadx_extra_save_slot has the wrong size");
}

SAEditorPrivate::~SAEditorPrivate()
{
	clearData();
}

/**
 * Clear the sa_save_slot structs.
 */
void SAEditorPrivate::clearData(void)
{
	// Delete all loaded sa_save_slot structs.
	// NOTE: sa_save_slot is a C struct, so use free().
	foreach (sa_save_slot *sa_save, data_main) {
		free(sa_save);
	}
	data_main.clear();

	// Delete loaded SADX extra save structs.
	// NOTE: sadx_extra_save_slot is a C struct, so use free().
	foreach (sadx_extra_save_slot *sadx_extra_save, data_sadx) {
		free(sadx_extra_save);
	}
	data_sadx.clear();
}

/**
 * Update the display.
 */
void SAEditorPrivate::updateDisplay(void)
{
	const int slotCount = data_main.size();
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
	const sa_save_slot *sa_save = data_main.at(slot);
	ui.saGeneral->load(sa_save);
	ui.saAdventure->load(sa_save);
	ui.saLevelStats->load(sa_save);
	ui.saMiscEmblems->load(sa_save);
	ui.saLevelClearCount->load(sa_save);

	// Bit flags.
	saEventFlags.setAllFlags(&sa_save->events.all[0], NUM_ELEMENTS(sa_save->events.all));
	saNPCFlags.setAllFlags(&sa_save->npc.all[0], NUM_ELEMENTS(sa_save->npc.all));

	// SADX-specific data.
	// NOTE: There's no way to hide specific tabs without removing them
	// from QTabWidget entirely. There is a stylesheet hack to hide
	// disabled tabs, but it didn't work for me.
	// http://qt-project.org/forums/viewthread/24364
	Q_Q(SAEditor);
	const int sadx_tab_idx = ui.tabWidget->indexOf(ui.tabSADXExtras);
	const sadx_extra_save_slot *sadx_extra_save = nullptr;
	if (slot <= data_sadx.size())
		sadx_extra_save = data_sadx.at(slot);
	if (sadx_extra_save) {
		// SADX extra data found. Load it.

		// Missions.
		sadxMissionFlags.setAllFlags(&sadx_extra_save->missions[0],
				NUM_ELEMENTS(sadx_extra_save->missions));

		if (sadx_tab_idx < 0) {
			// Show the "SADX Extras" tab.
			ui.tabWidget->insertTab(ui.tabWidget->indexOf(ui.tabLevelClearCount), ui.tabSADXExtras, SAEditor::tr("SAD&X Extras"));
			ui.tabSADXExtras->show();
		}
	} else {
		// No SADX extra data.
		if (sadx_tab_idx >= 0) {
			// Hide the "SADX Extras" tab.
			// TODO: Verify that taking ownership ensures the object
			// is deleted properly and there are no memory leaks.
			ui.tabWidget->removeTab(sadx_tab_idx);
			ui.tabSADXExtras->hide();
			ui.tabSADXExtras->setParent(q);
		}
	}
}

/**
 * Save data for the current slot.
 */
void SAEditorPrivate::saveCurrentSlot(void)
{
	const int slotCount = data_main.size();
	if (slot < 0 || slot >= slotCount) {
		// Invalid slot number.
		return;
	}

	// Save the data.
	sa_save_slot *sa_save = data_main.at(slot);
	ui.saGeneral->save(sa_save);
	ui.saAdventure->load(sa_save);
	ui.saLevelStats->save(sa_save);
	ui.saMiscEmblems->save(sa_save);
	ui.saLevelClearCount->save(sa_save);

	// Bit flags.
	saEventFlags.allFlags(&sa_save->events.all[0], NUM_ELEMENTS(sa_save->events.all));
	saNPCFlags.allFlags(&sa_save->npc.all[0], NUM_ELEMENTS(sa_save->npc.all));
}

/**
 * Byteswap an sa_save_slot.
 * @param sa_save sa_save_slot.
 */
void SAEditorPrivate::byteswap_sa_save_slot(sa_save_slot *sa_save)
{
	sa_save->playTime = __swab32(sa_save->playTime);

	for (int i = 0; i < NUM_ELEMENTS(sa_save->scores.all); i++) {
		sa_save->scores.all[i] = __swab32(sa_save->scores.all[i]);
	}
	for (int i = 0; i < NUM_ELEMENTS(sa_save->weights.all); i++) {
		sa_save->weights.all[i] = __swab16(sa_save->weights.all[i]);
	}
	for (int i = 0; i < NUM_ELEMENTS(sa_save->rings.all); i++) {
		sa_save->rings.all[i] = __swab16(sa_save->rings.all[i]);
	}

	sa_save->last_level = __swab16(sa_save->last_level);

	// Adventure mode.
	for (int i = 0; i < NUM_ELEMENTS(sa_save->adventure_mode.chr); i++) {
		// NOTE: We can't use __swab16() to byteswap
		// int16_t due to sign extension.
		// TODO: Move to byteswap.h?
		#define signed_swab16(x) ((((x) >> 8) & 0xFF) | (((x) & 0xFF) << 8))
		sa_save->adventure_mode.chr[i].unknown1 =
			signed_swab16(sa_save->adventure_mode.chr[i].unknown1);
		sa_save->adventure_mode.chr[i].unknown2 =
			signed_swab16(sa_save->adventure_mode.chr[i].unknown2);
		sa_save->adventure_mode.chr[i].start_entrance =
			__swab16(sa_save->adventure_mode.chr[i].start_entrance);
		sa_save->adventure_mode.chr[i].start_level_and_act =
			__swab16(sa_save->adventure_mode.chr[i].start_level_and_act);
		sa_save->adventure_mode.chr[i].unknown3 =
			signed_swab16(sa_save->adventure_mode.chr[i].unknown3);
	}
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

	// SAEventFlags model and widget.
	d->saEventFlagsModel = new BitFlagsModel(this);
	d->saEventFlagsModel->setBitFlags(&d->saEventFlags);
	d->ui.saEventFlagsView->setPageSize(64);
	d->ui.saEventFlagsView->setBitFlagsModel(d->saEventFlagsModel);

	// SANPCFlags model and widget.
	d->saNPCFlagsModel = new BitFlagsModel(this);
	d->saNPCFlagsModel->setBitFlags(&d->saNPCFlags);
	d->ui.saNPCFlagsView->setPageSize(0);
	d->ui.saNPCFlagsView->setBitFlagsModel(d->saNPCFlagsModel);

	// SADXMissionFlags model and widget.
	d->sadxMissionFlagsModel = new ByteFlagsModel(this);
	d->sadxMissionFlagsModel->setByteFlags(&d->sadxMissionFlags);
	d->ui.sadxMissionFlagsView->setPageSize(0);
	d->ui.sadxMissionFlagsView->setByteFlagsModel(d->sadxMissionFlagsModel);

	// Attempt to fix the scroll area's minimum width.
	// TODO: On theme change also?
	int w = d->ui.saLevelClearCount->sizeHint().width();
	w += qApp->style()->pixelMetric(QStyle::PM_ScrollBarExtent);
	d->ui.scrlLevelClearCount->setMinimumWidth(w);
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
			d->data_main.append(sa_save);
			d->data_sadx.append(nullptr);	// DC version - no SADX extras.

#if MCRECOVER_BYTEORDER == MCRECOVER_BIG_ENDIAN
			// Byteswap the data.
			// Dreamcast's SH-4 is little-endian.
			d->byteswap_sa_save_slot(sa_save);
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
		d->data_main.append(sa_save);

#if MCRECOVER_BYTEORDER == MCRECOVER_LIL_ENDIAN
		// Byteswap the data.
		// GameCube's PowerPC 750CL is big-endian.
		d->byteswap_sa_save_slot(sa_save);
#endif

		// Check for SADX extras.
		if (data.size() >= (SA_SAVE_ADDRESS_GCN + SA_SAVE_SLOT_LEN + SADX_EXTRA_SAVE_SLOT_LEN)) {
			// Found SADX extras.
			sadx_extra_save_slot *sadx_extra_save = (sadx_extra_save_slot*)malloc(sizeof(*sadx_extra_save));
			memcpy(sadx_extra_save, (data.data() + SA_SAVE_ADDRESS_GCN + SA_SAVE_SLOT_LEN), SADX_EXTRA_SAVE_SLOT_LEN);
			d->data_sadx.append(sadx_extra_save);

			// TODO: Byteswapping.
		} else {
			// No SADX extras.
			d->data_sadx.append(nullptr);
		}

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
	const int slotCount = d->data_main.size();
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
	const int slotCount = d->data_main.size();
	if (slot < 0)
		slot = 0;
	if (slot >= slotCount)
		slot = (slotCount - 1);

	if (slot < 0 || slot >= slotCount) {
		// Invalid slot number.
		return;
	}

	d->saveCurrentSlot();
	d->slot = slot;
	d->updateDisplay();
}
