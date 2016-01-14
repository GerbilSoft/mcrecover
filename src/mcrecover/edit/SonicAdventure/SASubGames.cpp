/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * SASubGames.hpp: Sonic Adventure - Sub Games editor.                     *
 *                                                                         *
 * Copyright (c) 2015-2016 by David Korth.                                 *
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

#include "SASubGames.hpp"

// C includes. (C++ namespace)
#include <cstdlib>

// Sonic Adventure save file definitions.
#include "sa_defs.h"

// Common data.
#include "SAData.h"

// TODO: Put this in a common header file somewhere.
#define NUM_ELEMENTS(x) ((int)(sizeof(x) / sizeof(x[0])))

/** SASubGamesPrivate **/

#include "ui_SASubGames.h"
class SASubGamesPrivate
{
	public:
		SASubGamesPrivate(SASubGames *q);

	protected:
		SASubGames *const q_ptr;
		Q_DECLARE_PUBLIC(SASubGames)
	private:
		Q_DISABLE_COPY(SASubGamesPrivate)

	public:
		Ui_SASubGames ui;

		// Current character being displayed.
		// TODO: Enum?
		int character;

		// FIXME: Mini-Game scores for Sky Chase and Sand Hill
		// are NOT shared between characters. (Not sure about Ice Cap.)
		// Hedgehog Hammer is shared, since only Amy has access.

		// TODO: Emblems.

		// Twinkle Circuit times are unique per character.
		sa_twinkle_circuit_times twinkle_circuit;

		// Boss Attack times are unique per character.
		sa_boss_attack_times boss_attack;

		/**
		 * Switch to another character.
		 * This function automatically calls updateDisplay().
		 * @param character Character ID.
		 */
		void switchCharacter(int character);

		/**
		 * Update the widgets for the selected character.
		 */
		void updateDisplay(void);

		/**
		 * Save the current character's stats.
		 */
		void saveCurrentStats(void);

		/** Static read-only data. **/

		/**
		 * Sub Game mapping. (8-bit bitfield)
		 * A '1' indicates that this character has access
		 * to the Sub Game; a '0' indicates that this character
		 * does not have access to the Sub Game.
		 * access to the subgame.
		 *
		 * Sub Game index order:
		 * - 0: Sky Chase
		 * - 1: Ice Cap
		 * - 2: Sand Hill
		 * - 3: Hedgehog Hammer
		 * - 4: Twinkle Circuit
		 * - 5: Boss Attack
		 */
		static const uint8_t subGameMap[7];
		// TODO: static const uint8_t instead of #define?
		#define SUB_GAME_SKY_CHASE		(1 << 0)
		#define SUB_GAME_ICE_CAP		(1 << 1)
		#define SUB_GAME_SAND_HILL		(1 << 2)
		#define SUB_GAME_HEDGEHOG_HAMMER	(1 << 3)
		#define SUB_GAME_TWINKLE_CIRCUIT	(1 << 4)
		#define SUB_GAME_BOSS_ATTACK		(1 << 5)
};

/**
 * Sub Game mapping. (8-bit bitfield)
 * A '1' indicates that this character has access
 * to the Sub Game; a '0' indicates that this character
 * does not have access to the Sub Game.
 * access to the subgame.
 *
 * Sub Game bitfield order:
 * - 0: Sky Chase
 * - 1: Ice Cap
 * - 2: Sand Hill
 * - 3: Hedgehog Hammer
 * - 4: Twinkle Circuit
 * - 5: Boss Attack
 */
const uint8_t SASubGamesPrivate::subGameMap[7] = {
	0x37,	// Sonic (all except Hedgehog Hammer)
	0x37,	// Tails (all except Hedgehog Hammer)
	0x30,	// Knuckles (Twinkle Circuit, Boss Attack)
	0x38,	// Amy (Hedgehog Hammer, Twinkle Circuit, Boss Attack)
	0x30,	// Gamma (Twinkle Circuit, Boss Attack)
	0x30,	// Big (Twinkle Circuit, Boss Attack)
	0x36,	// Metal Sonic (Ice Cap, Sand Hill, Twinkle Circuit, Boss Attack)
};

SASubGamesPrivate::SASubGamesPrivate(SASubGames *q)
	: q_ptr(q)
	, character(0)
{ }

/**
 * Switch levels to another character.
 * This function automatically calls updateDisplay().
 * @param character Character ID.
 */
void SASubGamesPrivate::switchCharacter(int character)
{
	// FIXME: Character enums or something.
	if (character < 0 || character > 6)
		return;

	// Save the current character's stats.
	saveCurrentStats();

	// Save the new character index.
	this->character = character;

	// Get the valid Sub Game settings for the selected character.
	const uint8_t subGames = subGameMap[character];

	// Determine which widgets to show or hide.
	ui.grpSkyChase->setVisible(!!(subGames & SUB_GAME_SKY_CHASE));
	ui.grpIceCap->setVisible(!!(subGames & SUB_GAME_ICE_CAP));
	ui.grpSandHill->setVisible(!!(subGames & SUB_GAME_SAND_HILL));
	ui.grpHedgehogHammer->setVisible(!!(subGames & SUB_GAME_HEDGEHOG_HAMMER));
	ui.grpTwinkleCircuit->setVisible(!!(subGames & SUB_GAME_TWINKLE_CIRCUIT));
	ui.grpBossAttack->setVisible(!!(subGames & SUB_GAME_BOSS_ATTACK));

	// Update the display.
	updateDisplay();
}

/**
 * Update the widgets for the selected character.
 */
void SASubGamesPrivate::updateDisplay(void)
{
	// TODO
#if 0
	// TODO: Make character a parameter, or not?
	// FIXME: Character enums or something.
	if (character >= 0 && character <= 5) {
		// Standard SA1 character.
		for (int level = 0; level < MAX_LEVELS; level++) {
			const int8_t saveIdx = saveMap[character][level];
			if (saveIdx < 0 || saveIdx >= NUM_ELEMENTS(scores.all))
				break;

			// Score
			levels[level].spnHighScore->setValue(scores.all[saveIdx]);

			// Time / Weight
			if (character != 5) {
				// Time (not Big)
				levels[level].tceBestTime->setValue(&times.all[saveIdx]);
			} else {
				// Weight (Big)
				const int8_t bigSaveIdx = (saveIdx - 28);
				levels[level].tceBestTime->setValue(weights.levels[bigSaveIdx]);
			}

			// Rings
			levels[level].spnMostRings->setValue(rings.all[saveIdx]);

			/**
			 * Level emblems:
			 * - A: saveIdx + 0
			 * - B: saveIdx + 32
			 * - C: saveIdx + 64
			 */
			levels[level].chkEmblems[0]->setChecked(emblems[saveIdx+0]);
			levels[level].chkEmblems[1]->setChecked(emblems[saveIdx+32]);
			levels[level].chkEmblems[2]->setChecked(emblems[saveIdx+64]);
		}
	} else if (character == 6) {
		// SADX: Metal Sonic.
		// Uses the same level map as Sonic.
		for (int level = 0; level < MAX_LEVELS; level++) {
			const int8_t saveIdx = saveMap[0][level];

			// Score
			levels[level].spnHighScore->setValue(metal_sonic.scores[saveIdx]);
			// Time
			levels[level].tceBestTime->setValue(&metal_sonic.times[saveIdx]);
			// Rings
			levels[level].spnMostRings->setValue(metal_sonic.rings[saveIdx]);

			/**
			 * Level emblems:
			 * - A: (saveIdx * 3) + 0
			 * - B: (saveIdx * 3) + 1
			 * - C: (saveIdx * 3) + 2
			 */
			levels[level].chkEmblems[0]->setChecked(metal_sonic.emblems[(saveIdx*3)+0]);
			levels[level].chkEmblems[1]->setChecked(metal_sonic.emblems[(saveIdx*3)+1]);
			levels[level].chkEmblems[2]->setChecked(metal_sonic.emblems[(saveIdx*3)+2]);
		}
	}
#endif
}

/**
 * Save the current character's stats.
 */
void SASubGamesPrivate::saveCurrentStats(void)
{
	// TODO
#if 0
	// TODO: Make character a parameter, or not?
	// FIXME: Character enums or something.
	if (character >= 0 && character <= 5) {
		// Standard SA1 character.
		for (int level = 0; level < MAX_LEVELS; level++) {
			const int8_t saveIdx = saveMap[character][level];
			if (saveIdx < 0 || saveIdx >= NUM_ELEMENTS(scores.all))
				break;

			// Score
			scores.all[saveIdx] = levels[level].spnHighScore->value();

			// Time / Weight
			if (character != 5) {
				// TODO: Constrain bounds?
				// Time (not Big)
				levels[level].tceBestTime->value(&times.all[saveIdx]);
			} else {
				// Weight (Big)
				const int8_t bigSaveIdx = (saveIdx - 28);
				levels[level].tceBestTime->value(weights.levels[bigSaveIdx]);
			}

			// Rings
			rings.all[saveIdx] = levels[level].spnMostRings->value();

			/**
			* Level emblems:
			* - A: saveIdx + 0
			* - B: saveIdx + 32
			* - C: saveIdx + 64
			*/
			emblems[saveIdx+0 ] = levels[level].chkEmblems[0]->isChecked();
			emblems[saveIdx+32] = levels[level].chkEmblems[1]->isChecked();
			emblems[saveIdx+64] = levels[level].chkEmblems[2]->isChecked();
		}
	} else if (character == 6) {
		// SADX: Metal Sonic.
		// Uses the same level map as Sonic.
		for (int level = 0; level < MAX_LEVELS; level++) {
			const int8_t saveIdx = saveMap[0][level];

			// Score
			metal_sonic.scores[saveIdx] = levels[level].spnHighScore->value();
			// Time
			levels[level].tceBestTime->value(&metal_sonic.times[saveIdx]);
			// Rings
			metal_sonic.rings[saveIdx] = levels[level].spnMostRings->value();

			/**
			 * Level emblems:
			 * - A: (saveIdx * 3) + 0
			 * - B: (saveIdx * 3) + 1
			 * - C: (saveIdx * 3) + 2
			 */
			metal_sonic.emblems[(saveIdx*3)+0] = levels[level].chkEmblems[0]->isChecked();
			metal_sonic.emblems[(saveIdx*3)+1] = levels[level].chkEmblems[1]->isChecked();
			metal_sonic.emblems[(saveIdx*3)+2] = levels[level].chkEmblems[2]->isChecked();
		}
	}
#endif
}

/** SASubGames **/

SASubGames::SASubGames(QWidget *parent)
	: QWidget(parent)
	, d_ptr(new SASubGamesPrivate(this))
{
	Q_D(SASubGames);
	d->ui.setupUi(this);

	// TODO: Add scroll area like SALevelStats.

	// Set up the emblem checkboxes.
	QString qsCssCheckBox = QLatin1String(sa_ui_css_emblem_checkbox);
	d->ui.chkSkyChaseAct1_1->setStyleSheet(qsCssCheckBox);
	d->ui.chkSkyChaseAct1_2->setStyleSheet(qsCssCheckBox);
	d->ui.chkSkyChaseAct2_1->setStyleSheet(qsCssCheckBox);
	d->ui.chkSkyChaseAct2_2->setStyleSheet(qsCssCheckBox);
	d->ui.chkSandHill_1->setStyleSheet(qsCssCheckBox);
	d->ui.chkSandHill_2->setStyleSheet(qsCssCheckBox);
	d->ui.chkHedgehogHammer_1->setStyleSheet(qsCssCheckBox);
	d->ui.chkHedgehogHammer_2->setStyleSheet(qsCssCheckBox);
	d->ui.chkTwinkleCircuit_1->setStyleSheet(qsCssCheckBox);
	d->ui.chkTwinkleCircuit_2->setStyleSheet(qsCssCheckBox);

	// Center-align the Twinkle Circuit emblem checkboxes.
	// TODO: More alignment.
	// TODO: Some of the Time boxes are so wide they no longer look like times.
	// (Added horizontal spacers to fix some of them...)
	d->ui.hboxTwinkleCircuitEmblems->setAlignment(d->ui.chkTwinkleCircuit_1, Qt::AlignHCenter);
	d->ui.hboxTwinkleCircuitEmblems->setAlignment(d->ui.chkTwinkleCircuit_2, Qt::AlignHCenter);

	// TODO: Add tab stops to TimeCodeEdit widgets?
	// TODO: Additional setup.
	// TODO: Clicking the "Edit" button has a lot of lag if
	// attempting to load the SADX Chao Garden. (but not SA2 files)

	// Initialize the widgets.
	d->switchCharacter(d->ui.cboCharacter->currentIndex());
}

SASubGames::~SASubGames()
{
	Q_D(SASubGames);
	delete d;
}

/** Events. **/

/**
 * Widget state has changed.
 * @param event State change event.
 */
void SASubGames::changeEvent(QEvent *event)
{
	if (event->type() == QEvent::LanguageChange) {
		// Retranslate the UI.
		Q_D(SASubGames);
		d->ui.retranslateUi(this);

		// Manual retranslation needed for "Metal Sonic".
		if (d->ui.cboCharacter->count() >= 7) {
			d->ui.cboCharacter->setItemText(6, tr("Metal Sonic"));
		}
	}

	// Pass the event to the base class.
	this->QWidget::changeEvent(event);
}

/** UI widget slots. **/

/**
 * The selected character was changed.
 * @param index New character ID.
 */
void SASubGames::on_cboCharacter_currentIndexChanged(int index)
{
	// Update the widgets for the newly-selected character.
	Q_D(SASubGames);
	d->switchCharacter(index);
}

/** Public functions. **/

/**
 * Load data from a Sonic Adventure save slot.
 * @param sa_save Sonic Adventure save slot.
 * The data must have already been byteswapped to host-endian.
 * @return 0 on success; non-zero on error.
 */
int SASubGames::load(const sa_save_slot *sa_save)
{
	Q_D(SASubGames);
	memcpy(&d->twinkle_circuit, &sa_save->twinkle_circuit, sizeof(d->twinkle_circuit));
	memcpy(&d->boss_attack,     &sa_save->boss_attack,     sizeof(d->boss_attack));

	// TODO
	return 0;
}

/**
 * Save data to a Sonic Adventure save slot.
 * @param sa_save Sonic Adventure save slot.
 * The data will be in host-endian format.
 * @return 0 on success; non-zero on error.
 */
int SASubGames::save(sa_save_slot *sa_save) const
{
	Q_D(const SASubGames);
	// TODO: Save the current character data.

	memcpy(&sa_save->twinkle_circuit, &d->twinkle_circuit, sizeof(sa_save->twinkle_circuit));
	memcpy(&sa_save->boss_attack,     &d->boss_attack,     sizeof(sa_save->boss_attack));

	// TODO
	return 0;
}

/**
 * Load data from a Sonic Adventure DX extra save slot.
 * @param sadx_extra_save Sonic Adventure DX extra save slot.
 * The data will be in host-endian format.
 * @return 0 on success; non-zero on error.
 */
int SASubGames::loadDX(const sadx_extra_save_slot *sadx_extra_save)
{
	Q_D(SASubGames);

	// If the Characters dropdown doesn't have Metal Sonic, add him now.
	// TODO: Remove Metal Sonic on clear()?
	if (d->ui.cboCharacter->count() < 7) {
		QIcon icon(QLatin1String(":/sonic/SA1/metal_sonic.png"));
		d->ui.cboCharacter->addItem(icon, tr("Metal Sonic"));
	}

	// Update the display.
	d->updateDisplay();
	return 0;
}

/**
 * Save data to a Sonic Adventure DX extra save slot.
 * @param sadx_extra_save Sonic Adventure DX extra save slot.
 * The data will be in host-endian format.
 * @return 0 on success; non-zero on error.
 */
int SASubGames::saveDX(sadx_extra_save_slot *sadx_extra_save)
{
	Q_D(const SASubGames);

	// TODO
	return 0;
}

/**
 * Clear the loaded data.
 */
void SASubGames::clear(void)
{
	Q_D(SASubGames);
	// TODO
}
