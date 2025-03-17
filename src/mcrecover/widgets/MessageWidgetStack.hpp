/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * MessageWidgetStack.hpp: Message widget stack.                           *
 *                                                                         *
 * Copyright (c) 2014-2025 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#ifndef __MCRECOVER_MESSAGEWIDGETSTACK_HPP__
#define __MCRECOVER_MESSAGEWIDGETSTACK_HPP__

#include <QWidget>
#include "MessageWidget.hpp"

class MessageWidgetStackPrivate;
class MessageWidgetStack : public QWidget
{
	Q_OBJECT
	typedef QWidget super;

public:
	explicit MessageWidgetStack(QWidget *parent = 0);
	virtual ~MessageWidgetStack();

protected:
	MessageWidgetStackPrivate *const d_ptr;
	Q_DECLARE_PRIVATE(MessageWidgetStack)
private:
	Q_DISABLE_COPY(MessageWidgetStack)

public:
	/**
	 * Show a message.
	 * @param msg Message text (supports Qt RichText formatting)
	 * @param icon Icon
	 * @param timeout Timeout, in milliseconds (0 for no timeout)
	 * @param closeOnDestroy Close the message when the specified QObject is destroyed.
	 */
	void showMessage(const QString &msg, MessageWidget::MsgIcon icon, int timeout = 0, QObject *closeOnDestroy = 0);

protected slots:
	/**
	 * A MessageWidget has been dismissed.
	 * @param widget MessageWidget
	 */
	void messageWidget_dismissed_slot(QWidget *widget);

	/**
	 * A MessageWidget has been destroyed.
	 * @param obj QObject that was destroyed
	 */
	void messageWidget_destroyed_slot(QObject *obj);
};

#endif /* __MCRECOVER_MESSAGEWIDGETSTACK_HPP__ */
