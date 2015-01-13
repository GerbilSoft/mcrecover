/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * SALevelClearCount.hpp: Sonic Adventure - Level Clear Count editor.      *
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

#include "SALevelClearCount.hpp"

// Qt widgets.
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QSpinBox>
#include <QtGui/QCheckBox>

// Sonic Adventure save file definitions.
#include "sa_defs.h"

// TODO: Put this in a common header file somewhere.
#define NUM_ELEMENTS(x) ((int)(sizeof(x) / sizeof(x[0])))

/** SALevelClearCountPrivate **/

#include "ui_SALevelClearCount.h"
class SALevelClearCountPrivate
{
	public:
		SALevelClearCountPrivate(SALevelClearCount *q);
		~SALevelClearCountPrivate();

	protected:
		SALevelClearCount *const q_ptr;
		Q_DECLARE_PUBLIC(SALevelClearCount)
	private:
		Q_DISABLE_COPY(SALevelClearCountPrivate)

	public:
		Ui_SALevelClearCount ui;

		// Total of 43 levels.
		#define TOTAL_LEVELS NUM_ELEMENTS(sa_level_clear_count::clear[0])
		// Level widgets.
		struct {
			QLabel *lblLevel;
			QSpinBox *spnCount[8];
		} levels[TOTAL_LEVELS];

		// Save file data.
		sa_level_clear_count clear_count;

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
		 * Update the widgets with the loaded data.
		 */
		void updateDisplay(void);

		/** Static read-only data **/

		// Level names. (ASCII, untranslated)
		static const char *levelNames[TOTAL_LEVELS];
};

// Level names. (ASCII, untranslated)
const char *SALevelClearCountPrivate::levelNames[TOTAL_LEVELS] = {
	"Hedgehog Hammer", "Emerald Coast", "Windy Valley", "Twinkle Park",	// 0-3
	"Speed Highway", "Red Mountain", "Sky Deck", "Lost World",		// 4-7
	"Ice Cap", "Casinopolis", "Final Egg", "Unused (11)",			// 8-11
	"Hot Shelter", "Unused (13)", "Unused (14)", "Chaos 0",			// 12-15
	"Chaos 2", "Chaos 4", "Chaos 6", "Perfect Chaos",			// 16-19
	"Egg Hornet", "Egg Walker", "Egg Viper", "ZERO",			// 20-23
	"E-101 Beta", "E-101 mkII", "Station Square", "Unused (27)",		// 24-27
	"Unused (28)", "Egg Carrier (Outside)", "Unused (30)", "Unused (31)",	// 28-31
	"Egg Carrier (Inside)", "Mystic Ruins", "The Past", "Twinkle Circuit",	// 32-35
	"Sky Chase Act 1", "Sky Chase Act 2", "Sand Hill", "Station Square\nChao Garden", // 36-39
	"Egg Carrier\nChao Garden", "Mystic Ruins\nChao Garden", "Chao Race",	// 40-42
};

SALevelClearCountPrivate::SALevelClearCountPrivate(SALevelClearCount *q)
	: q_ptr(q)
{
	// Clear the data.
	clear();
}

SALevelClearCountPrivate::~SALevelClearCountPrivate()
{
	// TODO: Is this needed?
	// The level widgets are owned by this widget, so they
	// should be automatically deleted...
	for (int i = 0; i < TOTAL_LEVELS; i++) {
		delete levels[i].lblLevel;
		for (int j = 0; j < NUM_ELEMENTS(levels[i].spnCount); j++) {
			delete levels[i].spnCount[j];
		}
	}
}

/**
 * Clear the loaded data.
 * This does NOT automatically update the UI.
 */
void SALevelClearCountPrivate::clear(void)
{
	memset(&clear_count, 0, sizeof(clear_count));
}

/**
 * Initialize level widgets.
 */
void SALevelClearCountPrivate::initLevels(void)
{
	// Create widgets for all levels.
	// TODO: Top or VCenter?
	// TODO: Scroll area is screwing with minimum width...
	Q_Q(SALevelClearCount);
	for (int level = 0; level < TOTAL_LEVELS; level++) {
		// Level name.
		levels[level].lblLevel = new QLabel(q);
		levels[level].lblLevel->setText(QLatin1String(levelNames[level]));
		levels[level].lblLevel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
		ui.gridLevels->addWidget(levels[level].lblLevel, level+1, 0, Qt::AlignTop);

		// Spinbox for each character.
		for (int chr = 0; chr < NUM_ELEMENTS(levels[level].spnCount); chr++) {
			levels[level].spnCount[chr] = new QSpinBox(q);
			levels[level].spnCount[chr]->setRange(0, 255);
			levels[level].spnCount[chr]->setSingleStep(1);
			levels[level].spnCount[chr]->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
			ui.gridLevels->addWidget(levels[level].spnCount[chr], level+1, chr+1, Qt::AlignTop);
		}
	}
}

/**
 * Update the widgets with the loaded data.
 */
void SALevelClearCountPrivate::updateDisplay(void)
{
	// NOTE: Outer loop is the character due to cache locality.
	for (int chr = 0; chr < NUM_ELEMENTS(levels[0].spnCount); chr++) {
		for (int level = 0; level < NUM_ELEMENTS(levels); level++) {
			levels[level].spnCount[chr]->setValue(clear_count.clear[chr][level]);
		}
	}
}

/** SALevelClearCount **/

SALevelClearCount::SALevelClearCount(QWidget *parent)
	: QWidget(parent)
	, d_ptr(new SALevelClearCountPrivate(this))
{
	Q_D(SALevelClearCount);
	d->ui.setupUi(this);

	// Fix alignment of the header labels.
	d->ui.gridLevels->setAlignment(d->ui.lblLevelName, Qt::AlignTop);
	d->ui.gridLevels->setAlignment(d->ui.lblSonic,     Qt::AlignTop);
	d->ui.gridLevels->setAlignment(d->ui.lblUnused1,   Qt::AlignTop);
	d->ui.gridLevels->setAlignment(d->ui.lblTails,     Qt::AlignTop);
	d->ui.gridLevels->setAlignment(d->ui.lblKnuckles,  Qt::AlignTop);
	d->ui.gridLevels->setAlignment(d->ui.lblUnused2,   Qt::AlignTop);
	d->ui.gridLevels->setAlignment(d->ui.lblAmy,       Qt::AlignTop);
	d->ui.gridLevels->setAlignment(d->ui.lblGamma,     Qt::AlignTop);
	d->ui.gridLevels->setAlignment(d->ui.lblBig,       Qt::AlignTop);

	// Initialize the level listing.
	d->initLevels();
}

SALevelClearCount::~SALevelClearCount()
{
	Q_D(SALevelClearCount);
	delete d;
}

/** Events. **/

/**
 * Widget state has changed.
 * @param event State change event.
 */
void SALevelClearCount::changeEvent(QEvent *event)
{
	if (event->type() == QEvent::LanguageChange) {
		// Retranslate the UI.
		Q_D(SALevelClearCount);
		d->ui.retranslateUi(this);
	}

	// Pass the event to the base class.
	this->QWidget::changeEvent(event);
}

/** Public functions. **/

/**
 * Load data from Sonic Adventure save slot.
 * @param sa_save Sonic Adventure save slot.
 * The data must have already been byteswapped to host-endian.
 * @return 0 on success; non-zero on error.
 */
int SALevelClearCount::load(const sa_save_slot *sa_save)
{
	Q_D(SALevelClearCount);
	memcpy(&d->clear_count, &sa_save->clear_count, sizeof(d->clear_count));

	// Update the display.
	d->updateDisplay();
	return 0;
}

/**
 * Clear the loaded data.
 */
void SALevelClearCount::clear(void)
{
	Q_D(SALevelClearCount);
	d->clear();
	d->updateDisplay();
}
