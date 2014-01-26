/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * MessageWidget.hpp: Message widget.                                      *
 *                                                                         *
 * Copyright (c) 2014 by David Korth.                                      *
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

#include "MessageWidget.hpp"
#include "McRecoverQApplication.hpp"

// Qt includes.
#include <QtGui/QPaintEvent>
#include <QtGui/QPainter>

/** MessageWidgetPrivate **/

#include "ui_MessageWidget.h"
class MessageWidgetPrivate
{
	public:
		MessageWidgetPrivate(MessageWidget *q);
		~MessageWidgetPrivate();

	protected:
		MessageWidget *const q_ptr;
		Q_DECLARE_PUBLIC(MessageWidget)
	private:
		Q_DISABLE_COPY(MessageWidgetPrivate)

	public:
		Ui::MessageWidget ui;

		// Icon being displayed.
		MessageWidget::MsgIcon icon;
		static const int iconSz = 22;
		void setIcon(MessageWidget::MsgIcon icon);

		// Message timeout. (TODO)
		int timeout;

		// Colors.
		static const QRgb colorCritical = 0xEE4444;
		static const QRgb colorQuestion = 0x66EE66;
		static const QRgb colorWarning = 0xEECC66;
		static const QRgb colorInformation = 0x66CCEE;
};

MessageWidgetPrivate::MessageWidgetPrivate(MessageWidget *q)
	: q_ptr(q)
	, icon(MessageWidget::ICON_NONE)
	, timeout(0)
{ }

MessageWidgetPrivate::~MessageWidgetPrivate()
{ }

/**
 * Set the icon.
 * icon Icon to set.
 */
void MessageWidgetPrivate::setIcon(MessageWidget::MsgIcon icon)
{
	if (icon < MessageWidget::ICON_NONE || icon >= MessageWidget::ICON_MAX)
		icon = MessageWidget::ICON_NONE;

	// TODO: Use system icons if available?
	const char *iconName = nullptr;
	switch (icon) {
		case MessageWidget::ICON_NONE:
		default:
			iconName = nullptr;
			break;
		case MessageWidget::ICON_CRITICAL:
			iconName = "dialog-error";
			break;
		case MessageWidget::ICON_QUESTION:
			iconName = "dialog-question";
			break;
		case MessageWidget::ICON_WARNING:
			iconName = "dialog-warning";
			break;
		case MessageWidget::ICON_INFORMATION:
			iconName = "dialog-information";
			break;
	}

	if (!iconName) {
		ui.lblIcon->setVisible(false);
	} else {
		QIcon icon = McRecoverQApplication::IconFromTheme(QLatin1String(iconName));
		ui.lblIcon->setPixmap(icon.pixmap(iconSz, iconSz));
	}
}

/** MessageWidget **/

MessageWidget::MessageWidget(QWidget *parent)
	: QWidget(parent)
	, d_ptr(new MessageWidgetPrivate(this))
{
	Q_D(MessageWidget);
	d->ui.setupUi(this);
	d->setIcon(d->icon);
}

MessageWidget::~MessageWidget()
{
	Q_D(MessageWidget);
	delete d;
}

/** Events. **/

/**
 * Paint event.
 * @param event QPaintEvent.
 */
void MessageWidget::paintEvent(QPaintEvent *event)
{
	QPainter painter(this);

	// Drawing rectangle should be this->rect(),
	// minus one pixel width and height.
	QRect drawRect(this->rect());
	drawRect.setWidth(drawRect.width() - 1);
	drawRect.setHeight(drawRect.height() - 1);

	// Determine the background color based on icon.
	QBrush bgColor;
	Q_D(MessageWidget);
	switch (d->icon) {
		case ICON_CRITICAL:
			bgColor = QColor(MessageWidgetPrivate::colorCritical);
			break;
		case ICON_QUESTION:
			bgColor = QColor(MessageWidgetPrivate::colorQuestion);
			break;
		case ICON_WARNING:
			bgColor = QColor(MessageWidgetPrivate::colorWarning);
			break;
		case ICON_INFORMATION:
			bgColor = QColor(MessageWidgetPrivate::colorInformation);
			break;
		default:
			// FIXME: This looks terrible.
			bgColor = QColor(Qt::white);
			break;
	}

	if (bgColor != QColor(Qt::white)) {
		painter.setPen(QColor(Qt::black));
		painter.setBrush(bgColor);
		painter.drawRoundedRect(drawRect, 5.0, 5.0);
	}
}

/** Slots. **/

/**
 * Show a message.
 * @param msg Message text. (supports Qt RichText formatting)
 * @param icon Icon.
 * @param timeout Timeout, in milliseconds. (0 for no timeout)
 */
void MessageWidget::showMessage(const QString &msg, MsgIcon icon, int timeout)
{
	Q_D(MessageWidget);
	d->ui.lblMessage->setText(msg);
	d->setIcon(icon);
	d->timeout = timeout; // TODO

	// Show the message widget.
	this->show();
}
