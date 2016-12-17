/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * SALevelStats.cpp: Sonic Adventure - Level Stats editor.                 *
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

#include "SALevelStats.hpp"

// Qt widgets.
#include <QHBoxLayout>
#include <QLabel>
#include <QSpinBox>
#include <QCheckBox>

// TimeCodeEdit widget.
#include "TimeCodeEdit.hpp"

// Sonic Adventure save file definitions.
#include "sa_defs.h"

// Common data.
#include "SAData.h"

// TODO: Put this in a common header file somewhere.
#define NUM_ELEMENTS(x) ((int)(sizeof(x) / sizeof(x[0])))

/** SALevelStatsPrivate **/

#include "ui_SALevelStats.h"
class SALevelStatsPrivate
{
	public:
		explicit SALevelStatsPrivate(SALevelStats *q);
		~SALevelStatsPrivate();

	protected:
		SALevelStats *const q_ptr;
		Q_DECLARE_PUBLIC(SALevelStats)
	private:
		Q_DISABLE_COPY(SALevelStatsPrivate)

	public:
		Ui_SALevelStats ui;

		// Up to 10 levels can be onscreen at once.
		#define MAX_LEVELS 10
		// Level widgets.
		struct {
			QLabel *lblLevel;
			QSpinBox *spnHighScore;
			QCheckBox *chkEmblems[3];
			QHBoxLayout *hboxEmblems;
			TimeCodeEdit *tceBestTime;
			QSpinBox *spnMostRings;
		} levels[MAX_LEVELS];

		// Save file data.
		sa_scores scores;
		sa_times times;
		sa_weights weights;
		sa_rings rings;

		// Save file data. (Metal Sonic)
		struct {
			uint32_t scores[10];
			sa_time_code times[10];
			uint16_t rings[10];
			bool emblems[30];
		} metal_sonic;

		// Emblems.
		// NOTE: There's only 130 emblems, but there's enough
		// space for 136, so we have to allocate the entire area.
		bool emblems[136];

		// Current character being displayed.
		int character;

		// Cached level names.
		QString actionStageNameCache[SA_LEVEL_NAMES_ACTION_COUNT];

		/**
		 * Clear the loaded data.
		 * This does NOT automatically update the UI.
		 */
		void clear(void);

		/**
		 * Initialize level widgets.
		 */
		void initLevels(void);

		/**
		 * Switch levels to another character.
		 * This function automatically calls updateDisplay().
		 * @param character Character ID.
		 */
		void switchLevels(int character);

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
		 * Character level mapping.
		 * Corresponds to sa_level_names_action[] entries.
		 * -1 == end of list
		 *
		 * NOTE: Metal Sonic is not included here.
		 */
		static const int8_t levelMap[6][MAX_LEVELS];

		/**
		 * Save file level mapping.
		 * Index == display level ID
		 * Value == save index (e.g. scores.all[x])
		 * A value of -1 indicates an invalid entry.
		 *
		 * NOTE: Metal Sonic is not included here.
		 */
		static const int8_t saveMap[6][MAX_LEVELS];
};

/**
 * Character level mapping.
 * Corresponds to sa_level_names_action[] entries.
 * -1 == end of list
 */
const int8_t SALevelStatsPrivate::levelMap[6][MAX_LEVELS] = {
	// Sonic
	{0, 1, 2, 3, 4, 5, 6, 7, 8, 9},
	// Tails
	{1, 2, 3, 7, 5, -1, -1, -1, -1, -1},
	// Knuckles
	{5, 2, 6, 8, 7, -1, -1, -1, -1, -1},
	// Amy
	{4, 10, 9, -1, -1, -1, -1, -1 ,-1 -1},
	// Gamma
	{9, 0, 1, 6, 10, -1, -1, -1, -1, -1},
	// Big
	{4, 3, 0, 10, -1, -1, -1, -1, -1, -1},
};

/**
 * Save file level mapping.
 * Index == display level ID
 * Value == save index (e.g. scores.all[x])
 * A value of -1 indicates an invalid entry.
 */
const int8_t SALevelStatsPrivate::saveMap[6][MAX_LEVELS] = {
	// Sonic (starts at 0)
	{0, 1, 8, 7, 2, 3, 4, 5, 6, 9},
	// Tails (starts at 10)
	{10, 14, 13, 12, 11, -1, -1, -1, -1, -1},
	// Knuckles (starts at 15)
	{15, 19, 16, 18, 17, -1, -1, -1, -1, -1},
	// Amy (starts at 20)
	{20, 22, 21, -1, -1, -1, -1, -1, -1 ,-1},
	// Gamma (starts at 23)
	{26, 23, 24, 25, 27, -1, -1, -1, -1, -1},
	// Big (starts at 28)
	{29, 30, 28, 31, -1, -1, -1, -1, -1, -1},
};

SALevelStatsPrivate::SALevelStatsPrivate(SALevelStats *q)
	: q_ptr(q)
	, character(0)
{
	// Clear the data.
	clear();

	// Cache level names as QStrings.
	for (int i = 0; i < NUM_ELEMENTS(actionStageNameCache); i++) {
		actionStageNameCache[i] = QLatin1String(sa_level_names_action[i]);
	}
}

SALevelStatsPrivate::~SALevelStatsPrivate()
{
	// TODO: Is this needed?
	// The level widgets are owned by this widget, so they
	// should be automatically deleted...
	for (int level = 0; level < MAX_LEVELS; level++) {
		for (int j = 0; j < 3; j++) {
			delete levels[level].chkEmblems[j];
		}
		delete levels[level].lblLevel;
		delete levels[level].spnHighScore;
		delete levels[level].hboxEmblems;
		delete levels[level].tceBestTime;
		delete levels[level].spnMostRings;
	}
}

/**
 * Clear the loaded data.
 * This does NOT automatically update the UI.
 */
void SALevelStatsPrivate::clear(void)
{
	character = 0;
	memset(&scores,  0, sizeof(scores));
	memset(&times,   0, sizeof(times));
	memset(&weights, 0, sizeof(weights));
	memset(&rings,   0, sizeof(rings));
	memset(&emblems, 0, sizeof(emblems));

	// Metal Sonic data.
	memset(&metal_sonic, 0, sizeof(metal_sonic));
	// TODO: Remove Metal Sonic if he's in the Characters box?
	// Or, only do that if loadDX(nullptr) is called?
}

/**
 * Initialize level widgets.
 */
void SALevelStatsPrivate::initLevels(void)
{
	QString qsCssCheckBox = QLatin1String(sa_ui_css_emblem_checkbox);

	// Create widgets for all levels, and hide them initially.
	Q_Q(SALevelStats);
	for (int level = 0; level < MAX_LEVELS; level++) {
		// Level name.
		levels[level].lblLevel = new QLabel(q);

		// High score.
		levels[level].spnHighScore = new QSpinBox(q);
		levels[level].spnHighScore->hide();
		// TODO: Actual maximum?
		levels[level].spnHighScore->setRange(0, 0x7FFFFFFF);
		levels[level].spnHighScore->setSingleStep(1);

		// Emblems.
		// FIXME: Possible memory leak? (does hboxEmblems get deleted?)
		levels[level].hboxEmblems = new QHBoxLayout();
		levels[level].hboxEmblems->setContentsMargins(0, 0, 0, 0);
		for (int j = 0; j < 3; j++) {
			levels[level].chkEmblems[j] = new QCheckBox(q);
			levels[level].chkEmblems[j]->hide();
			levels[level].chkEmblems[j]->setStyleSheet(qsCssCheckBox);
			levels[level].chkEmblems[j]->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
			levels[level].hboxEmblems->addWidget(levels[level].chkEmblems[j], 0, Qt::AlignLeft);
		}

		// Best time.
		levels[level].tceBestTime = new TimeCodeEdit(q);
		levels[level].tceBestTime->hide();

		// Most rings.
		levels[level].spnMostRings = new QSpinBox(q);
		levels[level].spnMostRings->hide();
		// TODO: Actual maximum?
		levels[level].spnMostRings->setRange(0, 0x7FFF);
		levels[level].spnMostRings->setSingleStep(1);

		// Add the widgets to the grid layout.
		ui.gridLevels->addWidget(levels[level].lblLevel,     level+1, 0, Qt::AlignVCenter);
		ui.gridLevels->addWidget(levels[level].spnHighScore, level+1, 1, Qt::AlignVCenter);
		ui.gridLevels->addLayout(levels[level].hboxEmblems,  level+1, 2, Qt::AlignVCenter);
		ui.gridLevels->addWidget(levels[level].tceBestTime,  level+1, 3, Qt::AlignVCenter);
		ui.gridLevels->addWidget(levels[level].spnMostRings, level+1, 4, Qt::AlignVCenter);
	}
}

/**
 * Switch levels to another character.
 * This function automatically calls updateDisplay().
 * @param character Character ID.
 */
void SALevelStatsPrivate::switchLevels(int character)
{
	// FIXME: Character enums or something.
	if (character < 0 || character > 6)
		return;

	// Save the current character's stats.
	saveCurrentStats();

	// Save the new character index.
	this->character = character;

	if (character == 6) {
		// Metal Sonic uses the same level map as Sonic.
		// Run the rest of the function as if Sonic is selected.
		character = 0;
	}

	// Get the level map for the selected character.
	const int8_t *levelID = &levelMap[character][0];

	// Show widgets that are needed.
	int i = 0;
	for (; i < MAX_LEVELS && *levelID != -1; i++, levelID++) {
		// TODO: Cache the level name QStrings.
		levels[i].lblLevel->setText(actionStageNameCache[*levelID]);
		levels[i].lblLevel->show();
		levels[i].spnHighScore->show();
		levels[i].spnMostRings->show();

		// FIXME: Weights for Big?
		for (int j = 0; j < 3; j++) {
			levels[i].chkEmblems[j]->show();
			levels[i].tceBestTime->show();

			if (character == 5) {
				// Big. Change times to weights.
				ui.lblBestTime->setText(SALevelStats::tr("Best Weight:"));
				levels[i].tceBestTime->setDisplayMode(TimeCodeEdit::DM_WEIGHT);
			} else {
				// Other character. Back to times.
				ui.lblBestTime->setText(SALevelStats::tr("Best Time:"));
				levels[i].tceBestTime->setDisplayMode(TimeCodeEdit::DM_MSF);
			}
		}
	}

	// Hide widgets that aren't needed.
	for (int j = i; j < MAX_LEVELS; j++) {
		levels[j].lblLevel->hide();
		levels[j].spnHighScore->hide();
		levels[j].spnMostRings->hide();

		// FIXME: Weights for Big?
		for (int k = 0; k < 3; k++) {
			levels[j].chkEmblems[k]->hide();
			levels[j].tceBestTime->hide();
		}
	}

	// Update the display.
	updateDisplay();
}

/**
 * Update the widgets for the selected character.
 */
void SALevelStatsPrivate::updateDisplay(void)
{
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
}

/**
 * Save the current character's stats.
 */
void SALevelStatsPrivate::saveCurrentStats(void)
{
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
}

/** SALevelStats **/

SALevelStats::SALevelStats(QWidget *parent)
	: super(parent)
	, d_ptr(new SALevelStatsPrivate(this))
{
	Q_D(SALevelStats);
	d->ui.setupUi(this);

	// Initialize the level listing.
	d->initLevels();
	d->switchLevels(d->ui.cboCharacter->currentIndex());
}

SALevelStats::~SALevelStats()
{
	Q_D(SALevelStats);
	delete d;
}

/** Events. **/

/**
 * Widget state has changed.
 * @param event State change event.
 */
void SALevelStats::changeEvent(QEvent *event)
{
	if (event->type() == QEvent::LanguageChange) {
		// Retranslate the UI.
		Q_D(SALevelStats);
		d->ui.retranslateUi(this);

		// Manual retranslation needed for "Metal Sonic".
		if (d->ui.cboCharacter->count() >= 7) {
			d->ui.cboCharacter->setItemText(6, tr("Metal Sonic"));
		}
	}

	// Pass the event to the base class.
	super::changeEvent(event);
}

/** UI widget slots. **/

/**
 * The selected character was changed.
 * @param index New character ID.
 */
void SALevelStats::on_cboCharacter_currentIndexChanged(int index)
{
	// Reinitialize the level grid with the correct
	// set of levels for the selected character.
	Q_D(SALevelStats);
	d->switchLevels(index);
}

/** Public functions. **/

/**
 * Load data from a Sonic Adventure save slot.
 * @param sa_save Sonic Adventure save slot.
 * The data must have already been byteswapped to host-endian.
 * @return 0 on success; non-zero on error.
 */
int SALevelStats::load(const sa_save_slot *sa_save)
{
	Q_D(SALevelStats);
	memcpy(&d->scores,  &sa_save->scores,  sizeof(d->scores));
	memcpy(&d->times,   &sa_save->times,   sizeof(d->times));
	memcpy(&d->weights, &sa_save->weights, sizeof(d->weights));
	memcpy(&d->rings,   &sa_save->rings,   sizeof(d->rings));

	// Emblems are stored as a bitmask. (LSB is emblem 0.)
	// Convert to a bool array to make it easier to access.
	bool *emblem = &d->emblems[0];
	for (int i = 0; i < NUM_ELEMENTS(sa_save->emblems); i++) {
		uint8_t bits = sa_save->emblems[i];
		for (int j = 0; j < 8; j++, bits >>= 1) {
			// TODO: Is the !! needed?
			*emblem++ = !!(bits & 1);
		}
	}

	// Update the display.
	d->updateDisplay();
	setModified(false);
	return 0;
}

/**
 * Save data to a Sonic Adventure save slot.
 * @param sa_save Sonic Adventure save slot.
 * The data will be in host-endian format.
 * @return 0 on success; non-zero on error.
 */
int SALevelStats::save(sa_save_slot *sa_save)
{
	Q_D(const SALevelStats);

	// Save the current character's stats.
	// TODO: Use modification signals to make this unnecessary.
	const_cast<SALevelStatsPrivate*>(d)->saveCurrentStats();

	memcpy(&sa_save->scores,  &d->scores,  sizeof(sa_save->scores));
	memcpy(&sa_save->times,   &d->times,   sizeof(sa_save->times));
	memcpy(&sa_save->weights, &d->weights, sizeof(sa_save->weights));
	memcpy(&sa_save->rings,   &d->rings,   sizeof(sa_save->rings));

	// Emblems are stored as a bitmask. (LSB is emblem 0.)
	// We're using a bool array internally.
	const bool *emblem = &d->emblems[0];
	for (int i = 0; i < NUM_ELEMENTS(sa_save->emblems); i++) {
		uint8_t bits = 0;
		for (int j = 0; j < 8; j++) {
			bits >>= 1;
			bits |= (*emblem++ ? 0x80 : 0);
		}
		sa_save->emblems[i] = bits;
	}

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
int SALevelStats::loadDX(const sadx_extra_save_slot *sadx_extra_save)
{
	Q_D(SALevelStats);

	if (sadx_extra_save) {
		memcpy(&d->metal_sonic.scores, &sadx_extra_save->scores_metal, sizeof(d->metal_sonic.scores));
		memcpy(&d->metal_sonic.times,  &sadx_extra_save->times_metal,  sizeof(d->metal_sonic.times));
		memcpy(&d->metal_sonic.rings,  &sadx_extra_save->rings_metal,  sizeof(d->metal_sonic.rings));

		// Emblems are stored as a bitmask. (LSB is emblem 0.)
		// We're using a bool array internally.
		// TODO: Verify byte ordering on GCN and PC.
		bool *emblem = &d->metal_sonic.emblems[0];
		uint32_t metal_emblems = sadx_extra_save->emblems_metal;
		for (int i = 0; i < NUM_ELEMENTS(d->metal_sonic.emblems); i++) {
			// TODO: Is the !! needed?
			*emblem++ = !!(metal_emblems & 1);
			metal_emblems >>= 1;
		}

		// If the Characters dropdown doesn't have Metal Sonic, add him now.
		// TODO: Remove Metal Sonic on clear()?
		if (d->ui.cboCharacter->count() < 7) {
			QIcon icon(QLatin1String(":/sonic/SA1/metal_sonic.png"));
			d->ui.cboCharacter->addItem(icon, tr("Metal Sonic"));
		}
	} else {
		// Save file is from SA1 and doesn't have SADX extras.
		// Remove Metal Sonic from the Characters dropdown.
		while (d->ui.cboCharacter->count() >= 7) {
			d->ui.cboCharacter->removeItem(6);
		}
	}

	setModified(false);
	return 0;
}

/**
 * Save data to a Sonic Adventure DX extra save slot.
 * @param sadx_extra_save Sonic Adventure DX extra save slot.
 * The data will be in host-endian format.
 * @return 0 on success; non-zero on error.
 */
int SALevelStats::saveDX(sadx_extra_save_slot *sadx_extra_save)
{
	Q_D(SALevelStats);

	// Save the current character's stats.
	// TODO: Use modification signals to make this unnecessary.
	// TODO: Only do this if the current character is Metal Sonic.
	const_cast<SALevelStatsPrivate*>(d)->saveCurrentStats();

	memcpy(&sadx_extra_save->scores_metal, &d->metal_sonic.scores, sizeof(sadx_extra_save->scores_metal));
	memcpy(&sadx_extra_save->times_metal,  &d->metal_sonic.times,  sizeof(sadx_extra_save->times_metal));
	memcpy(&sadx_extra_save->rings_metal,  &d->metal_sonic.rings,  sizeof(sadx_extra_save->rings_metal));

	// Emblems are stored as a bitmask. (LSB is emblem 0.)
	// We're using a bool array internally.
	// TODO: Verify byte ordering on GCN and PC.
	const bool *emblem = &d->metal_sonic.emblems[0];
	uint32_t metal_emblems = 0;
	for (int i = 0; i < NUM_ELEMENTS(d->metal_sonic.emblems); i++) {
		metal_emblems >>= 1;
		// TODO: Test this.
		metal_emblems |= (*emblem++ ? (1 << NUM_ELEMENTS(d->metal_sonic.emblems)) : 0);
	}
	sadx_extra_save->emblems_metal = metal_emblems;

	setModified(false);
	return 0;
}

/**
 * Clear the loaded data.
 */
void SALevelStats::clear(void)
{
	Q_D(SALevelStats);
	d->clear();
	d->updateDisplay();
	setModified(false);
}
