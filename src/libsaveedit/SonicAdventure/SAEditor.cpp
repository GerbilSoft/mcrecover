/***************************************************************************
 * GameCube Memory Card Recovery Program [libsaveedit]                     *
 * SAEditor.cpp: Sonic Adventure - save file editor.                       *
 *                                                                         *
 * Copyright (c) 2015-2018 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#include "SAEditor.hpp"

// C includes. (C++ namespace)
#include <cstdlib>
#include <cassert>

// Qt includes.
#include <QtCore/QEvent>

// Files.
#include "libmemcard/File.hpp"
#include "libmemcard/GcnFile.hpp"
#include "libmemcard/VmuFile.hpp"

#include "util/byteswap.h"
#include "sa_defs.h"

// BitFlags
#include "../models/BitFlagsModel.hpp"
#include "SAEventFlags.hpp"
#include "SANPCFlags.hpp"

// ByteFlags
#include "../models/ByteFlagsModel.hpp"
#include "SADXMissionFlags.hpp"

// Checksum algorithms.
#include "libgctools/Checksum.hpp"

// TODO: Put this in a common header file somewhere.
#define NUM_ELEMENTS(x) ((int)(sizeof(x) / sizeof(x[0])))

/** SAEditorPrivate **/

#include "ui_SAEditor.h"
#include "../EditorWidget_p.hpp"
class SAEditorPrivate : public EditorWidgetPrivate
{
	public:
		explicit SAEditorPrivate(SAEditor *q);
		virtual ~SAEditorPrivate();

	private:
		typedef EditorWidgetPrivate super;
		Q_DECLARE_PUBLIC(SAEditor)
		Q_DISABLE_COPY(SAEditorPrivate)

	public:
		Ui::SAEditor ui;

		// sa_save_slot structs.
		QVector<sa_save_slot*> data_main;
		QVector<sadx_extra_save_slot*> data_sadx;

		// Editor widgets. (non-flags)
		QVector<SAEditWidget*> saEditWidgets;
		QVector<SADXEditWidget*> sadxEditWidgets;

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
		 * Load data from a file.
		 * @param file File.
		 * @return 0 on success; non-zero on error.
		 */
		int load(File *file);

		/**
		 * Clear the sa_save_slot structs.
		 */
		void clear(void);

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

		/**
		 * Byteswap an sadx_extra_save_slot.
		 * @param sa_save sadx_extra_save_slot.
		 */
		static void byteswap_sadx_extra_save_slot(sadx_extra_save_slot *sadx_extra_save);
};

SAEditorPrivate::SAEditorPrivate(SAEditor* q)
	: super(q)
	, saEventFlagsModel(nullptr)
	, saNPCFlagsModel(nullptr)
	, sadxMissionFlagsModel(nullptr)
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
	static_assert(SA_MINI_GAME_SCORES_LEN == 108, "SA_MINI_GAME_SCORES_LEN is incorrect");
	static_assert(sizeof(sa_mini_game_scores) == SA_MINI_GAME_SCORES_LEN, "sa_mini_game_scores has the wrong size");
	static_assert(SA_TWINKLE_CIRCUIT_TIMES_LEN == 90, "SA_TWINKLE_CIRCUIT_TIMES_LEN is incorrect");
	static_assert(sizeof(sa_twinkle_circuit_times) == SA_TWINKLE_CIRCUIT_TIMES_LEN, "sa_twinkle_circuit_times has the wrong size");
	static_assert(SA_BOSS_ATTACK_TIMES_LEN == 54, "SA_BOSS_ATTACK_TIMES_LEN is incorrect");
	static_assert(sizeof(sa_boss_attack_times) == SA_BOSS_ATTACK_TIMES_LEN, "sa_boss_attack_times is the wrong size");
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

	static_assert(SADX_EXTRA_MINI_GAME_SCORES_METAL_LEN == 24, "SADX_EXTRA_MINI_GAME_SCORES_LEN is incorrect");
	static_assert(sizeof(sadx_extra_mini_game_scores_metal) == SADX_EXTRA_MINI_GAME_SCORES_METAL_LEN, "sadx_extra_mini_game_scores_metal is the wrong size");

	static_assert(SADX_EXTRA_SAVE_SLOT_LEN == 208, "SADX_EXTRA_SAVE_SLOT_LEN is incorrect");
	static_assert(sizeof(sadx_extra_save_slot) == SADX_EXTRA_SAVE_SLOT_LEN, "sadx_extra_save_slot has the wrong size");
}

SAEditorPrivate::~SAEditorPrivate()
{
	clear();
}

/**
 * Load data from a file.
 * @param file File.
 * @return 0 on success; non-zero on error.
 */
int SAEditorPrivate::load(File *file)
{
	// Clear the current data.
	this->file = nullptr;
	clear();

	// Read the new file.
	QByteArray data = file->loadFileData();

	// Determine which version of the game this save file is for.
	// TODO: Test for GCN first, then DC?
	// TODO: Verify checksums?
	int ret = -1;
	if (qobject_cast<VmuFile*>(file) != nullptr) {
		// DC version.

		// Three, count 'em, *three* save slots!
		const char *src = (data.data() + SA_SAVE_ADDRESS_DC_0);
		for (int i = 0; i < 3; i++, src += SA_SAVE_SLOT_LEN) {
			sa_save_slot *sa_save = (sa_save_slot*)malloc(sizeof(*sa_save));
			memcpy(sa_save, src, SA_SAVE_SLOT_LEN);
			data_main.append(sa_save);
			data_sadx.append(nullptr);	// DC version - no SADX extras.
#if SYS_BYTEORDER == SYS_BIG_ENDIAN
			// Byteswap the data.
			// Dreamcast's SH-4 is little-endian.
			byteswap_sa_save_slot(sa_save);
#endif /* SYS_BYTEORDER == SYS_BIG_ENDIAN */

			// Loaded successfully.
			ret = 0;
		}
	} else if (qobject_cast<GcnFile*>(file) != nullptr) {
		// GameCube verison.

		// Only one save slot.
		sa_save_slot *sa_save = (sa_save_slot*)malloc(sizeof(*sa_save));
		memcpy(sa_save, (data.data() + SA_SAVE_ADDRESS_GCN), SA_SAVE_SLOT_LEN);
		data_main.append(sa_save);

#if SYS_BYTEORDER == SYS_LIL_ENDIAN
		// Byteswap the data.
		// GameCube's PowerPC 750 is big-endian.
		byteswap_sa_save_slot(sa_save);
#endif /* SYS_BYTEORDER == SYS_LIL_ENDIAN */

		// Check for SADX extras.
		if (data.size() >= (SA_SAVE_ADDRESS_GCN + SA_SAVE_SLOT_LEN + SADX_EXTRA_SAVE_SLOT_LEN)) {
			// Found SADX extras.
			sadx_extra_save_slot *sadx_extra_save = (sadx_extra_save_slot*)malloc(sizeof(*sadx_extra_save));
			memcpy(sadx_extra_save, (data.data() + SA_SAVE_ADDRESS_GCN + SA_SAVE_SLOT_LEN), SADX_EXTRA_SAVE_SLOT_LEN);
			data_sadx.append(sadx_extra_save);
#if SYS_BYTEORDER == SYS_LIL_ENDIAN
			// Byteswap the data.
			// GameCube's PowerPC 750 is big-endian.
			byteswap_sadx_extra_save_slot(sadx_extra_save);
#endif /* SYS_BYTEORDER == SYS_LIL_ENDIAN */
		} else {
			// No SADX extras.
			data_sadx.append(nullptr);
		}

		// Loaded successfully.
		ret = 0;
	} else {
		// Unsupported file.
		// TODO: Add support for the Windows version.
		ret = -2;
		goto end;
	}

end:
	if (ret == 0) {
		// File loaded successfully.
		this->file = file;
	}

	// Update the display.
	Q_Q(SAEditor);
	setSaveSlots(data_main.size());
	setGeneralSettings(false);
	q->setCurrentSaveSlot(0);
	updateDisplay();
	return ret;
}

/**
 * Clear the sa_save_slot structs.
 */
void SAEditorPrivate::clear(void)
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
	assert(this->currentSaveSlot >= 0 && this->currentSaveSlot < this->saveSlots);

	// Display the data.
	const sa_save_slot *sa_save = data_main.at(this->currentSaveSlot);
	foreach (SAEditWidget *saEditWidget, saEditWidgets) {
		saEditWidget->load(sa_save);
	}

	// Bit flags.
	saEventFlags.setAllFlags(&sa_save->events.all[0], NUM_ELEMENTS(sa_save->events.all));
	saNPCFlags.setAllFlags(&sa_save->npc.all[0], NUM_ELEMENTS(sa_save->npc.all));

	// SADX-specific data.
	// NOTE: There's no way to hide specific tabs without removing them
	// from QTabWidget entirely. There is a stylesheet hack to hide
	// disabled tabs, but it didn't work for me.
	// http://qt-project.org/forums/viewthread/24364
	Q_Q(SAEditor);
	const int missions_tab_idx = ui.tabWidget->indexOf(ui.tabMissions);
	const sadx_extra_save_slot *sadx_extra_save = nullptr;
	if (this->currentSaveSlot < data_sadx.size()) {
		sadx_extra_save = data_sadx.at(this->currentSaveSlot);
	}
	if (sadx_extra_save) {
		// SADX extra data found. Load it.
		foreach (SADXEditWidget *sadxEditWidget, sadxEditWidgets) {
			sadxEditWidget->loadDX(sadx_extra_save);
		}

		// Missions.
		sadxMissionFlags.setAllFlags(&sadx_extra_save->missions[0],
				NUM_ELEMENTS(sadx_extra_save->missions));

		if (missions_tab_idx < 0) {
			// Show the "Missions" tab.
			ui.tabWidget->addTab(ui.tabMissions, SAEditor::tr("M&issions"));
			ui.tabWidget->show();
		}
	} else {
		// No SADX extra data.
		// Make sure the SADX sections are hidden.
		foreach (SADXEditWidget *sadxEditWidget, sadxEditWidgets) {
			sadxEditWidget->loadDX(nullptr);
		}

		if (missions_tab_idx >= 0) {
			// Hide the "Missions" tab.
			// TODO: Verify that taking ownership ensures the object
			// is deleted properly and there are no memory leaks.
			ui.tabWidget->removeTab(missions_tab_idx);
			ui.tabMissions->hide();
			ui.tabMissions->setParent(q);
		}
	}
}

/**
 * Save data for the current slot.
 */
void SAEditorPrivate::saveCurrentSlot(void)
{
	assert(this->currentSaveSlot >= 0 && this->currentSaveSlot < this->saveSlots);

	// Save the data.
	sa_save_slot *sa_save = data_main.at(this->currentSaveSlot);
	foreach (SAEditWidget *saEditWidget, saEditWidgets) {
		saEditWidget->save(sa_save);
	}

	// Bit flags.
	saEventFlags.allFlags(&sa_save->events.all[0], NUM_ELEMENTS(sa_save->events.all));
	saNPCFlags.allFlags(&sa_save->npc.all[0], NUM_ELEMENTS(sa_save->npc.all));

	// SADX extra data?
	sadx_extra_save_slot *sadx_extra_save = nullptr;
	if (this->currentSaveSlot < data_sadx.size()) {
		sadx_extra_save = data_sadx.at(this->currentSaveSlot);
	}
	if (sadx_extra_save) {
		// SADX extra data found. Save it.
		foreach (SADXEditWidget *sadxEditWidget, sadxEditWidgets) {
			sadxEditWidget->saveDX(sadx_extra_save);
		}

		// Missions.
		sadxMissionFlags.allFlags(&sadx_extra_save->missions[0],
				NUM_ELEMENTS(sadx_extra_save->missions));
	}
}

/**
 * Byteswap an sa_save_slot.
 * @param sa_save sa_save_slot.
 */
void SAEditorPrivate::byteswap_sa_save_slot(sa_save_slot *sa_save)
{
	sa_save->playTime = __swab32(sa_save->playTime);

	// TODO: Combine a few of these loops?
	for (int i = 0; i < NUM_ELEMENTS(sa_save->scores.all); i++) {
		sa_save->scores.all[i] = __swab32(sa_save->scores.all[i]);
	}
	for (int i = 0; i < NUM_ELEMENTS(sa_save->weights.all); i++) {
		sa_save->weights.all[i] = __swab16(sa_save->weights.all[i]);
	}
	for (int i = 0; i < NUM_ELEMENTS(sa_save->rings.all); i++) {
		sa_save->rings.all[i] = __swab16(sa_save->rings.all[i]);
	}
	for (int i = 0; i < NUM_ELEMENTS(sa_save->mini_game_scores.all); i++) {
		sa_save->mini_game_scores.all[i] = __swab32(sa_save->mini_game_scores.all[i]);
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

/**
 * Byteswap an sadx_extra_save_slot.
 * @param sa_save sadx_extra_save_slot.
 */
void SAEditorPrivate::byteswap_sadx_extra_save_slot(sadx_extra_save_slot *sadx_extra_save)
{
	// Black Market rings.
	sadx_extra_save->rings_black_market = __swab32(sadx_extra_save->rings_black_market);

	// Metal Sonic level stats.
	for (int i = 0; i < 10; i++) {
		sadx_extra_save->scores_metal[i] =
			__swab32(sadx_extra_save->scores_metal[i]);
		sadx_extra_save->rings_metal[i]  =
			__swab16(sadx_extra_save->rings_metal[i]);
	}

	// Metal Sonic mini-game scores.
	for (int i = 0; i < NUM_ELEMENTS(sadx_extra_save->mini_game_scores_metal.all); i++) {
		sadx_extra_save->mini_game_scores_metal.all[i] =
			__swab32(sadx_extra_save->mini_game_scores_metal.all[i]);
	}

	// Metal Sonic emblems. (32-bit bitfield, host-endian.)
	sadx_extra_save->emblems_metal = __swab32(sadx_extra_save->emblems_metal);
}

/** SAEditor **/

/**
 * Initialize the Sonic Adventure save file editor.
 * @param parent Parent widget.
 */
SAEditor::SAEditor(QWidget *parent)
	: super(new SAEditorPrivate(this), parent)
{
	Q_D(SAEditor);
	d->ui.setupUi(this);

	// Initialize the SAEditWidget vector.
	d->saEditWidgets.reserve(6);
	d->saEditWidgets.append(d->ui.saGeneral);
	d->saEditWidgets.append(d->ui.saAdventure);
	d->saEditWidgets.append(d->ui.saLevelStats);
	d->saEditWidgets.append(d->ui.saSubGames);
	d->saEditWidgets.append(d->ui.saMiscEmblems);
	d->saEditWidgets.append(d->ui.saLevelClearCount);

	// Initialize the SADXEditWidget vector.
	d->saEditWidgets.reserve(3);
	d->sadxEditWidgets.append(d->ui.saGeneral);
	d->sadxEditWidgets.append(d->ui.saLevelStats);
	d->sadxEditWidgets.append(d->ui.saSubGames);

	// SAEventFlags model and widget.
	d->saEventFlagsModel = new BitFlagsModel(this);
	d->saEventFlagsModel->setBitFlags(&d->saEventFlags);
	d->ui.saEventFlagsView->setBitFlagsModel(d->saEventFlagsModel);

	// SANPCFlags model and widget.
	d->saNPCFlagsModel = new BitFlagsModel(this);
	d->saNPCFlagsModel->setBitFlags(&d->saNPCFlags);
	d->ui.saNPCFlagsView->setBitFlagsModel(d->saNPCFlagsModel);

	// SADXMissionFlags model and widget.
	d->sadxMissionFlagsModel = new ByteFlagsModel(this);
	d->sadxMissionFlagsModel->setByteFlags(&d->sadxMissionFlags);
	d->ui.sadxMissionFlagsView->setByteFlagsModel(d->sadxMissionFlagsModel);

	// Connect the widgetHasBeenModified() signals.
	foreach (SAEditWidget *saEditWidget, d->saEditWidgets) {
		connect(saEditWidget, SIGNAL(hasBeenModified(bool)),
			this, SLOT(widgetHasBeenModified(bool)));
	}
	foreach (SADXEditWidget *sadxEditWidget, d->sadxEditWidgets) {
		connect(sadxEditWidget, SIGNAL(hasBeenModified(bool)),
			this, SLOT(widgetHasBeenModified(bool)));
	}
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
	super::changeEvent(event);
}

/** Public static functions. **/

/**
 * Is the specified file supported by this editor?
 * @return True if supported; false if not.
 */
bool SAEditor::isFileSupported(const File *file)
{
	bool ret = false;
	const QString filename = file->filename();
	// TODO: "PC" file for SADX PC?
	if (qobject_cast<const GcnFile*>(file) != nullptr) {
		// GameCube file. (SADX)
		if (file->gameID().left(3) == QLatin1String("GXS") &&
		    filename.startsWith(QLatin1String("SONICADVENTURE_DX_PLAYRECORD_"))) {
			// Game ID and filename are both correct.
			// Verify the length.
			// FIXME: Add a file->sizeInBytes() function.
			/*if (file->sizeInBytes() >= (SA_SAVE_ADDRESS_GCN + SA_SAVE_SLOT_LEN))*/ {
				// File is valid.
				ret = true;
			}
		}
	} else if (qobject_cast<const VmuFile*>(file) != nullptr) {
		// Dreamcast file. (SA1)
		if (filename == QLatin1String("SONICADV_SYS") ||
		    filename == QLatin1String("SONICADV_INT"))
		{
			// Filename is correct.
			// Verify the length.
			// FIXME: Add a file->sizeInBytes() function.
			/*if (file->sizeInBytes() >= (SA_SAVE_ADDRESS_DC_0 + (SA_SAVE_SLOT_LEN * 3)))*/ {
				// File is valid.
				ret = true;
			}
		}
	}

	return ret;
}

/** Public functions. **/

/**
 * Set the File to edit.
 * This function MUST be overridden by subclasses.
 *
 * @param file File to edit.
 * If the file isn't valid, it won't be set;
 * check file() afterwards to verify.
 *
 * @return 0 on success; non-zero on error (and file will not be set).
 * TODO: Error code constants?
 */
int SAEditor::setFile(File *file)
{
	Q_D(SAEditor);

	// Make sure the file is supported before doing anything else.
	// This also checks that the file length is correct.
	if (!isFileSupported(file)) {
		// Not supported.
		return -1;
	}

	// TODO: If a file is already loaded, prompt to
	// save it if it's been modified?

	// Load data from the specified file.
	return d->load(file);

	// TODO: Set as not modified?
	//this->setModified(false);
}

/** Public slots. **/

/**
 * Set the current save slot. [INTERNAL FUNCTION]
 *
 * This is called by the base class when the
 * setCurrentSaveSlot() function is called.
 * Subclasses should load the appropriate data
 * from the specified save slot.
 *
 * If an error occurs, the save slot will not be changed.
 *
 * @return 0 on success; non-zero on error.
 */
int SAEditor::setCurrentSaveSlot_int(int saveSlot)
{
	Q_D(SAEditor);
	assert(saveSlot >= 0 && saveSlot < d->saveSlots);
	if (d->currentSaveSlot == saveSlot)
		return -EINVAL;

	d->saveCurrentSlot();
	// TODO: Make saveSlot a parameter of updateDisplay()?
	// For now, we're updating d->currentSaveSlot directly.
	d->currentSaveSlot = saveSlot;
	d->updateDisplay();
	return 0;
}

/**
 * Save the data to the file.
 * @return 0 on success; negative POSIX error code on error.
 */
int SAEditor::save(void)
{
	Q_D(SAEditor);
	if (!d->file)
		return -EBADF;
	else if (d->file->isReadOnly())
		return -EROFS;

	// Make sure the current slot is saved.
	d->saveCurrentSlot();

	// Read the existing file data first.
	QByteArray data = d->file->loadFileData();

	// Determine which version of the game this save file is for.
	// TODO: Test for GCN first, then DC?
	int ret = -EINVAL;
	if (qobject_cast<VmuFile*>(d->file) != nullptr) {
		// DC version.

		// Three, count 'em, *three* save slots!
		char *dest = (data.data() + SA_SAVE_ADDRESS_DC_0);
		for (int i = 0; i < 3; i++, dest += SA_SAVE_SLOT_LEN) {
			sa_save_slot *sa_save = (sa_save_slot*)dest;
			assert(d->data_main.size() > i);
			if (d->data_main.size() > i) {
				// Save the slot.
				memcpy(sa_save, d->data_main.at(i), SA_SAVE_SLOT_LEN);
#if SYS_BYTEORDER == SYS_BIG_ENDIAN
				// Byteswap the data.
				// Dreamcast's SH-4 is little-endian.
				d->byteswap_sa_save_slot(sa_save);
#endif /* SYS_BYTEORDER == SYS_BIG_ENDIAN */
			} else {
				// No save slot available...
				// Zero out the data.
				memset(sa_save, 0, SA_SAVE_SLOT_LEN);
			}

			// Update the checksums.
			// TODO: Not tested!
			// Note that there are two sets of checksums:
			// - Game checksum (CRC-16) [one per slot]
			// - VMS checksum (custom)
			uint8_t *src = (uint8_t*)data.data() + SA_SAVE_ADDRESS_DC_0;
			for (int i = 0; i < 3; i++, src += SA_SAVE_SLOT_LEN) {
				uint16_t crc16 = Checksum::Crc16(src + 4, SA_SAVE_SLOT_LEN - 4);
				crc16 = cpu_to_le16(crc16);
				memcpy(src + 2, &crc16, sizeof(crc16));
			}

			// VMS checksum.
			uint16_t vmschk = Checksum::DreamcastVMU(src, data.size(), 0x46);
			vmschk = cpu_to_le16(vmschk);
			memcpy(&src[0x46], &vmschk, sizeof(vmschk));

			// Save slots copied.
			// Now it needs to be written to the file.
		}
	} else if (qobject_cast<GcnFile*>(d->file) != nullptr) {
		// GameCube verison.

		// Only one save slot.
		sa_save_slot *sa_save = (sa_save_slot*)(data.data() + SA_SAVE_ADDRESS_GCN);
		assert(d->data_main.size() > 0);
		if (d->data_main.size() > 0) {
			memcpy(sa_save, d->data_main.at(0), SA_SAVE_SLOT_LEN);
#if SYS_BYTEORDER == SYS_LIL_ENDIAN
			// Byteswap the data.
			// GameCube's PowerPC 750 is big-endian.
			d->byteswap_sa_save_slot(sa_save);
#endif /* SYS_BYTEORDER == SYS_LIL_ENDIAN */
		} else {
			// No save slot available...
			// Zero out the data.
			memset(sa_save, 0, SA_SAVE_SLOT_LEN);
		}

		// Check for SADX extras.
		sadx_extra_save_slot *sadx_extra_save = (sadx_extra_save_slot*)(data.data() + SA_SAVE_ADDRESS_GCN + SA_SAVE_SLOT_LEN);
		if (d->data_sadx.size() > 0) {
			// Found SADX extras.
			memcpy(sadx_extra_save, d->data_sadx.at(0), SADX_EXTRA_SAVE_SLOT_LEN);
#if SYS_BYTEORDER == SYS_LIL_ENDIAN
			// Byteswap the data.
			// GameCube's PowerPC 750 is big-endian.
			d->byteswap_sadx_extra_save_slot(sadx_extra_save);
#endif /* SYS_BYTEORDER == SYS_LIL_ENDIAN */
		} else {
			// No SADX extras.
			// Zero out the data.
			memset(sadx_extra_save, 0, SADX_EXTRA_SAVE_SLOT_LEN);
		}

		// Update the checksum.
		uint16_t crc16 = Checksum::Crc16((const uint8_t*)data.data() + SA_SAVE_ADDRESS_GCN + 4,
			SA_SAVE_SLOT_LEN + SADX_EXTRA_SAVE_SLOT_LEN - 4);
		crc16 = cpu_to_be16(crc16);
		memcpy(data.data() + 0x1442, &crc16, sizeof(crc16));

		// Save slots copied.
		// Now it needs to be written to the file.
		ret = 0;
	} else {
		// Unsupported file.
		// TODO: Add support for the Windows version.
		ret = -ENOSYS;
		goto end;
	}

	// Write the data.
	ret = d->file->write(0, data.data(), data.size());

end:
	return ret;
}

/**
 * Reload the save data.
 */
void SAEditor::reload(void)
{
	// TODO: If modified, warn?
	Q_D(SAEditor);
	d->load(d->file);
	this->setModified(false);
}

/**
 * Widget's modified state has been changed.
 * @param modified New modified status.
 */
void SAEditor::widgetHasBeenModified(bool modified)
{
	// This function only *sets* the editor modified state
	// if a widget has been modified. Clearing the state
	// requires a save and/or reload.
	if (modified) {
		this->setModified(true);
	}
}
