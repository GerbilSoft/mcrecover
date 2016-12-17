/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * QTreeViewOpt.hpp: QTreeView with drawing optimizations.                 *
 * Specifically, don't update rows that are offscreen.			   *
 *                                                                         *
 * Copyright (c) 2013-2016 by David Korth.                                 *
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

#ifndef __MCRECOVER_QTREEVIEWOPT_HPP__
#define __MCRECOVER_QTREEVIEWOPT_HPP__

// Qt includes and classes.
#include <QTreeView>
class QKeyEvent;
class QFocusEvent;

class QTreeViewOpt : public QTreeView
{
	Q_OBJECT
	typedef QTreeView super;

	public:
		explicit QTreeViewOpt(QWidget *parent = 0);

	private:
		Q_DISABLE_COPY(QTreeViewOpt);

	public:
		virtual void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);

	protected slots:
		void showColumnContextMenu(const QPoint &point);

	/** Shh... it's a secret to everybody. **/
	protected:
		virtual void keyPressEvent(QKeyEvent *event) override;
		virtual void focusOutEvent(QFocusEvent *event) override;
	signals:
		void keyPress(QKeyEvent *event);
		void focusOut(QFocusEvent *event);
};

#endif /* __MCRECOVER_QTREEVIEWOPT_HPP__ */
