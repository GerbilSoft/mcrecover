/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * PageFilterModel.cpp: Filter a QAbstractItemModel by pages.              *
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

// TODO: Move to edit/common/?
#include "PageFilterModel.hpp"

/** PageFilterModelPrivate **/

class PageFilterModelPrivate
{
	public:
		PageFilterModelPrivate(PageFilterModel *q);

	protected:
		PageFilterModel *const q_ptr;
		Q_DECLARE_PUBLIC(PageFilterModel)
	private:
		Q_DISABLE_COPY(PageFilterModelPrivate)

	public:
		int pageSize;

		int currentPage;
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

	if (sourceRowCount == 0 || (currentPage * pageSize) >= sourceRowCount) {
		// No items available.
		itemStart = 0;
		itemEnd = 0;
		itemCount = 0;
	} else {
		itemStart = pageSize * currentPage;
		itemEnd = itemStart + pageSize - 1;

		// Validate things.
		if (itemStart >= sourceRowCount)
			itemStart = sourceRowCount - 1;
		if (itemEnd >= sourceRowCount)
			itemEnd = sourceRowCount - 1;

		// Item count.
		itemCount = itemEnd - itemStart + 1;
	}

	// TODO: Only invalidate the filter if the offsets changed.
	q->invalidateFilter();
}

/** PageFilterModel **/

PageFilterModel::PageFilterModel(QObject *parent)
	: QSortFilterProxyModel(parent)
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
	QSortFilterProxyModel::setSourceModel(sourceModel);
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
	// TODO: Calculate page count.
	if (page < 0 /*|| page > maxPages*/)
		return;
	d->currentPage = page;
	d->calcPageOffsets();
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
 * @param pageSize Page size.
 */
void PageFilterModel::setPageSize(int pageSize)
{
	Q_D(PageFilterModel);
	if (d->pageSize == pageSize)
		return;
	d->pageSize = pageSize;
	d->calcPageOffsets();
}
