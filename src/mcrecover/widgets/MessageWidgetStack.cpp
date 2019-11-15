/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * MessageWidgetStack.hpp: Message widget stack.                           *
 *                                                                         *
 * Copyright (c) 2014-2016 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#include "MessageWidgetStack.hpp"
#include "McRecoverQApplication.hpp"

// Qt includes.
#include <QtCore/QSet>
#include <QtCore/QSignalMapper>
#include <QVBoxLayout>

// Qt animation includes.
#include <QtCore/QTimeLine>

/** MessageWidgetStackPrivate **/

class MessageWidgetStackPrivate
{
	public:
		explicit MessageWidgetStackPrivate(MessageWidgetStack *q);
		~MessageWidgetStackPrivate();

	protected:
		MessageWidgetStack *const q_ptr;
		Q_DECLARE_PUBLIC(MessageWidgetStack)
	private:
		Q_DISABLE_COPY(MessageWidgetStackPrivate)

	public:
		struct Ui_MessageWidgetStack {
			QVBoxLayout *vboxMain;

			void setupUi(QWidget *MessageWidgetStack);
		};
		Ui_MessageWidgetStack ui;

		// All active MessageWidgets.
		QSet<MessageWidget*> messageWidgets;

		// QSignalMapper for the dismissed() signal.
		QSignalMapper *mapperMessageWidgets;
};

MessageWidgetStackPrivate::MessageWidgetStackPrivate(MessageWidgetStack *q)
	: q_ptr(q)
	, mapperMessageWidgets(new QSignalMapper(q))
{
	// Connect the QSignalMapper slot for MessageWidget dismissed().
	QObject::connect(mapperMessageWidgets, SIGNAL(mapped(QWidget*)),
			 q, SLOT(messageWidget_dismissed_slot(QWidget*)));
}

MessageWidgetStackPrivate::~MessageWidgetStackPrivate()
{
	// NOTE: The MessageWidgets are owned by MessageWidgetStack,
	// so we don't need to delete them manually.
	messageWidgets.clear();
}

/**
 * Initialize the UI.
 * @param MessageWidgetStack MessageWidgetStack.
 */
void MessageWidgetStackPrivate::Ui_MessageWidgetStack::setupUi(QWidget *MessageWidgetStack)
{
	if (MessageWidgetStack->objectName().isEmpty())
		MessageWidgetStack->setObjectName(QLatin1String("MessageWidgetStack"));

	vboxMain = new QVBoxLayout(MessageWidgetStack);
	vboxMain->setObjectName(QLatin1String("vboxMain"));
	vboxMain->setMargin(0);

	QMetaObject::connectSlotsByName(MessageWidgetStack);
}

/** MessageWidgetStack **/

MessageWidgetStack::MessageWidgetStack(QWidget *parent)
	: super(parent)
	, d_ptr(new MessageWidgetStackPrivate(this))
{
	Q_D(MessageWidgetStack);
	d->ui.setupUi(this);
}

MessageWidgetStack::~MessageWidgetStack()
{
	Q_D(MessageWidgetStack);
	delete d;
}

/**
 * Show a message.
 * @param msg Message text. (supports Qt RichText formatting)
 * @param icon Icon.
 * @param timeout Timeout, in milliseconds. (0 for no timeout)
 * @param closeOnDestroy Close the message when the specified QObject is destroyed.
 */
void MessageWidgetStack::showMessage(const QString &msg, MessageWidget::MsgIcon icon, int timeout, QObject *closeOnDestroy)
{
	Q_D(MessageWidgetStack);
	MessageWidget *messageWidget = new MessageWidget(this);
	d->messageWidgets.insert(messageWidget);

	// Connect the signals.
	QObject::connect(messageWidget, &QObject::destroyed,
		this, &MessageWidgetStack::messageWidget_destroyed_slot);
	QObject::connect(messageWidget, SIGNAL(dismissed(bool)),
		d->mapperMessageWidgets, SLOT(map()));
	d->mapperMessageWidgets->setMapping(messageWidget, messageWidget);

	// Show the message.
	d->ui.vboxMain->addWidget(messageWidget);
	d->ui.vboxMain->setAlignment(messageWidget, Qt::AlignTop);
	messageWidget->showMessage(msg, icon, timeout, closeOnDestroy);
}

/** Slots. **/

/**
 * A MessageWidget has been dismissed.
 * @param widget MessageWidget.
 */
void MessageWidgetStack::messageWidget_dismissed_slot(QWidget *widget)
{
	// Remove and delete the widget.
	Q_D(MessageWidgetStack);
	if (d->messageWidgets.contains(qobject_cast<MessageWidget*>(widget))) {
		d->messageWidgets.remove(qobject_cast<MessageWidget*>(widget));
		delete widget;
	}
}

/**
 * A MessageWidget has been destroyed.
 * @param obj QObject that was destroyed.
 */
void MessageWidgetStack::messageWidget_destroyed_slot(QObject *obj)
{
	// Remove the widget.
	Q_D(MessageWidgetStack);
	d->messageWidgets.remove(qobject_cast<MessageWidget*>(obj));
}

