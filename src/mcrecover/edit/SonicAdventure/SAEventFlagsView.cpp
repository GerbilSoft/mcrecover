/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * SAEventFlagsView.cpp: Sonic Adventure - Event Flags editor.             *
 *                                                                         *
 * Copyright (c) 2015 by David Korth.                                      *
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

#include "SAEventFlagsView.hpp"

#include "SAEventFlags.hpp"
#include "SAEventFlagsModel.hpp"

// C includes. (C++ namespace)
#include <cassert>

// Sonic Adventure save file definitions.
#include "sa_defs.h"

// TODO: Put this in a common header file somewhere.
#define NUM_ELEMENTS(x) ((int)(sizeof(x) / sizeof(x[0])))

/** SAEventFlagsViewPrivate **/

#include "ui_SAEventFlagsView.h"
class SAEventFlagsViewPrivate
{
	public:
		SAEventFlagsViewPrivate(SAEventFlagsView *q);

	protected:
		SAEventFlagsView *const q_ptr;
		Q_DECLARE_PUBLIC(SAEventFlagsView)
	private:
		Q_DISABLE_COPY(SAEventFlagsViewPrivate)

	public:
		Ui_SAEventFlagsView ui;

		// Event Flags model.
		SAEventFlagsModel *eventFlagsModel;

		// Event Flags data.
		SAEventFlags eventFlags;
};

SAEventFlagsViewPrivate::SAEventFlagsViewPrivate(SAEventFlagsView *q)
	: q_ptr(q)
{ }

/** SAEventFlagsView **/

SAEventFlagsView::SAEventFlagsView(QWidget *parent)
	: QWidget(parent)
	, d_ptr(new SAEventFlagsViewPrivate(this))
{
	Q_D(SAEventFlagsView);
	d->ui.setupUi(this);

	// Initialize the event flags.
	d->eventFlagsModel = new SAEventFlagsModel(this);
	d->eventFlagsModel->setEventFlags(&d->eventFlags);
	d->ui.lstEventFlags->setModel(d->eventFlagsModel);
}

SAEventFlagsView::~SAEventFlagsView()
{
	Q_D(SAEventFlagsView);
	delete d;
}

/** Data access. **/

/**
 * Get the event flags from the widget.
 * Flags are copied from the widget.
 * @param eventFlags sa_event_flags to store the event flags in.
 */
void SAEventFlagsView::eventFlags(sa_event_flags *eventFlags) const
{
	Q_D(const SAEventFlagsView);
	uint8_t *flagByte = &eventFlags->all[0];
	assert(d->eventFlags.count() == (NUM_ELEMENTS(eventFlags->all) * 8));

	// TODO: Make sure this isn't backwards.
	for (int i = 0; i < d->eventFlags.count(); i++) {
		if (i % 8 == 0) {
			// New byte.
			// TODO: Optimize this?
			if (i > 0)
				flagByte++;
			*flagByte = 0;
		}

		// Get this flag.
		*flagByte |= !!(d->eventFlags.flag(i));
	}
}

/**
 * Set the event flags for the widget.
 * Flags are copied into the widget.
 * @param eventFlags sa_event_flags to store in the widget.
 */
void SAEventFlagsView::setEventFlags(const _sa_event_flags *eventFlags)
{
	Q_D(SAEventFlagsView);
	const uint8_t *flagByte = &eventFlags->all[0];
	assert(d->eventFlags.count() == (NUM_ELEMENTS(eventFlags->all) * 8));

	// TODO: Make sure this isn't backwards.
	uint8_t curByte = 0;
	for (int i = 0; i < d->eventFlags.count(); i++) {
		if (i % 8 == 0) {
			// New byte.
			curByte = *flagByte++;
		}

		// Set this flag.
		d->eventFlags.setFlag(i, !!(curByte & 0x80));
		curByte >>= 1;
	}
}
