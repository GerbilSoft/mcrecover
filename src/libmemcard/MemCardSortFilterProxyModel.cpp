/***************************************************************************
 * GameCube Memory Card Recovery Program [libmemcard]                      *
 * MemCardSortFilterProxyModel.hpp: MemCardModel sort filter proxy.        *
 *                                                                         *
 * Copyright (c) 2012-2018 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#include "MemCardSortFilterProxyModel.hpp"

MemCardSortFilterProxyModel::MemCardSortFilterProxyModel(QObject *parent)
	: super(parent)
{ }

bool MemCardSortFilterProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
	return super::filterAcceptsRow(source_row, source_parent);
}

bool MemCardSortFilterProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
	if (!left.isValid() || !right.isValid()) {
		// One or both indexes are invalid.
		// Use the default lessThan().
		return super::lessThan(left, right);
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
	return super::lessThan(left, right);
}
