/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * HerpDerpEggListener.hpp: Listener for... something. (shh)               *
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

#ifndef __MCRECOVER_SEKRIT_HERPDERPEGGLISTENER_HPP__
#define __MCRECOVER_SEKRIT_HERPDERPEGGLISTENER_HPP__

// Qt includes and classes.
#include <QtCore/QObject>
class QKeyEvent;
class QFocusEvent;

#include "HackDetection.hpp"

// HerpDerpEggListener private class.
class HerpDerpEggListenerPrivate;

class HerpDerpEggListener : public QObject
{
	Q_OBJECT

	public:
		HerpDerpEggListener(QObject *parent = 0);
		~HerpDerpEggListener();

	protected:
		HerpDerpEggListenerPrivate *const d_ptr;
		Q_DECLARE_PRIVATE(HerpDerpEggListener)
	private:
		Q_DISABLE_COPY(HerpDerpEggListener)

	public:
		/**
		 * Set the selected game ID. (ID6)
		 * @param gameID Game ID. (gamecode+company)
		 */
		void setSelGameID(const QString &gameID);

	public slots:
		void widget_keyPress(QKeyEvent *event);
		void widget_focusOut(QFocusEvent *event);
		void hd_destroyed(QObject *object);
};

#endif /* __MCRECOVER_SEARCHTHREAD_HPP__ */
