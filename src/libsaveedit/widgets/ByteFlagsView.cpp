/***************************************************************************
 * GameCube Memory Card Recovery Program [libsaveedit]                     *
 * ByteFlagsView.hpp: Byte Flags editor.                                   *
 *                                                                         *
 * Copyright (c) 2015-2025 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#include "ByteFlagsView.hpp"

#include "../models/ByteFlags.hpp"
#include "../models/ByteFlagsModel.hpp"
#include "../models/PageFilterModel.hpp"
#include "CenteredCheckBoxDelegate.hpp"

// C includes. (C++ namespace)
#include <cassert>

// TODO: Put this in a common header file somewhere.
#define NUM_ELEMENTS(x) ((int)(sizeof(x) / sizeof(x[0])))

/** ByteFlagsViewPrivate **/

#include "ui_ByteFlagsView.h"
class ByteFlagsViewPrivate
{
public:
	explicit ByteFlagsViewPrivate(ByteFlagsView *q);

protected:
	ByteFlagsView *const q_ptr;
	Q_DECLARE_PUBLIC(ByteFlagsView)
private:
	Q_DISABLE_COPY(ByteFlagsViewPrivate)

public:
	Ui_ByteFlagsView ui;

	// Page Filter model (owned by this widget)
	PageFilterModel *pageFilterModel;

	// Default page size
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

ByteFlagsViewPrivate::ByteFlagsViewPrivate(ByteFlagsView *q)
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
void ByteFlagsViewPrivate::updateDisplay(void)
{
	QAbstractItemModel *const model = pageFilterModel->sourceModel();
	if (!model) {
		// No model is set.
		// TODO: Keep tabs visible?
		ui.tabBar->setVisible(false);
		return;
	}

	// Resize the columns to fit the contents.
	// TODO: On theme change, pageSize change...?
	ui.lstEventFlags->resizeColumnToContents(ByteFlagsModel::COL_CHARACTER);
	for (int i = ByteFlagsModel::COL_BIT0; i < ByteFlagsModel::COL_BIT7; i++) {
		ui.lstEventFlags->resizeColumnToContents(i);
	}

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

	ui.lstEventFlags->setColumnWidth(ByteFlagsModel::COL_ID, id_width+1);
}

/**
 * Update the tab bar.
 * @param forceTextUpdate If true, update all tab text. Needed for language changes.
 */
void ByteFlagsViewPrivate::updateTabBar(bool forceTextUpdate)
{
	// Add/remove tabs as necessary.
	const ByteFlagsModel *model = qobject_cast<const ByteFlagsModel*>(pageFilterModel->sourceModel());
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

/** ByteFlagsView **/

ByteFlagsView::ByteFlagsView(QWidget *parent)
	: super(parent)
	, d_ptr(new ByteFlagsViewPrivate(this))
{
	Q_D(ByteFlagsView);
	d->ui.setupUi(this);

	// Set lstEventFlags' model.
	d->ui.lstEventFlags->setModel(d->pageFilterModel);

	// Set up a CentereCheckBoxDelegate.
	d->ui.lstEventFlags->setItemDelegate(new CenteredCheckBoxDelegate(this));

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

ByteFlagsView::~ByteFlagsView()
{
	delete d_ptr;
}

/** Events **/

/**
 * Widget state has changed.
 * @param event State change event
 */
void ByteFlagsView::changeEvent(QEvent *event)
{
	if (event->type() == QEvent::LanguageChange) {
		// Retranslate the UI.
		Q_D(ByteFlagsView);
		d->ui.retranslateUi(this);

		// Update the tab titles.
		d->updateTabBar(true);
	}

	// Pass the event to the base class.
	super::changeEvent(event);
}

/** Model access **/

/**
 * Get the ByteFlagsModel this widget is editing.
 * @return ByteFlagsModel
 */
ByteFlagsModel *ByteFlagsView::byteFlagsModel(void) const
{
	Q_D(const ByteFlagsView);
	return qobject_cast<ByteFlagsModel*>(d->pageFilterModel->sourceModel());
}

/**
 * Set the ByteFlagsModel to edit.
 * @param byteFlagsModel ByteFlagsModel
 */
void ByteFlagsView::setByteFlagsModel(ByteFlagsModel *byteFlagsModel)
{
	// TODO: Connect destroyed() signal for ByteFlagsModel?
	// TODO: Watch for row count changes to adjust pages?
	Q_D(ByteFlagsView);
	d->pageFilterModel->setSourceModel(byteFlagsModel);
	// TODO: Signal from pageFilterModel to adjust tabs?
	d->pageFilterModel->setPageSize(byteFlagsModel->pageSize());

	// Hide undefined bits.
	// TODO: When byteFlagsModel's byteFlags changes?
	const ByteFlags *byteFlags = byteFlagsModel->byteFlags();
	for (int i = 0; i < 8; i++) {
		const bool isHidden = byteFlags->flagType(i).isEmpty();
		const int col = (ByteFlagsModel::COL_BIT0 + i);
		d->ui.lstEventFlags->setColumnHidden(col, isHidden);
		d->ui.lstEventFlags->resizeColumnToContents(col);
	}

	// Update the QTabBar.
	d->updateDisplay();
}

/** Data access **/

/**
 * Get the page size.
 * @return Page size
 */
int ByteFlagsView::pageSize(void) const
{
	Q_D(const ByteFlagsView);
	return d->pageFilterModel->pageSize();
}

// TODO: Page count?
