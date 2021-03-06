/***************************************************************************
 * GameCube Memory Card Recovery Program [libsaveedit]                     *
 * SAGeneral.hpp: Sonic Adventure - General settings editor.               *
 *                                                                         *
 * Copyright (c) 2015-2018 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#include "SAGeneral.hpp"

// C includes. (C++ namespace)
#include <cstdlib>

// Sonic Adventure save file definitions.
#include "sa_defs.h"

// Common data.
#include "SAData.h"

// TODO: Put this in a common header file somewhere.
#define NUM_ELEMENTS(x) ((int)(sizeof(x) / sizeof(x[0])))

/** SAGeneralPrivate **/

#include "ui_SAGeneral.h"
class SAGeneralPrivate
{
	public:
		explicit SAGeneralPrivate(SAGeneral *q);

	protected:
		SAGeneral *const q_ptr;
		Q_DECLARE_PUBLIC(SAGeneral)
	private:
		Q_DISABLE_COPY(SAGeneralPrivate)

	public:
		Ui_SAGeneral ui;
};

SAGeneralPrivate::SAGeneralPrivate(SAGeneral *q)
	: q_ptr(q)
{ }

/** SAGeneral **/

SAGeneral::SAGeneral(QWidget *parent)
	: super(parent)
	, d_ptr(new SAGeneralPrivate(this))
{
	Q_D(SAGeneral);

	// Prevent modification signals from being emitted
	// during initialization.
	suspendHasBeenModified();
	d->ui.setupUi(this);

	// Show hours in the TimeCodeEdit.
	d->ui.tcePlayTime->setDisplayMode(TimeCodeEdit::DM_HMSF);
	// Signals for custom widgets aren't available in Qt Designer,
	// so connect them here.
	connect(d->ui.tcePlayTime, SIGNAL(valueChanged(int,int,int)),
		this, SLOT(widgetModifiedSlot()));
	connect(d->ui.tcePlayTime, SIGNAL(valueChangedHours(int)),
		this, SLOT(widgetModifiedSlot()));

	// Level names.
	// Does NOT include Chao Gardens or Chao Race. (last 4 entries)
	for (int i = 0; i < SA_LEVEL_NAMES_ALL_COUNT-4; i++) {
		d->ui.cboLastLevel->addItem(QLatin1String(sa_level_names_all[i]));
	}

	// Hide the "Black Market Rings" widgets for now.
	// It'll be shown again if an SADX save is loaded.
	d->ui.lblBlackMarketRings->hide();
	d->ui.spnBlackMarketRings->hide();

	// Finished initialization.
	unsuspendHasBeenModified();
}

SAGeneral::~SAGeneral()
{
	Q_D(SAGeneral);
	delete d;
}

/** Events. **/

/**
 * Widget state has changed.
 * @param event State change event.
 */
void SAGeneral::changeEvent(QEvent *event)
{
	if (event->type() == QEvent::LanguageChange) {
		// Retranslate the UI.
		Q_D(SAGeneral);
		// Prevent modification signals from being emitted
		// during retranslation.
		// FIXME: uic clears cboMessages and reinserts items,
		// which causes it to lose the currentIndex.
		suspendHasBeenModified();
		const int cboMessages_currentIndex = d->ui.cboMessages->currentIndex();
		d->ui.retranslateUi(this);
		d->ui.cboMessages->setCurrentIndex(cboMessages_currentIndex);
		unsuspendHasBeenModified();
	}

	// Pass the event to the base class.
	super::changeEvent(event);
}

/** Public functions. **/

/**
 * Load data from a Sonic Adventure save slot.
 * @param sa_save Sonic Adventure save slot.
 * The data must have already been byteswapped to host-endian.
 * @return 0 on success; non-zero on error.
 */
int SAGeneral::load(const sa_save_slot *sa_save)
{
	Q_D(SAGeneral);
	suspendHasBeenModified();

	// Play time.
	// Stored in NTSC frames. (1/60th of a second)
	// TODO: Verify for PAL?
	d->ui.tcePlayTime->setValueInNtscFrames(sa_save->playTime);

	// Options byte.
	d->ui.cboMessages->setCurrentIndex(SA_OPTIONS_MSG_VALUE(sa_save->options));
	d->ui.cboVoiceLanguage->setCurrentIndex(SA_OPTIONS_VOICE_LANG_VALUE(sa_save->options));
	d->ui.cboTextLanguage->setCurrentIndex(SA_OPTIONS_TEXT_LANG_VALUE(sa_save->options));

	// Rumble.
	// FIXME: Just the first bit, or check the whole byte?
	d->ui.chkRumble->setChecked(SA_RUMBLE_FEATURE_VALUE(sa_save->rumble));

	// Last character and level.
	d->ui.cboLastCharacter->setCurrentIndex(sa_save->last_char);
	// TODO: Verify this...
	int last_level = sa_save->last_level;
	if (last_level >= d->ui.cboLastLevel->count())
		last_level = 0;
	d->ui.cboLastLevel->setCurrentIndex(last_level);

	unsuspendHasBeenModified();
	setModified(false);
	return 0;
}

/**
 * Save data to a Sonic Adventure save slot.
 * @param sa_save Sonic Adventure save slot.
 * The data will be in host-endian format.
 * @return 0 on success; non-zero on error.
 */
int SAGeneral::save(sa_save_slot *sa_save)
{
	Q_D(const SAGeneral);

	// Play time.
	// Stored in NTSC frames. (1/60th of a second)
	// TODO: Verify for PAL?
	sa_save->playTime = d->ui.tcePlayTime->valueInNtscFrames();

	// Options byte.
	// TODO: Bit-shifting macros like SA_OPTIONS_*_VALUE()?
	uint8_t options = 0;
	if (d->ui.cboMessages->currentIndex() >= 0)
		options |= (d->ui.cboMessages->currentIndex() << 1);
	if (d->ui.cboVoiceLanguage->currentIndex() >= 0)
		options |= (d->ui.cboVoiceLanguage->currentIndex() << 2);
	if (d->ui.cboTextLanguage->currentIndex() >= 0)
		options |= (d->ui.cboTextLanguage->currentIndex() << 4);
	sa_save->options = options;

	// Last character and level.
	if (d->ui.cboLastCharacter->currentIndex() >= 0)
		sa_save->last_char = d->ui.cboLastCharacter->currentIndex();
	if (d->ui.cboLastLevel->currentIndex() >= 0)
		sa_save->last_level = d->ui.cboLastLevel->currentIndex();

	setModified(false);
	return 0;
}

/**
 * Load data from a Sonic Adventure DX extra save slot.
 * @param sadx_extra_save Sonic Adventure DX extra save slot.
 * The data must have already been byteswapped to host-endian.
 * If nullptr, SADX editor components will be hidden.
 * @return 0 on success; non-zero on error.
 */
int SAGeneral::loadDX(const sadx_extra_save_slot *sadx_extra_save)
{
	Q_D(SAGeneral);
	suspendHasBeenModified();

	if (sadx_extra_save) {
		// The only SADX information here is the "Black Market Rings".
		// TODO: Validate the value?
		d->ui.spnBlackMarketRings->setValue(sadx_extra_save->rings_black_market);

		// Make sure the "Black Market Rings" widgets are visible.
		d->ui.lblBlackMarketRings->show();
		d->ui.spnBlackMarketRings->show();
	} else {
		// Save file is from SA1 and doesn't have SADX extras.
		// Hide the "Black Market Rings" widgets.
		d->ui.lblBlackMarketRings->hide();
		d->ui.spnBlackMarketRings->hide();
	}

	unsuspendHasBeenModified();
	setModified(false);
	return 0;
}

/**
 * Save data to a Sonic Adventure DX extra save slot.
 * @param sadx_extra_save Sonic Adventure DX extra save slot.
 * The data will be in host-endian format.
 * @return 0 on success; non-zero on error.
 */
int SAGeneral::saveDX(sadx_extra_save_slot *sadx_extra_save)
{
	Q_D(const SAGeneral);

	// The only SADX information here is the "Black Market Rings".
	// TODO: Validate the value?
	sadx_extra_save->rings_black_market = d->ui.spnBlackMarketRings->value();

	setModified(false);
	return 0;
}

/**
 * Clear the loaded data.
 */
void SAGeneral::clear(void)
{
	Q_D(SAGeneral);
	// TODO
	setModified(false);
}
