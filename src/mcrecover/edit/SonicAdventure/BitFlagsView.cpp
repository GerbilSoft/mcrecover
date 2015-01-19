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

		// Default page size.
		static const int defaultPageSize = 64;

		/**
		 * Update the QTabBar.
		 */
		void updateTabBar(void);
};

BitFlagsViewPrivate::BitFlagsViewPrivate(BitFlagsView *q)
	: q_ptr(q)
	, pageFilterModel(nullptr)
{
	// Initialize the page filter model.
	pageFilterModel = new PageFilterModel(q);
	pageFilterModel->setPageSize(defaultPageSize);
}

/**
 * Update the QTabBar.
 */
void BitFlagsViewPrivate::updateTabBar(void)
{
	// TODO: Add/remove tabs as necessary.
	// For now, just hide the entire tab bar if it's a single page.
	ui.tabBar->setVisible(pageFilterModel->pageCount() > 1);
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

	// Update the QTabBar.
	d->updateTabBar();

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

	// Update the QTabBar.
	d->updateTabBar();
}

/** Data access. **/

/**
 * Get the page size.
 * @return Page size.
 */
int BitFlagsView::pageSize(void) const
{
	Q_D(const BitFlagsView);
	return d->pageFilterModel->pageSize();
}

/**
 * Set the page size.
 * @param pageSize Page size.
 */
void BitFlagsView::setPageSize(int pageSize)
{
	Q_D(BitFlagsView);
	// TODO: Signal from pageFilterModel to adjust tabs?
	d->pageFilterModel->setPageSize(pageSize);

	// Update the QTabBar.
	d->updateTabBar();
}

// TODO: Page count?
// TODO: Set tab names.

#if 0
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
