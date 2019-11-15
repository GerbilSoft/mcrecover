/***************************************************************************
 * GameCube Memory Card Recovery Program [libsaveedit]                     *
 * PageFilterModel.hpp: Filter a QAbstractItemModel by pages.              *
 *                                                                         *
 * Copyright (c) 2015-2018 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#ifndef __LIBSAVEEDIT_MODELS_PAGEFILTERMODEL_HPP__
#define __LIBSAVEEDIT_MODELS_PAGEFILTERMODEL_HPP__

// Qt includes.
#include <QSortFilterProxyModel>

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
		explicit PageFilterModel(QObject *parent);
		virtual ~PageFilterModel();

	protected:
		PageFilterModelPrivate *const d_ptr;
		Q_DECLARE_PRIVATE(PageFilterModel)
	private:
		Q_DISABLE_COPY(PageFilterModel)

	public:
		/** Qt Model/View interface. **/
		bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const final;
		void setSourceModel(QAbstractItemModel *sourceModel) final;

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

#endif /* __LIBSAVEEDIT_MODELS_PAGEFILTERMODEL_HPP__ */
