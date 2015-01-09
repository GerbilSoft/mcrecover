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

#include "MemCardSortFilterProxyModel.hpp"

MemCardSortFilterProxyModel::MemCardSortFilterProxyModel(QObject *parent)
	: QSortFilterProxyModel(parent)
{ }

bool MemCardSortFilterProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
	return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
}

bool MemCardSortFilterProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
	if (!left.isValid() || !right.isValid()) {
		// One or both indexes are invalid.
		// Use the default lessThan().
		return QSortFilterProxyModel::lessThan(left, right);
	}

	const QVariant vLeft = left.data();
	const QVariant vRight = right.data();

	// FIXME: Verify that this works with embedded null characters (L'\0'),
	// which is used to separate GameDesc from FileDesc in a single QString.
	if (vLeft.type() == QVariant::String &&
	    vRight.type() == QVariant::String)
	{
		// String. Do a case-insensitive comparison.
		QString sLeft = vLeft.toString();
		QString sRight = vRight.toString();
		return (sLeft.compare(sRight, Qt::CaseInsensitive) < 0);
	}

	// Unhandled type.
	// Use the default lessThan().
	return QSortFilterProxyModel::lessThan(left, right);
}
