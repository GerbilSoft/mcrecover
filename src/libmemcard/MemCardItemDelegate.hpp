/***************************************************************************
 * GameCube Memory Card Recovery Program [libmemcard]                      *
 * MemCardItemDelegate.hpp: MemCard item delegate for QTreeView.           *
 *                                                                         *
 * Copyright (c) 2013-2018 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
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
