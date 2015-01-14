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

// Qt widgets.
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QSpinBox>
#include <QtGui/QCheckBox>

// Sonic Adventure save file definitions.
#include "sa_defs.h"

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

		// Save file data.
		sa_scores scores;
		sa_times times;
		sa_weights weights;
		sa_rings rings;

		// Emblems.
		// NOTE: There's only 130 emblems, but there's enough
		// space for 136, so we have to allocate the entire area.
		bool emblems[136];

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
		 * This funciton automatically calls updateDisplay().
		 * @param character Character ID.
		 */
		void switchLevels(int character);

		/**
		 * Update the widgets with the loaded data.
		 */
		void updateDisplay(void);

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
	// Clear the data.
	clear();
}

SALevelStatsPrivate::~SALevelStatsPrivate()
{
	// TODO: Is this needed?
	// The level widgets are owned by this widget, so they
	// should be automatically deleted...
	for (int level = 0; level < MAX_LEVELS; level++) {
		for (int j = 0; j < 3; j++) {
			delete levels[level].chkEmblems[j];
			delete levels[level].spnBestTime[j];
		}
		delete levels[level].lblLevel;
		delete levels[level].spnHighScore;
		delete levels[level].hboxEmblems;
		delete levels[level].hboxBestTime;
		delete levels[level].spnMostRings;
	}
}

/**
 * Clear the loaded data.
 * This does NOT automatically update the UI.
 */
void SALevelStatsPrivate::clear(void)
{
	memset(&scores, 0, sizeof(scores));
	memset(&times, 0, sizeof(times));
	memset(&weights, 0, sizeof(weights));
	memset(&rings, 0, sizeof(rings));
	memset(&emblems, 0, sizeof(emblems));
}

/**
 * Initialize level widgets.
 */
void SALevelStatsPrivate::initLevels(void)
{
	static const char cssCheckBox[] =
		"QCheckBox::indicator {\n"
		"\twidth: 28px;\n"
		"\theight: 20px;\n"
		"}\n"
		"QCheckBox::indicator:checked {\n"
		"\timage: url(:/sonic/emblem.png);\n"
		"}\n"
		"QCheckBox::indicator:unchecked {\n"
		"\timage: url(:/sonic/noemblem.png);\n"
		"}\n";
	QString qsCssCheckBox = QLatin1String(cssCheckBox);

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
			levels[level].hboxEmblems->addWidget(levels[level].chkEmblems[j]);
			// FIXME: Improve alignment.
			levels[level].hboxEmblems->setAlignment(levels[level].chkEmblems[j], Qt::AlignLeft);
		}

		// Best time.
		// FIXME: Possible memory leak? (does hboxBestTime get deleted?)
		levels[level].hboxBestTime = new QHBoxLayout();
		levels[level].hboxBestTime->setContentsMargins(0, 0, 0, 0);
		for (int j = 0; j < 3; j++) {
			levels[level].spnBestTime[j] = new QSpinBox(q);
			levels[level].spnBestTime[j]->hide();
			// Other parameters are set in switchLevels().
			levels[level].hboxBestTime->addWidget(levels[level].spnBestTime[j]);
		}

		// Most rings.
		levels[level].spnMostRings = new QSpinBox(q);
		levels[level].spnMostRings->hide();
		// TODO: Actual maximum?
		levels[level].spnMostRings->setRange(0, 0x7FFF);
		levels[level].spnMostRings->setSingleStep(1);

		ui.gridLevels->addWidget(levels[level].lblLevel,     level+1, 0, Qt::AlignTop);
		ui.gridLevels->addWidget(levels[level].spnHighScore, level+1, 1, Qt::AlignTop);
		ui.gridLevels->addLayout(levels[level].hboxEmblems,  level+1, 2, Qt::AlignTop);
		ui.gridLevels->addLayout(levels[level].hboxBestTime, level+1, 3, Qt::AlignTop);
		ui.gridLevels->addWidget(levels[level].spnMostRings, level+1, 4, Qt::AlignTop);
	}
}

/**
 * Switch levels to another character.
 * This funciton automatically calls updateDisplay().
 * @param character Character ID.
 */
void SALevelStatsPrivate::switchLevels(int character)
{
	// FIXME: Character enums or something.
	if (character < 0 || character > 5)
		return;

	const int8_t *levelID = &levelMap[character][0];

	// Show widgets that are needed.
	int i = 0;
	for (; i < MAX_LEVELS && *levelID != -1; i++, levelID++) {
		levels[i].lblLevel->setText(QLatin1String(levelNames[*levelID]));
		levels[i].lblLevel->show();
		levels[i].spnHighScore->show();
		levels[i].spnMostRings->show();

		// FIXME: Weights for Big?
		for (int j = 0; j < 3; j++) {
			levels[i].chkEmblems[j]->show();
			levels[i].spnBestTime[j]->show();

			if (character == 5) {
				// Big. Change times to weights.
				ui.lblBestTime->setText(SALevelStats::tr("Best Weight:"));
				levels[i].spnBestTime[j]->setRange(0, 655350);
				levels[i].spnBestTime[j]->setSingleStep(10);
				levels[i].spnBestTime[j]->setSuffix(QLatin1String("g"));
			} else {
				// Other character. Back to times.
				ui.lblBestTime->setText(SALevelStats::tr("Best Time:"));
				levels[i].spnBestTime[j]->setRange(0, (j == 1 ? 59 : 99));
				levels[i].spnBestTime[j]->setSingleStep(1);
				levels[i].spnBestTime[j]->setSuffix(QString());
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
			levels[j].spnBestTime[k]->hide();
		}
	}

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

	for (int level = 0; level < MAX_LEVELS; level++) {
		const int8_t saveIdx = saveMap[character][level];
		if (saveIdx < 0 || saveIdx >= NUM_ELEMENTS(scores.all))
			break;

		// Score
		levels[level].spnHighScore->setValue(scores.all[saveIdx]);

		// Time / Weight
		if (character != 5) {
			// Time (not Big)
			levels[level].spnBestTime[0]->setValue(times.all[saveIdx].minutes);
			levels[level].spnBestTime[1]->setValue(times.all[saveIdx].seconds);
			// FIXME: Stored as 1/60th seconds; convert to 1/100th.
			levels[level].spnBestTime[2]->setValue(times.all[saveIdx].frames);
		} else {
			// Weight (Big)
			const int8_t bigSaveIdx = (saveIdx - 28);
			for (int j = 0; j < 3; j++) {
				const int weight = (weights.levels[bigSaveIdx][j] * 10);
				levels[level].spnBestTime[j]->setValue(weight);
			}
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
	d->switchLevels(index);
}

/** Public functions. **/

/**
 * Load data from Sonic Adventure save slot.
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
			*emblem++ = !!(bits & 1);
		}
	}

	// Update the display.
	d->updateDisplay();
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
}
