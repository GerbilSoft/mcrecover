/***************************************************************************
 * GameCube Memory Card Recovery Program [libsaveedit]                     *
 * BitFlagsView.hpp: Bit Flags editor.                                     *
 *                                                                         *
 * Copyright (c) 2015-2025 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#include "BitFlagsView.hpp"

#include "../models/BitFlags.hpp"
#include "../models/BitFlagsModel.hpp"
#include "../models/PageFilterModel.hpp"

// C includes. (C++ namespace)
#include <cassert>

// TODO: Put this in a common header file somewhere.
#define NUM_ELEMENTS(x) ((int)(sizeof(x) / sizeof(x[0])))

/** BitFlagsViewPrivate **/

#include "ui_BitFlagsView.h"
class BitFlagsViewPrivate
{
public:
	explicit BitFlagsViewPrivate(BitFlagsView *q);

protected:
	BitFlagsView *const q_ptr;
	Q_DECLARE_PUBLIC(BitFlagsView)
private:
	Q_DISABLE_COPY(BitFlagsViewPrivate)

public:
	Ui_BitFlagsView ui;

	// Page Filter model (owned by this widget)
	PageFilterModel *pageFilterModel;

	// Default page size.
	static constexpr int defaultPageSize = 64;

	/**
	 * Update the display.
	 */
	void updateDisplay(void);

	/**
	 * Update the tab bar.
	 * @param forceTextUpdate If true, update all tab text. Needed for language changes.
	 */
	void updateTabBar(bool forceTextUpdate = false);
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
 * Update the display.
 */
void BitFlagsViewPrivate::updateDisplay(void)
{
	QAbstractItemModel *const model = pageFilterModel->sourceModel();
	if (!model) {
		// No model is set.
		// TODO: Keep tabs visible?
		ui.tabBar->setVisible(false);
		return;
	}

	// Update the tab bar.
	updateTabBar();

	// Resize the columns to fit the contents.
	// TODO: On theme change, pageSize change...?
	ui.lstEventFlags->resizeColumnToContents(BitFlagsModel::COL_CHECKBOX);

	// ID should be as wide as the largest ID number.
	QFontMetrics fm = ui.lstEventFlags->fontMetrics();
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
	int id_width = fm.horizontalAdvance(QString::number(model->rowCount()-1));
	// FIXME: Add text margins. For now, just add width of 'W'.
	id_width += fm.horizontalAdvance(QChar(L'W'));
#else /* QT_VERSION < QT_VERSION_CHECK(5, 11, 0) */
	int id_width = fm.width(QString::number(model->rowCount()-1));
	// FIXME: Add text margins. For now, just add width of 'W'.
	id_width += fm.width(QChar(L'W'));
#endif /* QT_VERSION >= QT_VERSION_CHECK(5, 11, 0) */

	ui.lstEventFlags->setColumnWidth(BitFlagsModel::COL_ID, id_width+1);
	
	// Event Description and overall width.
	ui.lstEventFlags->resizeColumnToContents(BitFlagsModel::COL_DESCRIPTION);
	ui.lstEventFlags->resizeColumnToContents(model->columnCount());
}

/**
 * Update the tab bar.
 * @param forceTextUpdate If true, update all tab text. Needed for language changes.
 */
void BitFlagsViewPrivate::updateTabBar(bool forceTextUpdate)
{
	// Add/remove tabs as necessary.
	const BitFlagsModel *model = qobject_cast<const BitFlagsModel*>(pageFilterModel->sourceModel());
	assert(model != nullptr);
	if (!model)
		return;

	const int oldPages = ui.tabBar->count();
	const int newPages = pageFilterModel->pageCount();

	if (newPages < oldPages) {
		// Remove some tabs.
		if (ui.tabBar->currentIndex() >= newPages) {
			// Update the current tab first.
			ui.tabBar->setCurrentIndex(newPages - 1);
		}
		for (int i = newPages-1; i >= oldPages; i--) {
			ui.tabBar->removeTab(i);
		}
	} else if (newPages > oldPages) {
		// Add some tabs.
		for (int i = oldPages; i < newPages; i++) {
			ui.tabBar->addTab(model->pageName(i));
		}
	}

	if (forceTextUpdate) {
		// Update all tabs' titles.
		for (int i = qMin(oldPages, newPages); i >= 0; i--) {
			ui.tabBar->setTabText(i, model->pageName(i));
		}
	}

	// Hide the tab bar if there's only one page.
	ui.tabBar->setVisible(pageFilterModel->pageCount() > 1);
}

/** BitFlagsView **/

BitFlagsView::BitFlagsView(QWidget *parent)
	: super(parent)
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

	// Update the display.
	d->updateDisplay();

	// Connect tabBar's signals.
	connect(d->ui.tabBar, &QTabBar::currentChanged,
		d->pageFilterModel, &PageFilterModel::setCurrentPage);
	connect(d->pageFilterModel, &PageFilterModel::currentPageChanged,
		d->ui.tabBar, &QTabBar::setCurrentIndex);
}

BitFlagsView::~BitFlagsView()
{
	delete d_ptr;
}

/** Events **/

/**
 * Widget state has changed.
 * @param event State change event
 */
void BitFlagsView::changeEvent(QEvent *event)
{
	if (event->type() == QEvent::LanguageChange) {
		// Retranslate the UI.
		Q_D(BitFlagsView);
		d->ui.retranslateUi(this);

		// Update the tab titles.
		d->updateTabBar(true);
	}

	// Pass the event to the base class.
	super::changeEvent(event);
}

/** Model access **/

/**
 * Get the BitFlagsModel this widget is editing.
 * @return BitFlagsModel
 */
BitFlagsModel *BitFlagsView::bitFlagsModel(void) const
{
	Q_D(const BitFlagsView);
	return qobject_cast<BitFlagsModel*>(d->pageFilterModel->sourceModel());
}

/**
 * Set the BitFlagsModel to edit.
 * @param bitFlagsModel BitFlagsModel
 */
void BitFlagsView::setBitFlagsModel(BitFlagsModel *bitFlagsModel)
{
	// TODO: Connect destroyed() signal for BitFlagsModel?
	// TODO: Watch for row count changes to adjust pages?
	Q_D(BitFlagsView);
	d->pageFilterModel->setSourceModel(bitFlagsModel);
	// TODO: Signal from pageFilterModel to adjust tabs?
	d->pageFilterModel->setPageSize(bitFlagsModel->pageSize());

	// Update the QTabBar.
	d->updateDisplay();
}

/** Data access **/

/**
 * Get the page size.
 * @return Page size
 */
int BitFlagsView::pageSize(void) const
{
	Q_D(const BitFlagsView);
	return d->pageFilterModel->pageSize();
}

// TODO: Page count?
