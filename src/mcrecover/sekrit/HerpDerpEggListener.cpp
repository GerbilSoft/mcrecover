/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * HerpDerpEggListener.cpp: Listener for... something. (shh)               *
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

#include "HerpDerpEggListener.hpp"
#include "util/array_size.h"

// C includes.
#include <string.h>

// Qt includes.
#include <QtGui/QKeyEvent>
#include <QtGui/QApplication>
#include <QtGui/QDesktopWidget>

/** HerpDerpEggListenerPrivate **/

class HerpDerpEggListenerPrivate
{
	public:
		HerpDerpEggListenerPrivate(HerpDerpEggListener *q);
		~HerpDerpEggListenerPrivate();

	protected:
		HerpDerpEggListener *const q_ptr;
		Q_DECLARE_PUBLIC(HerpDerpEggListener)
	private:
		Q_DISABLE_COPY(HerpDerpEggListenerPrivate)

	public:
		HackDetection::DetectType detectType;
		int seq_pos;

		// Set of HackDetection objects.
		// NOTE: Must be QObject, since we can't use
		// qobject_cast<> in the destroyed() slot.
		QSet<QObject*> hdSet;

		/**
		 * Do "Hack Detection" on all monitors.
		 */
		void doHackDetection(void);
};

HerpDerpEggListenerPrivate::HerpDerpEggListenerPrivate(HerpDerpEggListener *q)
	: q_ptr(q)
	, detectType(HackDetection::DT_NONE)
	, seq_pos(0)
{ }	

HerpDerpEggListenerPrivate::~HerpDerpEggListenerPrivate()
{
	QSet<QObject*> hdSet_del;
	hdSet_del.swap(hdSet);
	qDeleteAll(hdSet_del);
}

/**
 * Do "Hack Detection" on all monitors.
 */
void HerpDerpEggListenerPrivate::doHackDetection(void)
{
	Q_Q(HerpDerpEggListener);
	QWidget *parent = qobject_cast<QWidget*>(q->parent());

	seq_pos = 0;
	QDesktopWidget *desktop = QApplication::desktop();
	for (int i = 0; i < desktop->numScreens(); i++) {
		HackDetection *hd = new HackDetection(i, parent);
		QObject::connect(hd, SIGNAL(destroyed(QObject*)),
				 q, SLOT(hd_destroyed(QObject*)));
		hdSet.insert(hd);
		hd->show();
	}
}

/** HerpDerpEggListener **/

HerpDerpEggListener::HerpDerpEggListener(QObject *parent)
	: QObject(parent)
	, d_ptr(new HerpDerpEggListenerPrivate(this))
{ }

HerpDerpEggListener::~HerpDerpEggListener()
{
	Q_D(HerpDerpEggListener);
	delete d;
}

/**
 * Set the selected game ID.
 * @param gameID Game ID. (gamecode+company)
 */
void HerpDerpEggListener::setSelGameID(const QString &gameID)
{
	Q_D(HerpDerpEggListener);
	d->detectType = HackDetection::DT_NONE;
	d->seq_pos = 0;
	if (gameID.size() != 6) {
		// Invalid game ID.
		return;
	}

	const QByteArray str_data = gameID.toLatin1();
	const char *str = str_data.constData();

	struct hurrMatch_t {
		char gameID[7];
		uint8_t dt;
	};

	static const hurrMatch_t hurrMatch[] = {
		/** HACK DETECTION **/
		{"G2XE8P", HackDetection::DT_H},
		{"G2XP8P", HackDetection::DT_H},
		{"G9SE8P", HackDetection::DT_H},
		{"G9SJ8P", HackDetection::DT_H},
		{"G9SP8P", HackDetection::DT_H},
		{"GSBJ8P", HackDetection::DT_H},
		{"GSNE8P", HackDetection::DT_H},
		{"GSNP8P", HackDetection::DT_H},
		{"GSOE8P", HackDetection::DT_H},
		{"GSOJ8P", HackDetection::DT_H},
		{"GSOP8P", HackDetection::DT_H},
		{"GXEE8P", HackDetection::DT_H},
		{"GXEJ8P", HackDetection::DT_H},
		{"GXEP8P", HackDetection::DT_H},
		{"GXSE8P", HackDetection::DT_H},
		{"GXSP6W", HackDetection::DT_H},
		{"GXSP8P", HackDetection::DT_H},
		{"RELSAB", HackDetection::DT_H},
		/** QUACK DETECTION **/
		{"GDDE41", HackDetection::DT_Q},
		{"GDDP41", HackDetection::DT_Q},
		{"GDOP41", HackDetection::DT_Q},
		/** SNACK DETECTION **/
		{"GKYE01", HackDetection::DT_S},
		{"GKYJ01", HackDetection::DT_S},
		{"GKYP01", HackDetection::DT_S},
	};

	for (int i = (ARRAY_SIZE(hurrMatch)-1); i >= 0; i--) {
		if (!strcmp(str, hurrMatch[i].gameID)) {
			d->detectType = (HackDetection::DetectType)hurrMatch[i].dt;
			break;
		}
	}
}

/**
 * Key press listener.
 * @param event Key press event.
 */
void HerpDerpEggListener::widget_keyPress(QKeyEvent *event)
{
	// Key sequence.
	static const uint8_t hack_seq[] = {
		Qt::Key_1, Qt::Key_9,
		Qt::Key_6, Qt::Key_5,
		Qt::Key_0, Qt::Key_9,
		Qt::Key_1, Qt::Key_7
	};

	Q_D(HerpDerpEggListener);
	if (d->detectType <= HackDetection::DT_NONE ||
	    d->detectType >= HackDetection::DT_MAX) {
		// Invalid detection.
		d->seq_pos = 0;
		return;
	}

	if (d->seq_pos < 0 || d->seq_pos >= ARRAY_SIZE(hack_seq))
		d->seq_pos = 0;

	if (event->key() == (int)hack_seq[d->seq_pos]) {
		d->seq_pos++;
		if (d->seq_pos == ARRAY_SIZE(hack_seq)) {
			d->seq_pos = 0;
			d->doHackDetection();
		}
	} else {
		d->seq_pos = 0;
	}
}

/**
 * HackDetection destroyed slot.
 * @param object HackDetection.
 */
void HerpDerpEggListener::hd_destroyed(QObject *object)
{
	Q_D(HerpDerpEggListener);
	if (d->hdSet.contains(object)) {
		// Delete all HackDetections.
		QSet<QObject*> hdSet_del;
		hdSet_del.swap(d->hdSet);
		hdSet_del.remove(object);
		qDeleteAll(hdSet_del);
	}
}
