/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * HerpDerpEggListener.hpp: Listener for... something. (shh)               *
 *                                                                         *
 * Copyright (c) 2014-2025 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#pragma once

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
	explicit HerpDerpEggListener(QObject *parent = 0);
	~HerpDerpEggListener();

protected:
	HerpDerpEggListenerPrivate *const d_ptr;
	Q_DECLARE_PRIVATE(HerpDerpEggListener)
private:
	Q_DISABLE_COPY(HerpDerpEggListener)

public:
	/**
	 * Set the selected game ID. (ID6)
	 * @param gameID Game ID (gamecode+company)
	 */
	void setSelGameID(const QString &gameID);

public slots:
	void widget_keyPress(QKeyEvent *event);
	void widget_focusOut(QFocusEvent *event);
	void hd_destroyed(QObject *object);
};
