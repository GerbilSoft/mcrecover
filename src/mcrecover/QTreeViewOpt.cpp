/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * QTreeViewOpt.cpp: QTreeView with drawing optimizations.                 *
 * Specifically, don't update rows that are offscreen.			   *
 *                                                                         *
 * Copyright (c) 2013 by David Korth.                                      *
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

#include "QTreeViewOpt.hpp"

void QTreeViewOpt::dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
	bool propagateEvent = true;
	// TODO: Support for checking multiple items.
	if (topLeft == bottomRight) {
		// Single item. This might be an icon animation.
		// If it is, make sure the icon is onscreen.
		QRect itemRect = this->visualRect(topLeft);

		// Get the viewport rect.
		QRect viewportRect(QPoint(0, 0), this->viewport()->size());
		if (!viewportRect.contains(itemRect)) {
			// Item is NOT visible.
			// Don't propagate the event.
			propagateEvent = false;
		}
	}

	if (propagateEvent) {
		// Propagate the dataChanged() event.
		QTreeView::dataChanged(topLeft, bottomRight);
	}
}
