/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * QTreeViewOpt.hpp: QTreeView with drawing optimizations.                 *
 * Specifically, don't update rows that are offscreen.                     *
 *                                                                         *
 * Copyright (c) 2013-2018 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
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
		void dataChanged(const QModelIndex &topLeft,
			const QModelIndex &bottomRight,
			const QVector<int> &roles = QVector<int>()) final;

	protected slots:
		void showColumnContextMenu(const QPoint &point);

	/** Shh... it's a secret to everybody. **/
	protected:
		void keyPressEvent(QKeyEvent *event) final;
		void focusOutEvent(QFocusEvent *event) final;
	signals:
		void keyPress(QKeyEvent *event);
		void focusOut(QFocusEvent *event);
};

#endif /* __MCRECOVER_QTREEVIEWOPT_HPP__ */
