/***************************************************************************
 * GameCube Memory Card Recovery Program [libsaveedit]                     *
 * SALevelClearCount.hpp: Sonic Adventure - Level Clear Count editor.      *
 *                                                                         *
 * Copyright (c) 2015-2018 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#include "SALevelClearCount.hpp"

// Qt includes.
#include <QtCore/QSignalMapper>
#include <QApplication>

// Qt widgets.
#include <QHBoxLayout>
#include <QLabel>
#include <QSpinBox>
#include <QCheckBox>
#include <QtGui/QResizeEvent>
#include <QScrollBar>

// C includes. (C++ namespace)
#include <cstring>

// Sonic Adventure save file definitions.
#include "sa_defs.h"

// Common data.
#include "SAData.h"

// TODO: Put this in a common header file somewhere.
#define NUM_ELEMENTS(x) ((int)(sizeof(x) / sizeof(x[0])))

/** SALevelClearCountPrivate **/

#include "ui_SALevelClearCount.h"
class SALevelClearCountPrivate
{
	public:
		explicit SALevelClearCountPrivate(SALevelClearCount *q);
		~SALevelClearCountPrivate();

	private:
		SALevelClearCount *const q_ptr;
		Q_DECLARE_PUBLIC(SALevelClearCount)
		Q_DISABLE_COPY(SALevelClearCountPrivate)

	public:
		Ui_SALevelClearCount ui;

		// Total of 43 levels.
		// FIXME: sizeof(sa_level_clear_counts::clear[0]) works in gcc,
		// but fails in MSVC 2010 with C2597.
		#define TOTAL_LEVELS 43
		#define TOTAL_CHARACTERS 8
		// Level widgets.
		struct {
			QLabel *lblLevel;
			QSpinBox *spnCount[TOTAL_CHARACTERS];
		} levels[TOTAL_LEVELS];

		// Signal mapper for QSpinBox modifications.
		QSignalMapper *mapperSpinBox;

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

		/**
		 * QScrollArea was resized, or QScrollBar visibility changed.
		 * @param event QEvent.
		 */
		void scrollAreaResized(QEvent *event);

		/** Static read-only data. **/

		/**
		 * Character level mapping.
		 * Each entry is a bitfield indicating if the
		 * character is able to access the level normally.
		 * - Index = character ID (including unused)
		 * - Value = bitfield
		 */
		static const uint64_t levelMap[TOTAL_CHARACTERS];
};

/**
 * Character level mapping.
 * Each entry is a bitfield indicating if the
 * character is able to access the level normally.
 * - Index = character ID (including unused)
 * - Value = bitfield
 */
const uint64_t SALevelClearCountPrivate::levelMap[TOTAL_CHARACTERS] = {
	// These values are based on my 100% save file.
	// NOTE: Chao Gardens are all listed as 0.
	// FIXME: Some values may be incorrect...
	0x00000078005E87FEULL,	// Sonic
	0x0000000000000000ULL,	// Unused
	0x0000007800320376ULL,	// Tails
	0x00000008000702F0ULL,	// Knuckles
	0x0000000000000000ULL,	// Unused
	0x0000000900801409ULL,	// Amy
	0x0000000803001426ULL,	// Gamma
	0x000000080004110AULL,	// Big
};

SALevelClearCountPrivate::SALevelClearCountPrivate(SALevelClearCount *q)
	: q_ptr(q)
	, mapperSpinBox(new QSignalMapper(q))
{
	// Clear the data.
	clear();

	// Connect the QSignalMapper slots.
	QObject::connect(mapperSpinBox, SIGNAL(mapped(int)),
			 q, SLOT(spnCount_mapped_slot(int)));
}

SALevelClearCountPrivate::~SALevelClearCountPrivate()
{
	// TODO: Is this needed?
	// The level widgets are owned by this widget, so they
	// should be automatically deleted...
	for (int i = TOTAL_LEVELS-1; i >= 0; i--) {
		delete levels[i].lblLevel;
		for (int j = NUM_ELEMENTS(levels[i].spnCount)-1; j >= 0; j--) {
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
	// Initialize the background color for unused clear counts.
	// TODO: Character to level mapping?
	QPalette pal = QApplication::palette("QSpinBox");
	// Use a light pink color.
	// TODO: Modify the existing base() color?
	// (Manipulating HSV isn't quite reliable...)
	// [Also, update the MemCardModel color generation.]
	QColor bgColor_unused = QColor(255, 192, 192);
	pal.setColor(QPalette::Base, bgColor_unused);

	// Level mappings.
	uint64_t levelMap[TOTAL_CHARACTERS];
	memcpy(levelMap, this->levelMap, sizeof(levelMap));

	// Create widgets for all levels.
	// TODO: Top or VCenter?
	// TODO: Scroll area is screwing with minimum width...
	Q_Q(SALevelClearCount);
	for (int level = 0; level < TOTAL_LEVELS; level++) {
		// Get the level name.
		QString levelName;
		// These level slots are marked as unused in SASave.
		switch (level) {
			case 11: case 13: case 14: case 27:
			case 28: case 30: case 31:
				// Level name is unused here.
				levelName = SALevelClearCount::tr("Unused (%1)").arg(level);
				break;
			default:
				// Use the level name.
				levelName = QLatin1String(sa_level_names_all[level]);
				break;
		}

		// Level name.
		levels[level].lblLevel = new QLabel(q);
		levels[level].lblLevel->setText(levelName);
		levels[level].lblLevel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
		ui.gridLevels->addWidget(levels[level].lblLevel, level, 0, Qt::AlignVCenter);

		// Spinbox for each character.
		for (int chr = 0; chr < NUM_ELEMENTS(levels[level].spnCount); chr++) {
			levels[level].spnCount[chr] = new QSpinBox(q);
			levels[level].spnCount[chr]->setRange(0, 255);
			levels[level].spnCount[chr]->setSingleStep(1);
			levels[level].spnCount[chr]->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
			ui.gridLevels->addWidget(levels[level].spnCount[chr], level, chr+1, Qt::AlignVCenter);

			if (!(levelMap[chr] & 1)) {
				// Level is not used by this character.
				// TODO: Set a stylesheet instead of a palette color?
				// Palette color works on Linux, but it usually causes
				// the widget to not be themed correctly on Windows.
				levels[level].spnCount[chr]->setPalette(pal);
			}
			levelMap[chr] >>= 1;

			// Connect the valueChanged() signal.
			QObject::connect(levels[level].spnCount[chr], SIGNAL(valueChanged(int)),
					 mapperSpinBox, SLOT(map()));
			mapperSpinBox->setMapping(levels[level].spnCount[chr],
						((level << 8) | chr));
		}
	}

	// Add a spacer to make sure everything is pushed to the top
	// if the window is expanded.
	// NOTE: QGridLayout takes ownership, so we shouldn't delete it ourselves.
	QSpacerItem *vspcSpacer = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
	ui.gridLevels->addItem(vspcSpacer, TOTAL_LEVELS, 0, 1, TOTAL_CHARACTERS+1);
}

/**
 * Update the widgets with the loaded data.
 */
void SALevelClearCountPrivate::updateDisplay(void)
{
	// Temporarily disconnect the signal mapper.
	// TODO: Mutex?
	Q_Q(SALevelClearCount);
	QObject::disconnect(mapperSpinBox, SIGNAL(mapped(int)),
			    q, SLOT(spnCount_mapped_slot(int)));

	// NOTE: Outer loop is the character due to cache locality.
	for (int chr = NUM_ELEMENTS(levels[0].spnCount)-1; chr >= 0; chr--) {
		for (int level = NUM_ELEMENTS(levels)-1; level >= 0; level--) {
			levels[level].spnCount[chr]->setValue(clear_count.all[chr][level]);
		}
	}

	// Reconnect the signal mapper.
	QObject::connect(mapperSpinBox, SIGNAL(mapped(int)),
			 q, SLOT(spnCount_mapped_slot(int)));
}

/**
 * QScrollArea was resized, or QScrollBar visibility changed.
 * @param event QEvent.
 */
void SALevelClearCountPrivate::scrollAreaResized(QEvent *event)
{
	if (event->type() == QEvent::Resize) {
		QResizeEvent *resizeEvent = static_cast<QResizeEvent*>(event);
		if (resizeEvent->oldSize().width() == resizeEvent->size().width()) {
			// Vertical resize only.
			return;
		}
	}
	// TODO: On QScrollBar show, change QScrollArea's
	// vertical scrollbar policy to "ScrollBarAsNeeded"?
	// It's set to "ScrollBarAlwaysOn" in order to eliminate
	// some flickering on load.

	// Set lblLevel's width to match the widest widget in column 0.
	// NOTE: Final row is a spacer item, so ignore it.
	int width = 0;
	for (int i = ui.gridLevels->rowCount()-2; i >= 0; i--) {
		QLayoutItem *item = ui.gridLevels->itemAtPosition(i, 0);
		if (item) {
			width = qMax(item->sizeHint().width(), width);
		}
	}
	ui.lblLevelName->setMinimumSize(QSize(width, 0));

	// Set padding to match scroll area padding.
	// The padding includes contentMargins from scrlLevels and gridLevels,
	// plus the width of the vertical scrollbar.
	QMargins margins1 = ui.scrlLevels->contentsMargins();
	QMargins margins2 = ui.gridLevels->contentsMargins();

	QMargins hMargins;
	hMargins.setLeft(margins1.left() + margins2.left());
	hMargins.setRight(margins1.right() + margins2.right());

	// Set the new gridHeader margins.
	ui.gridHeader->setContentsMargins(hMargins);
}

/** SALevelClearCount **/

SALevelClearCount::SALevelClearCount(QWidget *parent)
	: super(parent)
	, d_ptr(new SALevelClearCountPrivate(this))
{
	Q_D(SALevelClearCount);
	d->ui.setupUi(this);

	// Initialize the level listing.
	d->initLevels();

	// Install event filters for the QScrollArea.
	d->ui.scrlLevels->installEventFilter(this);
	d->ui.scrlLevels->verticalScrollBar()->installEventFilter(this);
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
	super::changeEvent(event);
}

/** Public functions. **/

/**
 * Load data from a Sonic Adventure save slot.
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
	setModified(false);
	return 0;
}

/**
 * Save data to a Sonic Adventure save slot.
 * @param sa_save Sonic Adventure save slot.
 * The data will be in host-endian format.
 * @return 0 on success; non-zero on error.
 */
int SALevelClearCount::save(sa_save_slot *sa_save)
{
	Q_D(const SALevelClearCount);
	memcpy(&sa_save->clear_count, &d->clear_count, sizeof(sa_save->clear_count));
	setModified(false);
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
	setModified(false);
}

/** Slots. **/

/**
 * spnCount signal mapper.
 * @param spnId Spinbox ID. (0xAABB; AA == level, BB == character)
 */
void SALevelClearCount::spnCount_mapped_slot(int spnId)
{
	// Save the data for the spinbox.
	const int chr = (spnId & 0xFF);
	const int level = (spnId >> 8);
	if (chr < 0 || chr >= TOTAL_CHARACTERS || level < 0 || level >= TOTAL_LEVELS)
		return;

	Q_D(SALevelClearCount);
	if (d->clear_count.all[chr][level] != d->levels[level].spnCount[chr]->value()) {
		// Value has changed.
		d->clear_count.all[chr][level] = d->levels[level].spnCount[chr]->value();
		this->widgetModifiedSlot();
	}
}

/**
 * QObject eventFilter.
 * Used to handle QScrollArea resize and QScrollBar show events.
 * @param watched Watched QObject.
 * @param event QEvent.
 * @return True to stop the event from being handled further; false to pass it down.
 */
bool SALevelClearCount::eventFilter(QObject *watched, QEvent *event)
{
	Q_D(SALevelClearCount);
	if ((event->type() == QEvent::Resize && watched == d->ui.scrlLevels) ||
	    (event->type() == QEvent::Show && watched == d->ui.scrlLevels->verticalScrollBar()))
	{
		// QScrollArea resize event.
		d->scrollAreaResized(event);
	}

	// Allow event processing to continue.
	return false;
}
