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
#include "FileComments.hpp"
#include "card.h"

// Qt includes.
#include <QtGui/QPainter>
#include <QtGui/QApplication>
#include <QtGui/QFont>
#include <QtGui/QFontMetrics>
#include <QtGui/QStyle>

#ifdef Q_OS_WIN
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif /* Q_OS_WIN */

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

#ifdef Q_OS_WIN
		// Win32: Theming functions.
	private:
		bool m_isXPTheme;
		bool m_isVistaTheme;
		static bool resolveSymbols(void);
	public:
		bool isXPTheme(bool update = false);
		bool isVistaTheme(void);
#endif /* Q_OS_WIN */
};

/** MemCardItemDelegatePrivate **/

#ifdef Q_OS_WIN
typedef bool (WINAPI *PtrIsAppThemed)(void);
typedef bool (WINAPI *PtrIsThemeActive)(void);

static HMODULE pUxThemeDll = NULL;
static PtrIsAppThemed pIsAppThemed = NULL;
static PtrIsThemeActive pIsThemeActive = NULL;
#endif /* Q_OS_WIN */

MemCardItemDelegatePrivate::MemCardItemDelegatePrivate(MemCardItemDelegate *q)
	: q(q)
#ifdef Q_OS_WIN
	, m_isXPTheme(false)
	, m_isVistaTheme(false)
#endif /* Q_OS_WIN */
{
	updateFonts();

#ifdef Q_OS_WIN
	// Update the XP theming info.
	isXPTheme(true);
#endif /* Q_OS_WIN */
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

#ifdef Q_OS_WIN
/**
 * Resolve symbols for XP/Vista theming.
 * Based on QWindowsXPStyle::resolveSymbols(). (qt-4.8.5)
 * @return True on success; false on failure.
 */
bool MemCardItemDelegatePrivate::resolveSymbols(void)
{
	static bool tried = false;
	if (!tried) {
		pUxThemeDll = LoadLibraryA("uxtheme");
		if (pUxThemeDll) {
			pIsAppThemed = (PtrIsAppThemed)GetProcAddress(pUxThemeDll, "IsAppThemed");
			if (pIsAppThemed) {
				pIsThemeActive = (PtrIsThemeActive)GetProcAddress(pUxThemeDll, "IsThemeActive");
			}
		}
		tried = true;
	}

	return (pIsAppThemed != NULL);
}

/**
 * Check if a Windows XP theme is in use.
 * Based on QWindowsXPStyle::useXP(). (qt-4.8.5)
 * @param update Update the system theming status.
 * @return True if a Windows XP theme is in use; false if not.
 */
bool MemCardItemDelegatePrivate::isXPTheme(bool update)
{
	if (!update)
		return m_isXPTheme;

	m_isXPTheme = (resolveSymbols() && pIsThemeActive() &&
		       (pIsAppThemed() || !QApplication::instance()));
	return m_isXPTheme;
}

/**
 * Check if a Windows Vista theme is in use.
 * Based on QWindowsVistaStyle::useVista(). (qt-4.8.5)
 * @return True if a Windows Vista theme is in use; false if not.
 */
bool MemCardItemDelegatePrivate::isVistaTheme(void)
{
	return (isXPTheme() &&
		QSysInfo::WindowsVersion >= QSysInfo::WV_VISTA &&
		(QSysInfo::WindowsVersion & QSysInfo::WV_NT_based));
}
#endif /* Q_OS_WIN */


/** MemCardItemDelegate **/

MemCardItemDelegate::MemCardItemDelegate(QObject *parent)
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
	if (!index.isValid() ||
	    !index.data().canConvert<FileComments>())
	{
		// Index is invalid, or this isn't FileComments.
		// Use the default paint().
		QStyledItemDelegate::paint(painter, option, index);
		return;
	}

	// GCN file comments.
	FileComments fileComments = index.data().value<FileComments>();

	// Alignment flags.
	static const int HALIGN_FLAGS =
			Qt::AlignLeft |
			Qt::AlignRight |
			Qt::AlignHCenter |
			Qt::AlignJustify;
	static const int VALIGN_FLAGS =
			Qt::AlignTop |
			Qt::AlignBottom |
			Qt::AlignVCenter;

	// Get the text alignment.
	int textAlignment = 0;
	if (index.data(Qt::TextAlignmentRole).canConvert(QVariant::Int))
		textAlignment = index.data(Qt::TextAlignmentRole).toInt();
	if (textAlignment == 0)
		textAlignment = option.displayAlignment;

	// Game description.
	// NOTE: Width is decremented in order to prevent
	// weird wordwrapping issues.
	const QFontMetrics fmGameDesc(d->fontGameDesc);
	QString gameDescElided = fmGameDesc.elidedText(
		fileComments.gameDesc(), Qt::ElideRight, option.rect.width()-1);
	QRect rectGameDesc = option.rect;
	rectGameDesc.setHeight(fmGameDesc.height());
	rectGameDesc = fmGameDesc.boundingRect(
		rectGameDesc, (textAlignment & HALIGN_FLAGS), gameDescElided);

	// File description.
	painter->setFont(d->fontFileDesc);
	const QFontMetrics fmFileDesc(d->fontFileDesc);
	QString fileDescElided = fmFileDesc.elidedText(
		fileComments.fileDesc(), Qt::ElideRight, option.rect.width()-1);
	QRect rectFileDesc = option.rect;
	rectFileDesc.setHeight(fmFileDesc.height());
	rectFileDesc.setY(rectGameDesc.y() + rectGameDesc.height());
	rectFileDesc = fmFileDesc.boundingRect(
		rectFileDesc, (textAlignment & HALIGN_FLAGS), fileDescElided);

	// Adjust for vertical alignment.
	int diff = 0;
	switch (textAlignment & VALIGN_FLAGS) {
		default:
		case Qt::AlignTop:
			// No adjustment is necessary.
			break;

		case Qt::AlignBottom:
			// Bottom alignment.
			diff = (option.rect.height() - rectGameDesc.height() - rectFileDesc.height());
			break;

		case Qt::AlignVCenter:
			// Center alignment.
			diff = (option.rect.height() - rectGameDesc.height() - rectFileDesc.height());
			diff /= 2;
			break;
	}

	if (diff != 0) {
		rectGameDesc.translate(0, diff);
		rectFileDesc.translate(0, diff);
	}

	painter->save();

	// Draw the background color first.
	QVariant bg_var = index.data(Qt::BackgroundRole);
	QStyleOptionViewItemV4 bgOption = option;
	QBrush bg;
	if (bg_var.canConvert<QBrush>()) {
		bg = bg_var.value<QBrush>();
	} else {
		// Check for Qt::BackgroundColorRole.
		bg_var = index.data(Qt::BackgroundColorRole);
		if (bg_var.canConvert<QColor>())
			bg = QBrush(bg_var.value<QColor>());
	}
	if (bg.style() != Qt::NoBrush)
		bgOption.backgroundBrush = bg;

	// Draw the style element.
	QStyle *style = bgOption.widget ? bgOption.widget->style() : QApplication::style();
	style->drawControl(QStyle::CE_ItemViewItem, &bgOption, painter, bgOption.widget);
	bgOption.backgroundBrush = QBrush();

#ifdef Q_OS_WIN
	// Adjust the palette for Vista themes.
	if (d->isVistaTheme()) {
		// Vista theme uses a slightly different palette.
		// See: qwindowsvistastyle.cpp::drawControl(), line 1524 (qt-4.8.5)
		QPalette *palette = &bgOption.palette;
                palette->setColor(QPalette::All, QPalette::HighlightedText, palette->color(QPalette::Active, QPalette::Text));
                // Note that setting a saturated color here results in ugly XOR colors in the focus rect
                palette->setColor(QPalette::All, QPalette::Highlight, palette->base().color().darker(108));
	}
#endif

	// Font color.
	if (option.state & QStyle::State_Selected)
		painter->setPen(bgOption.palette.highlightedText().color());
	else
		painter->setPen(bgOption.palette.text().color());

	painter->setFont(d->fontGameDesc);
	painter->drawText(rectGameDesc, gameDescElided);
	painter->setFont(d->fontFileDesc);
	painter->drawText(rectFileDesc, fileDescElided);

	painter->restore();
}

QSize MemCardItemDelegate::sizeHint(const QStyleOptionViewItem &option,
				    const QModelIndex &index) const
{
	if (!index.isValid() ||
	    !index.data().canConvert<FileComments>())
	{
		// Index is invalid, or this isn't FileComments.
		// Use the default sizeHint().
		QSize sz = QStyledItemDelegate::sizeHint(option, index);

		// Minimum height.
		static const int MIN_H = (CARD_ICON_H + 4);
		if (sz.height() < MIN_H)
			sz.setHeight(MIN_H);
		return sz;
	}

	// GCN file comments.
	FileComments fileComments = index.data().value<FileComments>();

	// Game description.
	const QFontMetrics fmGameDesc(d->fontGameDesc);
	QSize sz = fmGameDesc.size(0, fileComments.gameDesc());

	// File description.
	const QFontMetrics fmFileDesc(d->fontFileDesc);
	QSize fileSz = fmFileDesc.size(0, fileComments.fileDesc());
	sz.setHeight(sz.height() + fileSz.height());

	if (fileSz.width() > sz.width())
		sz.setWidth(fileSz.width());

	// Increase width by 1 to prevent accidental eliding.
	// NOTE: We can't just remove the "-1" from paint(),
	// because that still causes weird wordwrapping.
	if (sz.width() > 0)
		sz.setWidth(sz.width() + 1);

	return sz;
}
