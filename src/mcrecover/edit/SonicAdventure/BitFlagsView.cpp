/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * BitFlagsView.hpp: Bit Flags editor.                                     *
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

#include "BitFlagsView.hpp"

#include "BitFlags.hpp"
#include "BitFlagsModel.hpp"
#include "PageFilterModel.hpp"

// C includes. (C++ namespace)
#include <cassert>

// Sonic Adventure save file definitions.
#include "sa_defs.h"

// TODO: Put this in a common header file somewhere.
#define NUM_ELEMENTS(x) ((int)(sizeof(x) / sizeof(x[0])))

/** BitFlagsViewPrivate **/

#include "ui_BitFlagsView.h"
class BitFlagsViewPrivate
{
	public:
		BitFlagsViewPrivate(BitFlagsView *q);

	protected:
		BitFlagsView *const q_ptr;
		Q_DECLARE_PUBLIC(BitFlagsView)
	private:
		Q_DISABLE_COPY(BitFlagsViewPrivate)

	public:
		Ui_BitFlagsView ui;

		// Page Filter model. (owned by this widget)
		PageFilterModel *pageFilterModel;
};

BitFlagsViewPrivate::BitFlagsViewPrivate(BitFlagsView *q)
	: q_ptr(q)
	, pageFilterModel(nullptr)
{
	// Initialize the page filter model.
	// TODO: Expose page and pageSize properties.
	pageFilterModel = new PageFilterModel(q);
	pageFilterModel->setPageSize(64);
}

/** BitFlagsView **/

BitFlagsView::BitFlagsView(QWidget *parent)
	: QWidget(parent)
	, d_ptr(new BitFlagsViewPrivate(this))
{
	Q_D(BitFlagsView);
	d->ui.setupUi(this);

	// Set lstEventFlags' model.
	d->ui.lstEventFlags->setModel(d->pageFilterModel);

	// NOTE: QTabBar is initialized after the model is set to prevent
	// signals from being triggered before pageFilterModel is valid.

	// Qt Designer doesn't have QTabBar, so we have to set it up manually.
	// Disable expanding so the tabs look like normal tabs.
	// Disable drawBase because the tabs are right above the QTreeView.
	// (FIXME: With drawBase disabled, there's a 1px empty space for unselected tabs on Win7...)
	// TODO: Retranslate the tabs on language change.
	// TODO: Parent should set the tab names...
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

	// Connect tabBar's signals.
	connect(d->ui.tabBar, SIGNAL(currentChanged(int)),
		d->pageFilterModel, SLOT(setCurrentPage(int)));
	connect(d->pageFilterModel, SIGNAL(currentPageChanged(int)),
		d->ui.tabBar, SLOT(setCurrentIndex(int)));
}

BitFlagsView::~BitFlagsView()
{
	Q_D(BitFlagsView);
	delete d;
}

/** Model access. **/

/**
 * Get the BitFlagsModel this widget is editing.
 * @return BitFlagsModel.
 */
BitFlagsModel *BitFlagsView::bitFlagsModel(void) const
{
	Q_D(const BitFlagsView);
	return qobject_cast<BitFlagsModel*>(d->pageFilterModel->sourceModel());
}

/**
 * Set the BitFlagsModel to edit.
 * @param bitFlagsModel BitFlagsModel.
 */
void BitFlagsView::setBitFlagsModel(BitFlagsModel *bitFlagsModel)
{
	// TODO: Connect destroyed() signal for BitFlagsModel?
	Q_D(BitFlagsView);
	d->pageFilterModel->setSourceModel(bitFlagsModel);
}

#if 0
/**
 * Load data from a Sonic Adventure save slot.
 * @param sa_save Sonic Adventure save slot.
 * The data must have already been byteswapped to host-endian.
 * @return 0 on success; non-zero on error.
 */
int BitFlagsView::load(const _sa_save_slot *sa_save)
{
	Q_D(BitFlagsView);
	const uint8_t *flagByte = &sa_save->events.all[0];
	assert(d->bitFlags.count() == (NUM_ELEMENTS(sa_save->events.all) * 8));

	uint8_t curByte = 0;
	for (int i = 0; i < d->bitFlags.count(); i++) {
		if (i % 8 == 0) {
			// New byte.
			curByte = *flagByte++;
		}

		// Set this flag.
		d->bitFlags.setFlag(i, (curByte & 0x01));
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
int BitFlagsView::save(_sa_save_slot *sa_save) const
{
	Q_D(const BitFlagsView);
	uint8_t *flagByte = &sa_save->events.all[0];
	assert(d->bitFlags.count() == (NUM_ELEMENTS(sa_save->events.all) * 8));

	// TODO: Verify that this is correct.
	for (int i = 0; i < d->bitFlags.count(); i++) {
		if (i % 8 == 0) {
			// New byte.
			// TODO: Optimize this?
			if (i > 0)
				flagByte++;
			*flagByte = 0;
		}

		// Get this flag.
		*flagByte <<= 1;
		*flagByte |= !!(d->bitFlags.flag(i));
	}

	return 0;
}
#endif
