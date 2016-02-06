/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * PageFilterModel.hpp: Filter a QAbstractItemModel by pages.              *
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

#ifndef __MCRECOVER_EDIT_MODELS_PAGEFILTERMODEL_HPP__
#define __MCRECOVER_EDIT_MODELS_PAGEFILTERMODEL_HPP__

// Qt includes.
#include <QtGui/QSortFilterProxyModel>

class PageFilterModelPrivate;
class PageFilterModel : public QSortFilterProxyModel
{
	Q_OBJECT
	typedef QSortFilterProxyModel super;

	Q_PROPERTY(int currentPage READ currentPage WRITE setCurrentPage NOTIFY currentPageChanged)
	// TODO: Add signal for pageSizeChanged()
	Q_PROPERTY(int pageSize READ pageSize WRITE setPageSize)
	Q_PROPERTY(int pageCount READ pageCount NOTIFY pageCountChanged STORED false)

	public:
		PageFilterModel(QObject *parent);
		virtual ~PageFilterModel();

	protected:
		PageFilterModelPrivate *const d_ptr;
		Q_DECLARE_PRIVATE(PageFilterModel)
	private:
		Q_DISABLE_COPY(PageFilterModel)

	public:
		/** Qt Model/View interface. **/
		virtual bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
		virtual void setSourceModel(QAbstractItemModel *sourceModel) override;

	signals:
		/**
		 * Current page has changed.
		 * @param page Current page.
		 */
		void currentPageChanged(int page);

		/**
		 * Page count has changed.
		 * @param pageCount New page count.
		 */
		void pageCountChanged(int pageCount);

	public:
		/** Data access. **/

		/**
		 * Get the current page.
		 * @return Current page.
		 */
		int currentPage(void) const;

		/**
		 * Get the page size.
		 * @return Page size.
		 */
		int pageSize(void) const;

		/**
		 * Set the page size.
		 * If 0, a single page will be used. (effectively a no-op)
		 * @param pageSize Page size.
		 */
		void setPageSize(int pageSize);

		/**
		 * Get the page count.
		 * @return Page count.
		 */
		int pageCount(void) const;

	public slots:
		/**
		 * Set the current page.
		 * @param page Page to set as current.
		 */
		void setCurrentPage(int page);
};

#endif /* __MCRECOVER_EDIT_MODELS_PAGEFILTERMODEL_HPP__ */
