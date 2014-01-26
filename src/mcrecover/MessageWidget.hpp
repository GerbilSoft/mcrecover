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

#ifndef __MCRECOVER_MESSAGEWIDGET_HPP__
#define __MCRECOVER_MESSAGEWIDGET_HPP__

#include <QtGui/QWidget>

class MessageWidgetPrivate;

class MessageWidget : public QWidget
{
	Q_OBJECT

	public:
		MessageWidget(QWidget *parent = 0);
		~MessageWidget();

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
		void paintEvent(QPaintEvent *event);

		/**
		 * Hide event.
		 * @param event QHideEvent.
		 */
		void hideEvent(QHideEvent *event);

	public slots:
		/**
		 * Show a message.
		 * @param msg Message text. (supports Qt RichText formatting)
		 * @param icon Icon.
		 * @param timeout Timeout, in milliseconds. (0 for no timeout)
		 */
		void showMessage(const QString &msg, MsgIcon icon, int timeout);

	protected slots:
		/**
		 * Message timer has expired.
		 */
		void tmrTimeout_timeout(void);
};

#endif /* __MCRECOVER_MEMCARDVIEW_HPP__ */
