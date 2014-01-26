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
#include <QtCore/QTimer>
#include <QtGui/QPaintEvent>
#include <QtGui/QPainter>

// Qt animation includes.
#include <QtCore/QTimeLine>

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
		QTimer *tmrTimeout;

		// Colors.
		// TODO: Use system colors on KDE?
		static const QRgb colorCritical = 0xEE4444;
		static const QRgb colorQuestion = 0x66EE66;
		static const QRgb colorWarning = 0xEECC66;
		static const QRgb colorInformation = 0x66CCEE;

		// Animation.
		QTimeLine *timeLine;
		int calcBestHeight(void) const;
		bool animateOnShow;
};

MessageWidgetPrivate::MessageWidgetPrivate(MessageWidget *q)
	: q_ptr(q)
	, icon(MessageWidget::ICON_NONE)
	, tmrTimeout(new QTimer(q))
	, timeLine(new QTimeLine(500, q))
	, animateOnShow(false)
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
	if (this->icon == icon)
		return;
	this->icon = icon;

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
		ui.lblIcon->setVisible(true);
	}

	Q_Q(MessageWidget);
	if (q->isVisible())
		q->update();
}

/**
 * Calculate the best height for the widget.
 * @return Best height.
 */
int MessageWidgetPrivate::calcBestHeight(void) const
{
	int height = ui.content->sizeHint().height();
	height += ui.hboxMain->contentsMargins().top();
	height += ui.hboxMain->contentsMargins().bottom();
	height += ui.hboxFrame->contentsMargins().top();
	height += ui.hboxFrame->contentsMargins().bottom();
	return height;
}

/** MessageWidget **/

MessageWidget::MessageWidget(QWidget *parent)
	: QWidget(parent)
	, d_ptr(new MessageWidgetPrivate(this))
{
	Q_D(MessageWidget);
	d->ui.setupUi(this);
	d->ui.hboxMain->setAlignment(Qt::AlignTop);
	d->ui.hboxFrame->setAlignment(d->ui.lblIcon, Qt::AlignTop);
	d->ui.hboxFrame->setAlignment(d->ui.lblMessage, Qt::AlignTop);
	d->ui.hboxFrame->setAlignment(d->ui.btnDismiss, Qt::AlignTop);
	d->setIcon(d->icon);

	// Connect the timer signal.
	QObject::connect(d->tmrTimeout, SIGNAL(timeout()),
			 this, SLOT(tmrTimeout_timeout()));

	// Connect the timeline signals.
	QObject::connect(d->timeLine, SIGNAL(valueChanged(qreal)),
			 this, SLOT(timeLineChanged_slot(qreal)));
	QObject::connect(d->timeLine, SIGNAL(finished()),
			 this, SLOT(timeLineFinished_slot()));
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
	// Call the superclass paintEvent first.
	QWidget::paintEvent(event);

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

/**
 * Show event.
 * @param event QShowEent.
 */
void MessageWidget::showEvent(QShowEvent *event)
{
	// Call the superclass showEvent.
	QWidget::showEvent(event);

	Q_D(MessageWidget);
	if (d->animateOnShow) {
		// Start the animation.
		d->animateOnShow = false;
		d->timeLine->setDirection(QTimeLine::Forward);
		if (d->timeLine->state() == QTimeLine::NotRunning) {
			d->timeLine->start();
		}
	}
}

/**
 * Hide event.
 * @param event QHideEvent.
 */
void MessageWidget::hideEvent(QHideEvent *event)
{
	// Stop the timer.
	Q_D(MessageWidget);
	d->tmrTimeout->stop();

	// Call the superclass hideEvent.
	QWidget::hideEvent(event);
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

	// Set up the timer.
	d->tmrTimeout->stop();
	d->tmrTimeout->setInterval(timeout);

	// If the widget is already visible, just update it.
	if (this->isVisible()) {
		update();
		return;
	}

	// Do an animated show.
	this->showAnimated();
}

/**
 * Show the MessageWidget using animation.
 */
void MessageWidget::showAnimated(void)
{
	Q_D(MessageWidget);
	setFixedHeight(0);
	d->ui.content->setGeometry(0, 0, width(), d->calcBestHeight());
	d->animateOnShow = true;
	d->tmrTimeout->stop();
	this->show();
}

/**
 * Hide the MessageWidget using animation.
 */
void MessageWidget::hideAnimated(void)
{
	Q_D(MessageWidget);

	// Start the animation.
	d->animateOnShow = false;
	d->tmrTimeout->stop();
	d->timeLine->setDirection(QTimeLine::Backward);
	if (d->timeLine->state() == QTimeLine::NotRunning) {
		d->timeLine->start();
	}
}

/**
 * Message timer has expired.
 */
void MessageWidget::tmrTimeout_timeout(void)
{
	// Hide the message.
	this->hide();
}


/**
 * Animation timeline has changed.
 * @param value Timeline value.
 */
void MessageWidget::timeLineChanged_slot(qreal value)
{
	Q_D(MessageWidget);
	this->setFixedHeight(qMin(value, qreal(1.0)) * d->calcBestHeight());
}

/**
 * Animation timeline has finished.
 */
void MessageWidget::timeLineFinished_slot(void)
{
	Q_D(MessageWidget);
	if (d->timeLine->direction() == QTimeLine::Forward) {
		// Make sure the widget is full-size.
		this->setFixedHeight(d->calcBestHeight());

		// Start the timeout timer, if specified.
		if (d->tmrTimeout->interval() > 0)
			d->tmrTimeout->start();
	} else {
		// Hide the widget.
		this->hide();
	}
}

/**
 * "Dismiss" button has been clicked.
 */
void MessageWidget::on_btnDismiss_clicked(void)
{
	// Hide the message using animation.
	Q_D(MessageWidget);
	if (d->timeLine->state() == QTimeLine::NotRunning)
		this->hideAnimated();
}
