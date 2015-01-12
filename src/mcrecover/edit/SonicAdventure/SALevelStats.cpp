/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * SALevelStats.cpp: Sonic Adventure - Level Stats editor.                 *
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

#include "SALevelStats.hpp"

// Qt includes.
#include <QtCore/QTimer>
#include <QtGui/QPaintEvent>
#include <QtGui/QPainter>

// Qt widgets.
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QSpinBox>
#include <QtGui/QCheckBox>

// Sonic Adventure save file definitions.
#include "sa_defs.h"
#include "util/byteswap.h"

// TODO: Put this in a common header file somewhere.
#define NUM_ELEMENTS(x) ((int)(sizeof(x) / sizeof(x[0])))

/** SALevelStatsPrivate **/

#include "ui_SALevelStats.h"
class SALevelStatsPrivate
{
	public:
		SALevelStatsPrivate(SALevelStats *q);
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
			QSpinBox *spnBestTime[3];
			QHBoxLayout *hboxBestTime;
			QSpinBox *spnMostRings;
		} levels[MAX_LEVELS];
		int levelsInUse;

		/**
		 * Clear all level widgets.
		 */
		void clearLevels(void);

		/**
		 * Initialize level widgets.
		 * This funciton automatically calls updateDisplay().
		 * @param character Character ID.
		 */
		void initLevels(int character);

		/**
		 * Update the widgets with the loaded data.
		 */
		void updateDisplay(void);

		// Save file data.
		sa_scores scores;
		sa_times times;

		/** Static read-only data **/

		// Level names. (ASCII, untranslated)
		static const char *levelNames[11];

		/**
		 * Character level mapping.
		 * Corresponds to levelNames[] entries.
		 * -1 == end of list
		 */
		static const int8_t levelMap[6][MAX_LEVELS];

		/**
		 * Save file level mapping.
		 * Index == display level ID
		 * Value == save index (e.g. scores.all[x])
		 * A value of -1 indicates an invalid entry.
		 */
		static const int8_t saveMap[6][MAX_LEVELS];
};

// Level names. (ASCII, untranslated)
const char *SALevelStatsPrivate::levelNames[11] = {
	"Emerald Coast",	// 0
	"Windy Valley",		// 1
	"Casinopolis",		// 2
	"Ice Cap",		// 3
	"Twinkle Park",		// 4
	"Speed Highway",	// 5
	"Red Mountain",		// 6
	"Sky Deck",		// 7
	"Lost World",		// 8
	"Final Egg",		// 9
	"Hot Shelter"		// 10
};

/**
 * Character level mapping.
 * Corresponds to levelNames[] entries.
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
{
	// Make sure sa_defs.h is correct.
	static_assert(SA_SCORES_LEN == 128, "SA_SCORES_LEN is incorrect");
	static_assert(sizeof(sa_scores) == SA_SCORES_LEN, "sa_scores has the wrong size");
	static_assert(SA_SAVE_FILE_LEN == 1184, "SA_SAVE_FILE_LEN is incorrect");
	static_assert(sizeof(sa_save_file) == SA_SAVE_FILE_LEN, "sa_save_file has the wrong size");

	// No levels are allocated initially.
	levelsInUse = 0;

	// Zero out the scores.
	// TODO: Rest of SA data.
	memset(&scores, 0, sizeof(scores));
}

SALevelStatsPrivate::~SALevelStatsPrivate()
{
	// TODO: Is this needed?
	// The level widgets are owned by this widget, so they
	// should be automatically deleted...
	clearLevels();
}

/**
 * Clear all level widgets.
 */
void SALevelStatsPrivate::clearLevels(void)
{
	for (int i = 0; i < levelsInUse; i++) {
		for (int j = 0; j < 3; j++) {
			delete levels[i].chkEmblems[j];
			delete levels[i].spnBestTime[j];
		}
		delete levels[i].lblLevel;
		delete levels[i].spnHighScore;
		delete levels[i].hboxEmblems;
		delete levels[i].hboxBestTime;
		delete levels[i].spnMostRings;
	}

	// All done.
	// TODO: Take a copy of levelsInUse, then
	// clear it before deleting anything?
	// Probably not needed...
	levelsInUse = 0;
}

/**
 * Initialize level widgets.
 * @param character Character ID.
 */
void SALevelStatsPrivate::initLevels(int character)
{
	// FIXME: Character enums or something.
	if (character < 0 || character > 5)
		return;

	const int8_t *levelID = &levelMap[character][0];
	Q_Q(SALevelStats);

	// Create widgets for levels that aren't already displayed.
	int i = 0;
	for (; i < MAX_LEVELS && *levelID != -1; i++, levelID++) {
		if (i < levelsInUse) {
			// Widgets already exist.
			// Only update the level name.
			// TODO: Change Big's Time widgets to Weight.
			levels[i].lblLevel->setText(QLatin1String(levelNames[*levelID]));
			continue;
		}

		// Create new widgets.

		// Level name.
		levels[i].lblLevel = new QLabel(q);
		levels[i].lblLevel->setText(QLatin1String(levelNames[*levelID]));

		// High score.
		levels[i].spnHighScore = new QSpinBox(q);
		// TODO: Actual maximum?
		levels[i].spnHighScore->setRange(0, 0x7FFFFFFF);
		levels[i].spnHighScore->setSingleStep(1);

		// Emblems.
		// FIXME: Possible memory leak? (does hboxEmblems get deleted?)
		levels[i].hboxEmblems = new QHBoxLayout();
		levels[i].hboxEmblems->setContentsMargins(0, 0, 0, 0);
		for (int j = 0; j < 3; j++) {
			levels[i].chkEmblems[j] = new QCheckBox(q);
			levels[i].hboxEmblems->addWidget(levels[i].chkEmblems[j]);
			// FIXME: Improve alignment.
			levels[i].hboxEmblems->setAlignment(levels[i].chkEmblems[j], Qt::AlignLeft);
		}

		// Best time.
		// FIXME: Possible memory leak? (does hboxBestTime get deleted?)
		levels[i].hboxBestTime = new QHBoxLayout();
		levels[i].hboxBestTime->setContentsMargins(0, 0, 0, 0);
		for (int j = 0; j < 3; j++) {
			levels[i].spnBestTime[j] = new QSpinBox(q);
			levels[i].spnBestTime[j]->setRange(0, (j == 1 ? 59 : 99));
			levels[i].spnBestTime[j]->setSingleStep(1);
			levels[i].hboxBestTime->addWidget(levels[i].spnBestTime[j]);
		}

		// Most rings.
		levels[i].spnMostRings = new QSpinBox(q);
		// TODO: Actual maximum?
		levels[i].spnMostRings->setRange(0, 0x7FFF);
		levels[i].spnMostRings->setSingleStep(1);

		ui.gridLevels->addWidget(levels[i].lblLevel,     i+1, 0, Qt::AlignTop);
		ui.gridLevels->addWidget(levels[i].spnHighScore, i+1, 1, Qt::AlignTop);
		ui.gridLevels->addLayout(levels[i].hboxEmblems,  i+1, 2, Qt::AlignTop);
		ui.gridLevels->addLayout(levels[i].hboxBestTime, i+1, 3, Qt::AlignTop);
		ui.gridLevels->addWidget(levels[i].spnMostRings, i+1, 4, Qt::AlignTop);
	}

	// Delete any level widgets that are no longer in use.
	for (int j = i; j < levelsInUse; j++) {
		for (int k = 0; k < 3; k++) {
			delete levels[j].chkEmblems[k];
			delete levels[j].spnBestTime[k];
		}
		delete levels[j].lblLevel;
		delete levels[j].spnHighScore;
		delete levels[j].hboxEmblems;
		delete levels[j].hboxBestTime;
		delete levels[j].spnMostRings;
	}

	// Store the new number of levels in use.
	this->levelsInUse = i;

	// Update the display.
	updateDisplay();
}

/**
 * Update the widgets with the loaded data.
 */
void SALevelStatsPrivate::updateDisplay(void)
{
	// TODO: Make character a parameter, or not?
	// FIXME: Character enums or something.
	int character = ui.cboCharacter->currentIndex();
	if (character < 0 || character > 5)
		return;

	for (int i = 0; i < levelsInUse; i++) {
		int8_t saveIdx = saveMap[character][i];
		if (saveIdx < 0 || saveIdx >= NUM_ELEMENTS(scores.all))
			break;

		// Score
		levels[i].spnHighScore->setValue(scores.all[saveIdx]);

		// Time (except for Big)
		if (character != 5) {
			levels[i].spnBestTime[0]->setValue(times.all[saveIdx].minutes);
			levels[i].spnBestTime[1]->setValue(times.all[saveIdx].seconds);
			// FIXME: Stored as 1/60th seconds; convert to 1/100th.
			levels[i].spnBestTime[2]->setValue(times.all[saveIdx].frames);
		}
	}
}

/** SALevelStats **/

SALevelStats::SALevelStats(QWidget *parent)
	: QWidget(parent)
	, d_ptr(new SALevelStatsPrivate(this))
{
	Q_D(SALevelStats);
	d->ui.setupUi(this);

	// Fix alignment of the header labels.
	d->ui.gridLevels->setAlignment(d->ui.lblLevelName, Qt::AlignTop);
	d->ui.gridLevels->setAlignment(d->ui.lblHighScore, Qt::AlignTop);
	d->ui.gridLevels->setAlignment(d->ui.lblEmblems, Qt::AlignTop);
	d->ui.gridLevels->setAlignment(d->ui.lblBestTime, Qt::AlignTop);
	d->ui.gridLevels->setAlignment(d->ui.lblMostRings, Qt::AlignTop);

	// Initialize the level listing.
	d->initLevels(d->ui.cboCharacter->currentIndex());
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
	}

	// Pass the event to the base class.
	this->QWidget::changeEvent(event);
}

/** UI widget slots. **/

/**
 * Character was changed.
 * @param index New character ID.
 */
void SALevelStats::on_cboCharacter_currentIndexChanged(int index)
{
	// Reinitialize the level grid with the correct
	// set of levels for the selected character.
	Q_D(SALevelStats);
	d->initLevels(index);
}

/** Public functions. **/

/**
 * Load data from Sonic Adventure save data.
 * TODO: Big Endian / Little Endian switch?
 * @param sa_save Sonic Adventure save data.
 * @return 0 on success; non-zero on error.
 */
int SALevelStats::loadSaveData(const _sa_save_file *sa_save)
{
	Q_D(SALevelStats);
	// FIXME: Add endianness settings somewhere.

	// Load the scores.
	memcpy(&d->scores, &sa_save->scores, sizeof(d->scores));
	for (int i = 0; i < NUM_ELEMENTS(d->scores.all); i++) {
		d->scores.all[i] = be32_to_cpu(d->scores.all[i]);
	}

	// Load the times.
	// NOTE: This doesn't require byteswapping.
	memcpy(&d->times, &sa_save->times, sizeof(d->times));

	// Update the display.
	d->updateDisplay();

	return 0;
}
