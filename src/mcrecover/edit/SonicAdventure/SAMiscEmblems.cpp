/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * SAMiscEmblems.cpp: Sonic Adventure - Miscellaneous Emblems editor.      *
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

#include "SAMiscEmblems.hpp"

// Qt includes.
#include <QtCore/QSignalMapper>

// Qt widgets.
#include <QtGui/QLabel>
#include <QtGui/QSpinBox>
#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>

// C includes. (C++ namespace)
#include <cstring>

// Sonic Adventure save file definitions.
#include "sa_defs.h"

// Common data.
#include "SAData.h"

// TODO: Put this in a common header file somewhere.
#define NUM_ELEMENTS(x) ((int)(sizeof(x) / sizeof(x[0])))

/** SAMiscEmblemsPrivate **/

#include "ui_SAMiscEmblems.h"
class SAMiscEmblemsPrivate
{
	public:
		SAMiscEmblemsPrivate(SAMiscEmblems *q);
		~SAMiscEmblemsPrivate();

	protected:
		SAMiscEmblems *const q_ptr;
		Q_DECLARE_PUBLIC(SAMiscEmblems)
	private:
		Q_DISABLE_COPY(SAMiscEmblemsPrivate)

	public:
		Ui_SAMiscEmblems ui;

		// Chao Race emblems.
		QCheckBox *chkChaoRace[5];
		// Adventure Field emblems.
		QCheckBox *chkAdvField[12];

		/**
		 * Initialize widgets.
		 */
		void initWidgets(void);
};

SAMiscEmblemsPrivate::SAMiscEmblemsPrivate(SAMiscEmblems *q)
	: q_ptr(q)
{ }

SAMiscEmblemsPrivate::~SAMiscEmblemsPrivate()
{
	// TODO: Is this needed?
	// The widgets are owned by this widget, so they
	// should be automatically deleted...
	for (int i = NUM_ELEMENTS(chkChaoRace)-1; i >= 0; i--) {
		delete chkChaoRace[i];
	}
	for (int i = NUM_ELEMENTS(chkAdvField)-1; i >= 0; i--) {
		delete chkAdvField[i];
	}
}

/**
 * Initialize widgets.
 */
void SAMiscEmblemsPrivate::initWidgets(void)
{
	// Create widgets for the emblems.
	// TODO: Top or VCenter?
	// TODO: Scroll area is screwing with minimum width...
	Q_Q(SAMiscEmblems);
	QString qsCssCheckBox = QLatin1String(sa_ui_css_emblem_checkbox_large);

	// Chao Race emblems.
	for (int i = 0; i < NUM_ELEMENTS(chkChaoRace); i++) {
		chkChaoRace[i] = new QCheckBox(q);
		chkChaoRace[i]->setStyleSheet(qsCssCheckBox);
		chkChaoRace[i]->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
		ui.hboxChaoRace->addWidget(chkChaoRace[i], 0, Qt::AlignCenter);
	}

	// Adventure Field emblems.
	for (int i = 0; i < 12; i++) {
		// TODO: Adventure Field names.
		chkAdvField[i] = new QCheckBox(q);
		chkAdvField[i]->setStyleSheet(qsCssCheckBox);
		chkAdvField[i]->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
		ui.gridAdventureFields->addWidget(chkAdvField[i], i/4, i%4+1, Qt::AlignCenter);
	}
}

/** SAMiscEmblems **/

SAMiscEmblems::SAMiscEmblems(QWidget *parent)
	: super(parent)
	, d_ptr(new SAMiscEmblemsPrivate(this))
{
	Q_D(SAMiscEmblems);
	d->ui.setupUi(this);

	// Initialize the widgets.
	d->initWidgets();
}

SAMiscEmblems::~SAMiscEmblems()
{
	Q_D(SAMiscEmblems);
	delete d;
}

/** Events. **/

/**
 * Widget state has changed.
 * @param event State change event.
 */
void SAMiscEmblems::changeEvent(QEvent *event)
{
	if (event->type() == QEvent::LanguageChange) {
		// Retranslate the UI.
		Q_D(SAMiscEmblems);
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
int SAMiscEmblems::load(const sa_save_slot *sa_save)
{
	Q_D(SAMiscEmblems);

	// Chao Race emblems.
	d->chkChaoRace[0]->setChecked(SA_TEST_EMBLEM(sa_save->emblems, 106));
	d->chkChaoRace[1]->setChecked(SA_TEST_EMBLEM(sa_save->emblems, 107));
	d->chkChaoRace[2]->setChecked(SA_TEST_EMBLEM(sa_save->emblems, 108));
	d->chkChaoRace[3]->setChecked(SA_TEST_EMBLEM(sa_save->emblems, 109));
	d->chkChaoRace[4]->setChecked(SA_TEST_EMBLEM(sa_save->emblems, 110));

	// Adventure Field emblems.
	d->chkAdvField[ 0]->setChecked(SA_TEST_EMBLEM(sa_save->emblems, 118));
	d->chkAdvField[ 1]->setChecked(SA_TEST_EMBLEM(sa_save->emblems, 119));
	d->chkAdvField[ 2]->setChecked(SA_TEST_EMBLEM(sa_save->emblems, 120));
	d->chkAdvField[ 3]->setChecked(SA_TEST_EMBLEM(sa_save->emblems, 121));
	d->chkAdvField[ 4]->setChecked(SA_TEST_EMBLEM(sa_save->emblems, 122));
	d->chkAdvField[ 5]->setChecked(SA_TEST_EMBLEM(sa_save->emblems, 123));
	d->chkAdvField[ 6]->setChecked(SA_TEST_EMBLEM(sa_save->emblems, 124));
	d->chkAdvField[ 7]->setChecked(SA_TEST_EMBLEM(sa_save->emblems, 125));
	d->chkAdvField[ 8]->setChecked(SA_TEST_EMBLEM(sa_save->emblems, 126));
	d->chkAdvField[ 9]->setChecked(SA_TEST_EMBLEM(sa_save->emblems, 127));
	d->chkAdvField[10]->setChecked(SA_TEST_EMBLEM(sa_save->emblems, 128));
	d->chkAdvField[11]->setChecked(SA_TEST_EMBLEM(sa_save->emblems, 129));

	return 0;
}

/**
 * Save data to a Sonic Adventure save slot.
 * @param sa_save Sonic Adventure save slot.
 * The data will be in host-endian format.
 * @return 0 on success; non-zero on error.
 */
int SAMiscEmblems::save(sa_save_slot *sa_save) const
{
	Q_D(const SAMiscEmblems);

	// Chao Race emblems.
	// TODO: Helper function to get emblem by index.
	sa_save->emblems[13] &= ~0x7C;
	sa_save->emblems[13] |= (d->chkChaoRace[0]->isChecked() ? 0x04 : 0x00);
	sa_save->emblems[13] |= (d->chkChaoRace[1]->isChecked() ? 0x08 : 0x00);
	sa_save->emblems[13] |= (d->chkChaoRace[2]->isChecked() ? 0x10 : 0x00);
	sa_save->emblems[13] |= (d->chkChaoRace[3]->isChecked() ? 0x20 : 0x00);
	sa_save->emblems[13] |= (d->chkChaoRace[4]->isChecked() ? 0x40 : 0x00);

	// Adventure Field emblems.
	// TODO: Helper function to get emblem by index.
	sa_save->emblems[14] &= ~0xC0;
	sa_save->emblems[14] |= (d->chkAdvField[ 0]->isChecked() ? 0x40 : 0x00);
	sa_save->emblems[14] |= (d->chkAdvField[ 1]->isChecked() ? 0x80 : 0x00);
	sa_save->emblems[15] = 0;
	sa_save->emblems[15] |= (d->chkAdvField[ 2]->isChecked() ? 0x01 : 0x00);
	sa_save->emblems[15] |= (d->chkAdvField[ 3]->isChecked() ? 0x02 : 0x00);
	sa_save->emblems[15] |= (d->chkAdvField[ 4]->isChecked() ? 0x04 : 0x00);
	sa_save->emblems[15] |= (d->chkAdvField[ 5]->isChecked() ? 0x08 : 0x00);
	sa_save->emblems[15] |= (d->chkAdvField[ 6]->isChecked() ? 0x10 : 0x00);
	sa_save->emblems[15] |= (d->chkAdvField[ 7]->isChecked() ? 0x20 : 0x00);
	sa_save->emblems[15] |= (d->chkAdvField[ 8]->isChecked() ? 0x40 : 0x00);
	sa_save->emblems[15] |= (d->chkAdvField[ 9]->isChecked() ? 0x80 : 0x00);
	sa_save->emblems[16] &= ~0x03;
	sa_save->emblems[16] |= (d->chkAdvField[10]->isChecked() ? 0x01 : 0x00);
	sa_save->emblems[16] |= (d->chkAdvField[11]->isChecked() ? 0x02 : 0x00);

	return 0;
}
