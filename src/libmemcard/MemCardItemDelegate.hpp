/***************************************************************************
 * GameCube Memory Card Recovery Program [libmemcard]                      *
 * MemCardItemDelegate.hpp: MemCard item delegate for QTreeView.           *
 *                                                                         *
 * Copyright (c) 2013-2018 by David Korth.                                 *
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
 * You should have received a copy of the GNU General Public License       *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.   *
 ***************************************************************************/

#ifndef __MCRECOVER_MEMCARDITEMDELEGATE_HPP__
#define __MCRECOVER_MEMCARDITEMDELEGATE_HPP__

// Qt includes.
#include <QStyledItemDelegate>

class MemCardItemDelegatePrivate;
class MemCardItemDelegate : public QStyledItemDelegate
{
	Q_OBJECT
	typedef QStyledItemDelegate super;

	public:
		explicit MemCardItemDelegate(QObject *parent);
		virtual ~MemCardItemDelegate();

	protected:
		MemCardItemDelegatePrivate *const d_ptr;
		Q_DECLARE_PRIVATE(MemCardItemDelegate)
	private:
		Q_DISABLE_COPY(MemCardItemDelegate)

	public:
		void paint(QPainter *painter, const QStyleOptionViewItem &option,
			   const QModelIndex &index) const final;
		QSize sizeHint(const QStyleOptionViewItem &option,
			       const QModelIndex &index) const final;

	private slots:
		/**
		 * The system theme has changed.
		 */
		void themeChanged_slot(void);
};

#endif /* __MCRECOVER_MEMCARDITEMDELEGATE_HPP__ */
