/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * MemCardItemDelegate.cpp: MemCard item delegate for QListView.           *
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

#include "MemCardItemDelegate.hpp"

#include "MemCardModel.hpp"

// Qt includes.
#include <QtGui/QPainter>
#include <QtGui/QApplication>
#include <QtGui/QFont>
#include <QtGui/QFontMetrics>
#include <QtGui/QStyle>

MemCardItemDelegate::MemCardItemDelegate(QObject* parent)
	: QStyledItemDelegate(parent)
{ }

void MemCardItemDelegate::paint(QPainter *painter,
			const QStyleOptionViewItem &option,
			const QModelIndex &index) const
{
	// TODO: Register custom type for FileDescription.
	// For now, just assume that if QModelIndex.column() == 1,
	// it's the correct one.
	if (index.isValid() && index.column() == 1) {
		// File description.

		// TODO: Ensure that it has either 1 or 2 lines.
		QStringList desc = index.data().toString().split(QChar(L'\n'));
		if (desc.isEmpty())
			desc.append(QString());

		painter->save();

		// TODO: Save the QTreeView widget, use it to get the style.
		// TODO: Custom background color?
		QStyle *style = QApplication::style();
		style->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, painter);

		QFont fontGameDesc = QApplication::font();
		const QFontMetrics fmGameDesc(fontGameDesc);

		QFont fontFileDesc = fontGameDesc;
		fontFileDesc.setPointSize(fontFileDesc.pointSize() * 4 / 5);
		const QFontMetrics fmFileDesc(fontFileDesc);

		QRect rect = option.rect;

		// TODO: Centering.
		painter->setFont(fontGameDesc);

		// Font color.
		if (option.state & QStyle::State_Selected)
			painter->setPen(option.palette.highlightedText().color());
		else
			painter->setPen(option.palette.text().color());

		painter->drawText(rect, desc.at(0));
		if (desc.size() > 1) {
			painter->setFont(fontFileDesc);
			rect.setY(rect.y() + fmGameDesc.height());
			painter->drawText(rect, desc.at(1));
		}

		painter->restore();
	} else {
		// Use the default paint().
		QStyledItemDelegate::paint(painter, option, index);
	}
}

#if 0
QSize sizeHint(const QStyleOptionViewItem &option,
	       const QModelIndex &index) const;
#endif
