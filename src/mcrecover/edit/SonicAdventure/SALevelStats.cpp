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

// Qt animation includes.
#include <QtCore/QTimeLine>

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
		 * @param character Character ID.
		 */
		void initLevels(int character);

		// Level names. (ASCII, untranslated)
		static const char *levelNames[11];

		/**
		 * Character level mapping.
		 * Corresponds to levelNames[] entries.
		 * -1 == end of list
		 */
		static const int8_t levelMap[6][MAX_LEVELS];
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

SALevelStatsPrivate::SALevelStatsPrivate(SALevelStats *q)
	: q_ptr(q)
{
	// No levels are allocated initially.
	levelsInUse = 0;
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
	// Clear everything first so we have a clean slate.
	clearLevels();

	// FIXME: Character enums or something.
	if (character < 0 || character > 5)
		return;

	const int8_t *levelID = &levelMap[character][0];
	Q_Q(SALevelStats);
	int i = 0;
	for (; i < MAX_LEVELS && *levelID != -1; i++, levelID++) {
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

	// Store the number of levels in use.
	this->levelsInUse = i;
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
