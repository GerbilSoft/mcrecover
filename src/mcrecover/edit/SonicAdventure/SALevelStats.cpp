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
		struct Ui_SALevelStats {
			QVBoxLayout *vboxMain;
			QGridLayout *gridLevels;

			// Grid labels.
			QLabel *lblLevelName;
			QLabel *lblHighScore;
			QLabel *lblEmblems;
			QLabel *lblBestTime;
			QLabel *lblMostRings;

			#define MAX_LEVELS 11
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

			void setupUi(QWidget *SALevelStats);
			void retranslateUi(QWidget *SALevelStats);
		};
		Ui_SALevelStats ui;

		/**
		 * Clear all level widgets.
		 */
		void clearLevels(void);

		/**
		 * Initialize level widgets.
		 * TODO: Specify a character.
		 */
		void initLevels(void);

		// Level names. (ASCII, untranslated)
		static const char *levelNames[MAX_LEVELS];
};

// Level names. (ASCII, untranslated)
const char *SALevelStatsPrivate::levelNames[MAX_LEVELS] = {
	"Emerald Coast",
	"Windy Valley",
	"Casinopolis",
	"Ice Cap",
	"Twinkle Park",
	"Speed Highway",
	"Red Mountain",
	"Sky Deck",
	"Lost World",
	"Final Egg",
	"Hot Shelter"
};

SALevelStatsPrivate::SALevelStatsPrivate(SALevelStats *q)
	: q_ptr(q)
{
	// No levels are allocated initially.
	ui.levelsInUse = 0;
}

SALevelStatsPrivate::~SALevelStatsPrivate()
{
	// TODO: Is this needed?
	// The level widgets are owned by this widget, so they
	// should be automatically deleted...
	clearLevels();
}

/**
 * Initialize the UI.
 * @param SALevelStats SALevelStats.
 */
void SALevelStatsPrivate::Ui_SALevelStats::setupUi(QWidget *SALevelStats)
{
	if (SALevelStats->objectName().isEmpty())
		SALevelStats->setObjectName(QLatin1String("SALevelStats"));

	vboxMain = new QVBoxLayout(SALevelStats);
	vboxMain->setContentsMargins(0, 0, 0, 0);
	vboxMain->setObjectName(QLatin1String("vboxMain"));

	// FIXME: Possible memory leak? (does gridLevels get deleted?)
	gridLevels = new QGridLayout();
	gridLevels->setContentsMargins(0, 0, 0, 0);
	gridLevels->setObjectName(QLatin1String("gridLevels"));
	vboxMain->addLayout(gridLevels);
	vboxMain->setAlignment(gridLevels, Qt::AlignTop);

	// Grid labels.
	lblLevelName = new QLabel(SALevelStats);
	lblLevelName->setObjectName(QLatin1String("lblLevelName"));
	gridLevels->addWidget(lblLevelName, 0, 0);

	lblHighScore = new QLabel(SALevelStats);
	lblHighScore->setObjectName(QLatin1String("lblHighScore"));
	gridLevels->addWidget(lblHighScore, 0, 1);

	lblEmblems = new QLabel(SALevelStats);
	lblEmblems->setObjectName(QLatin1String("lblEmblems"));
	gridLevels->addWidget(lblEmblems, 0, 2);

	lblBestTime = new QLabel(SALevelStats);
	lblBestTime->setObjectName(QLatin1String("lblBestTime"));
	gridLevels->addWidget(lblBestTime, 0, 3);

	lblMostRings = new QLabel(SALevelStats);
	lblMostRings->setObjectName(QLatin1String("lblMostRings"));
	gridLevels->addWidget(lblMostRings, 0, 4);

	retranslateUi(SALevelStats);

	QMetaObject::connectSlotsByName(SALevelStats);
}

/**
 * Retranslate the UI.
 * @param SALevelStats SALevelStats.
 */
void SALevelStatsPrivate::Ui_SALevelStats::retranslateUi(QWidget *SALevelStats)
{
	lblLevelName->setText(SALevelStats::tr("Level:"));
	lblHighScore->setText(SALevelStats::tr("High Score:"));
	lblEmblems->setText(SALevelStats::tr("Emblems:"));
	lblBestTime->setText(SALevelStats::tr("Best Time:"));
	lblMostRings->setText(SALevelStats::tr("Most Rings:"));
	Q_UNUSED(SALevelStats);
}

/**
 * Clear all level widgets.
 */
void SALevelStatsPrivate::clearLevels(void)
{
	for (int i = 0; i < ui.levelsInUse; i++) {
		delete ui.levels[i].lblLevel;
		delete ui.levels[i].spnHighScore;
		delete ui.levels[i].chkEmblems[3];
		delete ui.levels[i].hboxEmblems;
		delete ui.levels[i].spnBestTime[3];
		delete ui.levels[i].hboxBestTime;
		delete ui.levels[i].spnMostRings;
	}

	// All done.
	// TODO: Take a copy of levelsInUse, then
	// clear it before deleting anything?
	// Probably not needed...
	ui.levelsInUse = 0;
}

/**
 * Initialize level widgets.
 * TODO: Specify a character.
 */
void SALevelStatsPrivate::initLevels(void)
{
	// Clear everything first so we have a clean slate.
	clearLevels();

	// TODO: Use a character ID to determine how many levels
	// and which levels to use.
	Q_Q(SALevelStats);
	for (int i = 0; i < MAX_LEVELS; i++) {
		// Level name.
		ui.levels[i].lblLevel = new QLabel(q);
		ui.levels[i].lblLevel->setText(QLatin1String(levelNames[i]));

		// High score.
		ui.levels[i].spnHighScore = new QSpinBox(q);
		// TODO: Actual maximum?
		ui.levels[i].spnHighScore->setRange(0, 0x7FFFFFFF);
		ui.levels[i].spnHighScore->setSingleStep(1);

		// Emblems.
		// FIXME: Possible memory leak? (does hboxEmblems get deleted?)
		ui.levels[i].hboxEmblems = new QHBoxLayout();
		ui.levels[i].hboxEmblems->setContentsMargins(0, 0, 0, 0);
		for (int j = 0; j < 3; j++) {
			ui.levels[i].chkEmblems[j] = new QCheckBox(q);
			ui.levels[i].hboxEmblems->addWidget(ui.levels[i].chkEmblems[j]);
		}

		// Best time.
		// FIXME: Possible memory leak? (does hboxBestTime get deleted?)
		ui.levels[i].hboxBestTime = new QHBoxLayout();
		ui.levels[i].hboxBestTime->setContentsMargins(0, 0, 0, 0);
		for (int j = 0; j < 3; j++) {
			ui.levels[i].spnBestTime[j] = new QSpinBox(q);
			ui.levels[i].spnBestTime[j]->setRange(0, (j == 1 ? 59 : 99));
			ui.levels[i].spnBestTime[j]->setSingleStep(1);
			ui.levels[i].hboxBestTime->addWidget(ui.levels[i].spnBestTime[j]);
		}

		// Most rings.
		ui.levels[i].spnMostRings = new QSpinBox(q);
		// TODO: Actual maximum?
		ui.levels[i].spnMostRings->setRange(0, 0x7FFF);
		ui.levels[i].spnMostRings->setSingleStep(1);

		ui.gridLevels->addWidget(ui.levels[i].lblLevel, i+1, 0);
		ui.gridLevels->addWidget(ui.levels[i].spnHighScore, i+1, 1);
		ui.gridLevels->addLayout(ui.levels[i].hboxEmblems, i+1, 2);
		ui.gridLevels->addLayout(ui.levels[i].hboxBestTime, i+1, 3);
		ui.gridLevels->addWidget(ui.levels[i].spnMostRings, i+1, 4);
	}
}

/** SALevelStats **/

SALevelStats::SALevelStats(QWidget *parent)
	: QWidget(parent)
	, d_ptr(new SALevelStatsPrivate(this))
{
	Q_D(SALevelStats);
	d->ui.setupUi(this);

	d->initLevels();
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
