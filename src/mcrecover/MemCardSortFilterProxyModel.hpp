/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * MemCardSortFilterProxyModel.hpp: MemCardModel sort filter proxy.        *
 *                                                                         *
 * Copyright (c) 2012-2014 by David Korth.                                 *
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

#ifndef __MCRECOVER_MEMCARDSORTFILTERPROXYMODEL_HPP__
#define __MCRECOVER_MEMCARDSORTFILTERPROXYMODEL_HPP__

#include <QtGui/QSortFilterProxyModel>

class MemCardSortFilterProxyModel : public QSortFilterProxyModel
{
	Q_OBJECT

	public:
		MemCardSortFilterProxyModel(QObject *parent = 0);

	private:
		Q_DISABLE_COPY(MemCardSortFilterProxyModel)

	public:
		virtual bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
		virtual bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;
};

#endif /* __MCRECOVER_MEMCARDSORTFILTERPROXYMODEL_HPP__ */
