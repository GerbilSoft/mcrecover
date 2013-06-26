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

class MemCardItemDelegatePrivate
{
	public:
		MemCardItemDelegatePrivate(MemCardItemDelegate *q);

	private:
		MemCardItemDelegate *const q;
		Q_DISABLE_COPY(MemCardItemDelegatePrivate);

	public:
		QFont fontGameDesc;
		QFont fontFileDesc;

		/**
		 * Update the fonts.
		 */
		void updateFonts(void);
};

/** MemCardItemDelegatePrivate **/

MemCardItemDelegatePrivate::MemCardItemDelegatePrivate(MemCardItemDelegate *q)
	: q(q)
{
	updateFonts();
}

/**
 * Update the fonts.
 */
void MemCardItemDelegatePrivate::updateFonts(void)
{
	// TODO: Get the font from the widget.
	// TODO: Update these if the font changes.
	fontGameDesc = QApplication::font();

	fontFileDesc = fontGameDesc;
	int pointSize = fontFileDesc.pointSize();
	if (pointSize >= 10)
		pointSize = (pointSize * 4 / 5);
	else
		pointSize--;
	fontFileDesc.setPointSize(pointSize);
}


/** MemCardItemDelegate **/

MemCardItemDelegate::MemCardItemDelegate(QObject* parent)
	: QStyledItemDelegate(parent)
	, d(new MemCardItemDelegatePrivate(this))
{ }

MemCardItemDelegate::~MemCardItemDelegate()
{
	delete d;
}

void MemCardItemDelegate::paint(QPainter *painter,
			const QStyleOptionViewItem &option,
			const QModelIndex &index) const
{
	// TODO: Register custom type for FileComments.
	// For now, just assume that if QModelIndex.column() == 1,
	// it's the correct one.
	if (index.isValid() && index.column() == 1) {
		// GCN file comments.
		// TODO: Disambiguate:
		// - "File Comments" == both lines
		// - "Game Description" == first line
		// - "File Description" == second line

		// TODO: Ensure that it has either 1 or 2 lines.
		QStringList desc = index.data().toString().split(QChar(L'\n'));
		if (desc.isEmpty())
			desc.append(QString());

		painter->save();

		// TODO: Save the QTreeView widget, use it to get the style.
		bool isBgPainted = false;
		if (!(option.state & QStyle::State_Selected)) {
			// Check if a custom background color is specified.
			QVariant bg_var = index.data(Qt::BackgroundRole);
			if (bg_var.canConvert<QBrush>()) {
				QBrush bg = bg_var.value<QBrush>();
				painter->fillRect(option.rect, bg);
				isBgPainted = true;
			} else {
				// Check for Qt::BackgroundColorRole.
				bg_var = index.data(Qt::BackgroundColorRole);
				if (bg_var.canConvert<QColor>()) {
					QColor bg = bg_var.value<QColor>();
					painter->fillRect(option.rect, bg);
					isBgPainted = true;
				}
			}
		}

		if (!isBgPainted) {
			// Use the default styling.
			QStyle *style = QApplication::style();
			style->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, painter);
		}

		QRect rect = option.rect;

		// TODO: Centering.
		painter->setFont(d->fontGameDesc);

		// Font color.
		if (option.state & QStyle::State_Selected)
			painter->setPen(option.palette.highlightedText().color());
		else
			painter->setPen(option.palette.text().color());

		// Game description.
		// NOTE: Width is decremented in order to prevent
		// weird wordwrapping issues.
		const QFontMetrics fmGameDesc(d->fontGameDesc);
		QString gameDescElided = fmGameDesc.elidedText(
			desc.at(0), Qt::ElideRight, rect.width()-1);
		painter->drawText(rect, gameDescElided);

		if (desc.size() > 1) {
			// File description.
			painter->setFont(d->fontFileDesc);
			const QFontMetrics fmFileDesc(d->fontFileDesc);
			rect.setY(rect.y() + fmGameDesc.height());
			QString fileDescElided = fmFileDesc.elidedText(
				desc.at(1), Qt::ElideRight, rect.width()-1);
			painter->drawText(rect, fileDescElided);
		}

		painter->restore();
	} else {
		// Use the default paint().
		QStyledItemDelegate::paint(painter, option, index);
	}
}

QSize MemCardItemDelegate::sizeHint(const QStyleOptionViewItem &option,
				    const QModelIndex &index) const
{
	// TODO: Register custom type for FileComments.
	// For now, just assume that if QModelIndex.column() == 1,
	// it's the correct one.
	if (index.isValid() && index.column() == 1) {
		// GCN file comments.
		// TODO: Disambiguate:
		// - "File Comments" == both lines
		// - "Game Description" == first line
		// - "File Description" == second line

		// TODO: Ensure that it has either 1 or 2 lines.
		QStringList desc = index.data().toString().split(QChar(L'\n'));
		if (desc.isEmpty())
			desc.append(QString());

		// Game description.
		const QFontMetrics fmGameDesc(d->fontGameDesc);
		QSize sz = fmGameDesc.size(0, desc.at(0));

		if (desc.size() > 1) {
			// File description.
			const QFontMetrics fmFileDesc(d->fontFileDesc);
			QSize fileSz = fmFileDesc.size(0, desc.at(1));
			sz.setHeight(sz.height() + fileSz.height());
			if (fileSz.width() > sz.width())
				sz.setWidth(fileSz.width());
		}

		// Increase width by 1 to prevent accidental eliding.
		// NOTE: We can't just remove the "-1" from paint(),
		// because that still causes weird wordwrapping.
		if (sz.width() > 0)
			sz.setWidth(sz.width() + 1);

		return sz;
	} else {
		return QStyledItemDelegate::sizeHint(option, index);
	}
}
