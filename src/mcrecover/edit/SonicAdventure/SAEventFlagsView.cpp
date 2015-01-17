/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * SAEventFlagsView.cpp: Sonic Adventure - Event Flags editor.             *
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

#include "SAEventFlagsView.hpp"

#include "SAEventFlags.hpp"
#include "SAEventFlagsModel.hpp"
#include "PageFilterModel.hpp"

// C includes. (C++ namespace)
#include <cassert>

// Sonic Adventure save file definitions.
#include "sa_defs.h"

// TODO: Put this in a common header file somewhere.
#define NUM_ELEMENTS(x) ((int)(sizeof(x) / sizeof(x[0])))

/** SAEventFlagsViewPrivate **/

#include "ui_SAEventFlagsView.h"
class SAEventFlagsViewPrivate
{
	public:
		SAEventFlagsViewPrivate(SAEventFlagsView *q);

	protected:
		SAEventFlagsView *const q_ptr;
		Q_DECLARE_PUBLIC(SAEventFlagsView)
	private:
		Q_DISABLE_COPY(SAEventFlagsViewPrivate)

	public:
		Ui_SAEventFlagsView ui;

		// Event Flags model.
		SAEventFlagsModel *eventFlagsModel;
		PageFilterModel *pageFilterModel;

		// Event Flags data.
		SAEventFlags eventFlags;
};

SAEventFlagsViewPrivate::SAEventFlagsViewPrivate(SAEventFlagsView *q)
	: q_ptr(q)
{ }

/** SAEventFlagsView **/

SAEventFlagsView::SAEventFlagsView(QWidget *parent)
	: QWidget(parent)
	, d_ptr(new SAEventFlagsViewPrivate(this))
{
	Q_D(SAEventFlagsView);
	d->ui.setupUi(this);

	// Initialize the event flags.
	d->eventFlagsModel = new SAEventFlagsModel(this);
	d->eventFlagsModel->setEventFlags(&d->eventFlags);
	// Page filter.
	// TODO: Add page size to the constructor?
	d->pageFilterModel = new PageFilterModel(this);
	d->pageFilterModel->setSourceModel(d->eventFlagsModel);
	//d->pageFilterModel->setPageSize(64);
	d->ui.lstEventFlags->setModel(d->pageFilterModel);

	// NOTE: QTabBar is initialized after the model is set to prevent
	// signals from being triggered before pageFilterModel is valid.

	// Qt Designer doesn't have QTabBar, so we have to set it up manually.
	// Disable expanding so the tabs look like normal tabs.
	// Disable drawBase because the tabs are right above the QTreeView.
	// (FIXME: With drawBase disabled, there's a 1px empty space for unselected tabs on Win7...)
	// TODO: Retranslate the tabs on language change.
	d->ui.tabBar->setExpanding(false);
	d->ui.tabBar->setDrawBase(true);
	d->ui.tabBar->addTab(tr("Unused?"));
	d->ui.tabBar->addTab(tr("General"));
	d->ui.tabBar->addTab(tr("Sonic"));
	d->ui.tabBar->addTab(tr("Tails"));
	d->ui.tabBar->addTab(tr("Knuckles"));
	d->ui.tabBar->addTab(tr("Amy"));
	d->ui.tabBar->addTab(tr("Gamma"));
	d->ui.tabBar->addTab(tr("Big"));
}

SAEventFlagsView::~SAEventFlagsView()
{
	Q_D(SAEventFlagsView);
	delete d;
}

/** Data access. **/

/**
 * Load data from a Sonic Adventure save slot.
 * @param sa_save Sonic Adventure save slot.
 * The data must have already been byteswapped to host-endian.
 * @return 0 on success; non-zero on error.
 */
int SAEventFlagsView::load(const _sa_save_slot *sa_save)
{
	Q_D(SAEventFlagsView);
	const uint8_t *flagByte = &sa_save->events.all[0];
	assert(d->eventFlags.count() == (NUM_ELEMENTS(sa_save->events.all) * 8));

	uint8_t curByte = 0;
	for (int i = 0; i < d->eventFlags.count(); i++) {
		if (i % 8 == 0) {
			// New byte.
			curByte = *flagByte++;
		}

		// Set this flag.
		d->eventFlags.setFlag(i, (curByte & 0x01));
		curByte >>= 1;
	}

	return 0;
}

/**
 * Save data to a Sonic Adventure save slot.
 * @param sa_save Sonic Adventure save slot.
 * The data will be in host-endian format.
 * @return 0 on success; non-zero on error.
 */
int SAEventFlagsView::save(_sa_save_slot *sa_save) const
{
	Q_D(const SAEventFlagsView);
	uint8_t *flagByte = &sa_save->events.all[0];
	assert(d->eventFlags.count() == (NUM_ELEMENTS(sa_save->events.all) * 8));

	// TODO: Verify that this is correct.
	for (int i = 0; i < d->eventFlags.count(); i++) {
		if (i % 8 == 0) {
			// New byte.
			// TODO: Optimize this?
			if (i > 0)
				flagByte++;
			*flagByte = 0;
		}

		// Get this flag.
		*flagByte <<= 1;
		*flagByte |= !!(d->eventFlags.flag(i));
	}

	return 0;
}

/** UI widget slots. **/

/**
 * Current tab has changed.
 * @param index Tab index.
 */
void SAEventFlagsView::on_tabBar_currentChanged(int index)
{
	Q_D(SAEventFlagsView);
	d->pageFilterModel->setCurrentPage(index);
}
