/***************************************************************************
 * GameCube Memory Card Recovery Program [libmemcard]                      *
 * MemCardSortFilterProxyModel.hpp: MemCardModel sort filter proxy.        *
 *                                                                         *
 * Copyright (c) 2012-2018 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#ifndef __MCRECOVER_MEMCARDSORTFILTERPROXYMODEL_HPP__
#define __MCRECOVER_MEMCARDSORTFILTERPROXYMODEL_HPP__

#include <QSortFilterProxyModel>

class MemCardSortFilterProxyModel : public QSortFilterProxyModel
{
	Q_OBJECT
	typedef QSortFilterProxyModel super;

	public:
		explicit MemCardSortFilterProxyModel(QObject *parent = 0);

	private:
		Q_DISABLE_COPY(MemCardSortFilterProxyModel)

	public:
		bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const final;
		bool lessThan(const QModelIndex &left, const QModelIndex &right) const final;
};

#endif /* __MCRECOVER_MEMCARDSORTFILTERPROXYMODEL_HPP__ */
