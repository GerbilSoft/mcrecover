/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * SAGeneral.hpp: Sonic Adventure - General settings editor.               *
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

#include "SAGeneral.hpp"

// C includes. (C++ namespace)
#include <cstdlib>

// Sonic Adventure save file definitions.
#include "sa_defs.h"

// TODO: Put this in a common header file somewhere.
#define NUM_ELEMENTS(x) ((int)(sizeof(x) / sizeof(x[0])))

/** SAGeneralPrivate **/

#include "ui_SAGeneral.h"
class SAGeneralPrivate
{
	public:
		SAGeneralPrivate(SAGeneral *q);

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
	: QWidget(parent)
	, d_ptr(new SAGeneralPrivate(this))
{
	Q_D(SAGeneral);
	d->ui.setupUi(this);
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
		d->ui.retranslateUi(this);
	}

	// Pass the event to the base class.
	this->QWidget::changeEvent(event);
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

	// Play time.
	// Stored in NTSC frames. (1/60th of a second)
	// TODO: Verify for PAL?
	// TODO: Convert frames to 1/100th of seconds.
	// FIXME: Find an unsigned equivalent to div().
	uint32_t playTime = sa_save->playTime;
	d->ui.spnPlayTimeFrames->setValue(playTime % 60);
	playTime /= 60;
	d->ui.spnPlayTimeSeconds->setValue(playTime % 60);
	playTime /= 60;
	d->ui.spnPlayTimeMinutes->setValue(playTime % 60);
	playTime /= 60;
	// FIXME: Value may be too large?
	d->ui.spnPlayTimeHours->setValue(playTime);

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

	return 0;
}

/**
 * Save data to a Sonic Adventure save slot.
 * @param sa_save Sonic Adventure save slot.
 * The data will be in host-endian format.
 * @return 0 on success; non-zero on error.
 */
int SAGeneral::save(sa_save_slot *sa_save) const
{
	Q_D(const SAGeneral);

	// Play time.
	// Stored in NTSC frames. (1/60th of a second)
	// TODO: Verify for PAL?
	// TODO: Convert frames from 1/100th of seconds.
	uint32_t playTime =
		((uint32_t)d->ui.spnPlayTimeHours->value() * 60 * 60 * 60) +
		(d->ui.spnPlayTimeMinutes->value() * 60 * 60) +
		(d->ui.spnPlayTimeSeconds->value() * 60) +
		(d->ui.spnPlayTimeSeconds->value());

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

	return 0;
}

/**
 * Clear the loaded data.
 */
void SAGeneral::clear(void)
{
	Q_D(SAGeneral);
	// TODO
}
