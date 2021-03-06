/***************************************************************************
 * GameCube Memory Card Recovery Program [libsaveedit]                     *
 * PageFilterModel.cpp: Filter a QAbstractItemModel by pages.              *
 *                                                                         *
 * Copyright (c) 2015-2018 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#include "PageFilterModel.hpp"

// C includes. (C++ namespace)
#include <cstdlib>

/** PageFilterModelPrivate **/

class PageFilterModelPrivate
{
	public:
		explicit PageFilterModelPrivate(PageFilterModel *q);

	protected:
		PageFilterModel *const q_ptr;
		Q_DECLARE_PUBLIC(PageFilterModel)
	private:
		Q_DISABLE_COPY(PageFilterModelPrivate)

	public:
		int pageSize;

		// TODO: Assuming itemCount never changes.
		// Connect signals from sourceModel for row count changes.
		int currentPage;
		int pageCount;	// Total number of pages.
		int itemStart;	// Index of first item currently displayed.
		int itemEnd;	// Index of last item currently displayed.
		int itemCount;	// Number of items currently being displayed.

		/**
		 * Calculate the page offsets.
		 */
		void calcPageOffsets(void);
};

PageFilterModelPrivate::PageFilterModelPrivate(PageFilterModel *q)
	: q_ptr(q)
	, pageSize(64)
	, currentPage(0)
	, pageCount(0)
	, itemStart(0)
	, itemEnd(0)
	, itemCount(0)
{ }

/**
 * Calculate the page offsets.
 */
void PageFilterModelPrivate::calcPageOffsets(void)
{
	Q_Q(PageFilterModel);
	const int sourceRowCount = (q->sourceModel()
				? q->sourceModel()->rowCount()
				: 0);

	if (sourceRowCount == 0) {
		// No items available.
		currentPage = 0;	// TODO: Emit signal?
		pageCount = 0;		// TODO: Emit signal?
		itemStart = 0;
		itemEnd = 0;
		itemCount = 0;
	} else {
		// NOTE: pageCount can't be 0 here, since we
		// already checked for sourceRowCount == 0.

		// Calculate the total number of pages.
		int tmp_pageCount;
		if (pageSize > 0) {
			div_t pages_div = div(sourceRowCount, pageSize);
			tmp_pageCount = pages_div.quot;
			tmp_pageCount += (pages_div.rem > 0);
		} else {
			// Single page.
			tmp_pageCount = 1;
		}

		if (tmp_pageCount != pageCount) {
			// Page count has changed.
			if (currentPage >= tmp_pageCount) {
				// Current page will be out of range.
				// Fix the current page first.
				currentPage = tmp_pageCount - 1;
				emit q->currentPageChanged(currentPage);
			}
			pageCount = tmp_pageCount;
			emit q->pageCountChanged(pageCount);
		} else {
			// Validate the current page.
			if (currentPage >= pageCount) {
				// Current page is out of range.
				currentPage = pageCount - 1;
				emit q->currentPageChanged(currentPage);
			}
		}

		// Calculate the start and end indexes.
		if (pageSize > 0) {
			itemStart = pageSize * currentPage;
			itemEnd = itemStart + pageSize - 1;

			// Validate things.
			if (itemStart >= sourceRowCount)
				itemStart = sourceRowCount - 1;
			if (itemEnd >= sourceRowCount)
				itemEnd = sourceRowCount - 1;

			// Item count.
			itemCount = itemEnd - itemStart + 1;
		} else {
			// Single page.
			itemStart = 0;
			itemEnd = sourceRowCount - 1;
			itemCount = sourceRowCount;
		}
	}

	// TODO: Only invalidate the filter if the offsets changed.
	q->invalidateFilter();
}

/** PageFilterModel **/

PageFilterModel::PageFilterModel(QObject *parent)
	: super(parent)
	, d_ptr(new PageFilterModelPrivate(this))
{ }

PageFilterModel::~PageFilterModel()
{
	Q_D(PageFilterModel);
	delete d;
}

/** Qt Model/View interface. **/

bool PageFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
	Q_UNUSED(sourceParent)
	Q_D(const PageFilterModel);
	if (d->itemCount == 0)
		return false;
	if (sourceRow < d->itemStart || sourceRow > d->itemEnd)
		return false;
	return true;
}

void PageFilterModel::setSourceModel(QAbstractItemModel *sourceModel)
{
	Q_D(PageFilterModel);
	super::setSourceModel(sourceModel);
	d->calcPageOffsets();
}

/** Data access. **/

/**
 * Get the current page.
 * @return Current page.
 */
int PageFilterModel::currentPage(void) const
{
	Q_D(const PageFilterModel);
	return d->currentPage;
}

/**
 * Set the current page.
 * @param page Page to set as current.
 */
void PageFilterModel::setCurrentPage(int page)
{
	Q_D(PageFilterModel);
	if (d->currentPage == page)
		return;
	if (page < 0 || page >= d->pageCount)
		return;
	d->currentPage = page;
	d->calcPageOffsets();
	// NOTE: calcPageOffsets() may change the page number.
	// If it does, then it emitted currentPageChanged() itself.
	if (d->currentPage == page)
		emit currentPageChanged(page);
	
}

/**
 * Get the page size.
 * @return Page size.
 */
int PageFilterModel::pageSize(void) const
{
	Q_D(const PageFilterModel);
	return d->pageSize;
}

/**
 * Set the page size.
 * If 0, a single page will be used. (effectively a no-op)
 * @param pageSize Page size.
 */
void PageFilterModel::setPageSize(int pageSize)
{
	Q_D(PageFilterModel);
	if (d->pageSize == pageSize || pageSize < 0)
		return;
	d->pageSize = pageSize;
	d->calcPageOffsets();
}

/**
 * Get the page count.
 * @return Page count.
 */
int PageFilterModel::pageCount(void) const
{
	Q_D(const PageFilterModel);
	return d->pageCount;
}
