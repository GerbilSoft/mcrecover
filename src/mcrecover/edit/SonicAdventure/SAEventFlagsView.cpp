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

//#include "SAEventFlags.hpp" // TODO
#include "SAEventFlagsModel.hpp"

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
	d->ui.lstEventFlags->setModel(d->eventFlagsModel);
}

SAEventFlagsView::~SAEventFlagsView()
{
	Q_D(SAEventFlagsView);
	delete d;
}
