/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * MessageWidget.hpp: Message widget.                                      *
 *                                                                         *
 * Copyright (c) 2014-2018 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#ifndef __MCRECOVER_MESSAGEWIDGET_HPP__
#define __MCRECOVER_MESSAGEWIDGET_HPP__

#include <QWidget>

class MessageWidgetPrivate;

class MessageWidget : public QWidget
{
	Q_OBJECT
	typedef QWidget super;

	Q_ENUMS(MsgIcon)

	public:
		explicit MessageWidget(QWidget *parent = 0);
		virtual ~MessageWidget();

	protected:
		MessageWidgetPrivate *const d_ptr;
		Q_DECLARE_PRIVATE(MessageWidget)
	private:
		Q_DISABLE_COPY(MessageWidget)

	public:
		/**
		 * Icon types.
		 */
		enum MsgIcon {
			ICON_NONE,
			ICON_CRITICAL,
			ICON_QUESTION,
			ICON_WARNING,
			ICON_INFORMATION,

			ICON_MAX
		};

	protected:
		/**
		 * Paint event.
		 * @param event QPaintEvent.
		 */
		void paintEvent(QPaintEvent *event) final;

		/**
		 * Hide event.
		 * @param event QHideEvent.
		 */
		void showEvent(QShowEvent *event) final;

	public slots:
		/**
		 * Show a message.
		 * @param msg Message text. (supports Qt RichText formatting)
		 * @param icon Icon.
		 * @param timeout Timeout, in milliseconds. (0 for no timeout)
		 * @param closeOnDestroy Close the message when the specified QObject is destroyed.
		 */
		void showMessage(const QString &msg, MsgIcon icon, int timeout = 0, QObject *closeOnDestroy = 0);

		/**
		 * Show the MessageWidget using animation.
		 * NOTE: You should probably use showMessage()!
		 */
		void showAnimated(void);

		/**
		 * Hide the MessageWidget using animation.
		 */
		void hideAnimated(void);

	protected slots:
		/**
		 * Message timer has expired.
		 */
		void tmrTimeout_timeout(void);

		/**
		 * Animation timeline has changed.
		 * @param value Timeline value.
		 */
		void timeLineChanged_slot(qreal value);

		/**
		 * Animation timeline has finished.
		 */
		void timeLineFinished_slot(void);

		/**
		 * "Dismiss" button has been clicked.
		 */
		void on_btnDismiss_clicked(void);

	signals:
		/**
		 * Message has been dismissed,
		 * either manually or via timeout.
		 * @param timeout True if the message time out.
		 */
		void dismissed(bool timeout);
};

#endif /* __MCRECOVER_MESSAGEWIDGET_HPP__ */
