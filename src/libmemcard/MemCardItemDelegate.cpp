/***************************************************************************
 * GameCube Memory Card Recovery Program [libmemcard]                      *
 * MemCardItemDelegate.cpp: MemCard item delegate for QTreeView.           *
 *                                                                         *
 * Copyright (c) 2013-2020 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#include "MemCardItemDelegate.hpp"

#include "MemCardModel.hpp"
#include "card.h"

// Qt includes.
#include <QtGui/QPainter>
#include <QApplication>
#include <QtGui/QFont>
#include <QtGui/QFontMetrics>
#include <QStyle>

#ifdef Q_OS_WIN
#include <windows.h>
#endif /* Q_OS_WIN */

class MemCardItemDelegatePrivate
{
	public:
		explicit MemCardItemDelegatePrivate(MemCardItemDelegate *q);

	protected:
		MemCardItemDelegate *const q_ptr;
		Q_DECLARE_PUBLIC(MemCardItemDelegate)
	private:
		Q_DISABLE_COPY(MemCardItemDelegatePrivate)

	public:
		// Font retrieval.
		QFont fontGameDesc(const QWidget *widget = 0) const;
		QFont fontFileDesc(const QWidget *widget = 0) const;

#ifdef Q_OS_WIN
		// Win32: Theming functions.
	private:
		// HACK: Mark this as mutable so const functions can update it.
		mutable bool m_isXPTheme;
		static bool resolveSymbols(void);
	public:
		bool isXPTheme(bool update = false) const;
		bool isVistaTheme(void) const;
#endif /* Q_OS_WIN */
};

/** MemCardItemDelegatePrivate **/

MemCardItemDelegatePrivate::MemCardItemDelegatePrivate(MemCardItemDelegate *q)
	: q_ptr(q)
#ifdef Q_OS_WIN
	, m_isXPTheme(false)
#endif /* Q_OS_WIN */
{
#ifdef Q_OS_WIN
	// Update the XP theming info.
	isXPTheme(true);
#endif /* Q_OS_WIN */
}

/**
 * Get the Game Description font.
 * @param widget Relevant widget. (If nullptr, use QApplication.)
 * @return Game Description font.
 */
QFont MemCardItemDelegatePrivate::fontGameDesc(const QWidget *widget) const
{
	// TODO: This should be cached, but we don't have a
	// reasonable way to update it if the system font
	// is changed...
	return (widget != nullptr
		? widget->font()
		: QApplication::font());
}

/**
 * Get the File Description font.
 * @param widget Relevant widget. (If nullptr, use QApplication.)
 * @return File Description font.
 */
QFont MemCardItemDelegatePrivate::fontFileDesc(const QWidget *widget) const
{
	// TODO: This should be cached, but we don't have a
	// reasonable way to update it if the system font
	// is changed...
	QFont fontFileDesc = fontGameDesc(widget);
	int pointSize = fontFileDesc.pointSize();
	if (pointSize >= 10)
		pointSize = (pointSize * 4 / 5);
	else
		pointSize--;
	fontFileDesc.setPointSize(pointSize);
	return fontFileDesc;
}

#ifdef Q_OS_WIN
typedef bool (WINAPI *PtrIsAppThemed)(void);
typedef bool (WINAPI *PtrIsThemeActive)(void);

static HMODULE pUxThemeDll = nullptr;
static PtrIsAppThemed pIsAppThemed = nullptr;
static PtrIsThemeActive pIsThemeActive = nullptr;

/**
 * Resolve symbols for XP/Vista theming.
 * Based on QWindowsXPStyle::resolveSymbols(). (qt-4.8.5)
 * @return True on success; false on failure.
 */
bool MemCardItemDelegatePrivate::resolveSymbols(void)
{
	static bool tried = false;
	if (!tried) {
		pUxThemeDll = LoadLibraryW(L"uxtheme");
		if (pUxThemeDll) {
			pIsAppThemed = (PtrIsAppThemed)GetProcAddress(pUxThemeDll, "IsAppThemed");
			if (pIsAppThemed) {
				pIsThemeActive = (PtrIsThemeActive)GetProcAddress(pUxThemeDll, "IsThemeActive");
			}
		}
		tried = true;
	}

	return (pIsAppThemed != nullptr);
}

/**
 * Check if a Windows XP theme is in use.
 * Based on QWindowsXPStyle::useXP(). (qt-4.8.5)
 * @param update Update the system theming status.
 * @return True if a Windows XP theme is in use; false if not.
 */
bool MemCardItemDelegatePrivate::isXPTheme(bool update) const
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
bool MemCardItemDelegatePrivate::isVistaTheme(void) const
{
	return (isXPTheme() &&
		QSysInfo::WindowsVersion >= QSysInfo::WV_VISTA &&
		(QSysInfo::WindowsVersion & QSysInfo::WV_NT_based));
}
#endif /* Q_OS_WIN */

/** MemCardItemDelegate **/

MemCardItemDelegate::MemCardItemDelegate(QObject *parent)
	: super(parent)
	, d_ptr(new MemCardItemDelegatePrivate(this))
{
#ifdef Q_OS_WIN
	// Connect the "themeChanged" signal.
	connect(qApp, SIGNAL(themeChanged()),
		this, SLOT(themeChanged_slot()));
#endif /* Q_OS_WIN */
}

MemCardItemDelegate::~MemCardItemDelegate()
{
	Q_D(MemCardItemDelegate);
	delete d;
}

void MemCardItemDelegate::paint(QPainter *painter,
			const QStyleOptionViewItem &option,
			const QModelIndex &index) const
{
	if (!index.isValid()) {
		// Index is invalid.
		// Use the default paint().
		super::paint(painter, option, index);
		return;
	}

	// TODO: Combine code with sizeHint().

	// GCN file comments: "GameDesc\0FileDesc"
	// If no '\0' is present, assume this is regular text
	// and use the default paint().
	QString s_fileComments = index.data().toString();
	QStringList sl = s_fileComments.split(QChar(L'\0'));
	if (sl.size() != 2) {
		// No '\0' is present.
		// Use the default paint().
		super::paint(painter, option, index);
		return;
	}

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

	QRect textRect = option.rect;
	QStyleOptionViewItem bgOption = option;

	// Horizontal margins.
	// Reference: http://doc.qt.io/qt-4.8/qitemdelegate.html#sizeHint
	QStyle *const style = bgOption.widget ? bgOption.widget->style() : QApplication::style();
	//const int hmargin = style->pixelMetric(QStyle::PM_FocusFrameHMargin, &option) * 2;

	// Reduce the text rectangle by the hmargin.
	// TODO: vmargin?
	// FIXME: This cuts off the text and doesn't match the alignment
	// of the other columns... (Maybe it's only useful if we're showing
	// an icon in the same column as the text?)
	//textRect.adjust(hmargin, 0, -hmargin, 0);

	// Get the fonts.
	Q_D(const MemCardItemDelegate);
	QFont fontGameDesc = d->fontGameDesc(bgOption.widget);
	QFont fontFileDesc = d->fontFileDesc(bgOption.widget);

	// Total text height.
	int textHeight = 0;

	// Text boundaries.
	QVector<QRect> v_rect;
	v_rect.resize(sl.size());

	for (int i = 0; i < sl.size(); i++) {
		// Name uses the normal font.
		// Description lines use a slightly smaller font.
		QString &line = sl[i];
		QRect &rect = v_rect[i];

		const QFontMetrics fm(i == 0 ? fontGameDesc : fontFileDesc);
		line = fm.elidedText(line, Qt::ElideRight, textRect.width()-1);
		QRect tmpRect(textRect.x(), textRect.y() + textHeight, textRect.width(), fm.height());
		textHeight += fm.height();
		rect = fm.boundingRect(tmpRect, (textAlignment & HALIGN_FLAGS), line);
	}

	// Adjust for vertical alignment.
	int diff = 0;
	switch (textAlignment & VALIGN_FLAGS) {
		default:
		case Qt::AlignTop:
			// No adjustment is necessary.
			break;

		case Qt::AlignBottom:
			// Bottom alignment.
			diff = (textRect.height() - textHeight);
			break;

		case Qt::AlignVCenter:
			// Center alignment.
			diff = (textRect.height() - textHeight);
			diff /= 2;
			break;
	}

	if (diff != 0) {
		std::for_each(v_rect.begin(), v_rect.end(),
			[diff](QRect &rect) {
				rect.translate(0, diff);
			}
		);
	}

	painter->save();

	// Draw the background color first.
	QVariant bg_var = index.data(Qt::BackgroundRole);
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
	if (bgOption.state & QStyle::State_Selected) {
		painter->setPen(bgOption.palette.highlightedText().color());
	} else {
		painter->setPen(bgOption.palette.text().color());
	}

	// Draw the text lines.
	painter->setFont(fontGameDesc);
	int i = 0;
	auto iter_rect = v_rect.cbegin();
	const auto iter_sl_cend = sl.cend();
	for (auto iter_sl = sl.cbegin(); iter_sl != iter_sl_cend; ++iter_sl, ++iter_rect, ++i) {
		if (i == 1) {
			painter->setFont(fontFileDesc);
		}
		painter->drawText(*iter_rect, *iter_sl);
	}

	painter->restore();
}

QSize MemCardItemDelegate::sizeHint(const QStyleOptionViewItem &option,
				    const QModelIndex &index) const
{
	if (!index.isValid()) {
		// Index is invalid.
		// Use the default sizeHint().
		QSize sz = super::sizeHint(option, index);

		// Minimum height.
		static const int MIN_H = (CARD_ICON_H + 4);
		if (sz.height() < MIN_H)
			sz.setHeight(MIN_H);
		return sz;
	}

	// TODO: Combine code with paint().

	// GCN file comments: "GameDesc\0FileDesc"
	// If no '\0' is present, assume this is regular text
	// and use the default paint().
	QString s_fileComments = index.data().toString();
	QStringList sl = s_fileComments.split(QChar(L'\0'));
	if (sl.size() != 2) {
		// No '\0' is present.
		// TODO: Combine with !index.isValid() case.
		QSize sz = super::sizeHint(option, index);

		// Minimum height.
		static const int MIN_H = (CARD_ICON_H + 4);
		if (sz.height() < MIN_H)
			sz.setHeight(MIN_H);
		return sz;
	}

	// Get the fonts.
	Q_D(const MemCardItemDelegate);
	QFont fontGameDesc = d->fontGameDesc(option.widget);
	QFont fontFileDesc = d->fontFileDesc(option.widget);

	QSize sz;
	for (int i = 0; i < sl.size(); i++) {
		// Game description uses the normal font.
		// File description lines use a slightly smaller font.
		const QString &line = sl[i];

		const QFontMetrics fm(i == 0 ? fontGameDesc : fontFileDesc);
		QSize szLine = fm.size(0, line);
		sz.setHeight(sz.height() + szLine.height());

		if (szLine.width() > sz.width()) {
			sz.setWidth(szLine.width());
		}
	}

	// Increase width by 1 to prevent accidental eliding.
	// NOTE: We can't just remove the "-1" from paint(),
	// because that still causes weird wordwrapping.
	if (sz.width() > 0)
		sz.setWidth(sz.width() + 1);

	return sz;
}

/** Slots. **/

/**
 * The system theme has changed.
 */
void MemCardItemDelegate::themeChanged_slot(void)
{
#ifdef Q_OS_WIN
	// Update the XP theming info.
	Q_D(MemCardItemDelegate);
	d->isXPTheme(true);
#endif
}
