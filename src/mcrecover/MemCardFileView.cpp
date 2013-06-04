/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * MemCardFileView.cpp: MemCardFile view widget.                           *
 *                                                                         *
 * Copyright (c) 2012-2013 by David Korth.                                 *
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

#include "MemCardFileView.hpp"

#include "MemCardFile.hpp"


/** MemCardFileViewPrivate **/

class MemCardFileViewPrivate
{
	public:
		MemCardFileViewPrivate(MemCardFileView *q);
		~MemCardFileViewPrivate();

	private:
		MemCardFileView *const q;
		Q_DISABLE_COPY(MemCardFileViewPrivate);

	public:
		const MemCardFile *file;

		/**
		 * Update the widget display.
		 */
		void updateWidgetDisplay(void);
};

MemCardFileViewPrivate::MemCardFileViewPrivate(MemCardFileView *q)
	: q(q)
	, file(NULL)
{ }

MemCardFileViewPrivate::~MemCardFileViewPrivate()
{ }


/**
 * Update the widget display.
 */
void MemCardFileViewPrivate::updateWidgetDisplay(void)
{
	if (!file) {
		// Clear the widget display.
		q->lblFileIcon->clear();
		q->lblFileBanner->clear();
		q->lblFilename->clear();
		return;
	}

	// Set the widget display.

	// File icon.
	// TODO: Icon animation. (Synchronized with QTreeView?)
	QPixmap icon = file->icon(0);
	if (!icon.isNull()) {
		q->lblFileIcon->setPixmap(icon);
		q->lblFileIcon->setVisible(true);
	} else {
		q->lblFileIcon->setVisible(false);
		q->lblFileIcon->clear();
	}

	// File banner.
	QPixmap banner = file->banner();
	if (!banner.isNull()) {
		q->lblFileBanner->setPixmap(banner);
		q->lblFileBanner->setVisible(true);
	} else {
		q->lblFileBanner->setVisible(false);
		q->lblFileBanner->clear();
	}

	// Filename.
	q->lblFilename->setText(file->filename());
}


/** McRecoverWindow **/

MemCardFileView::MemCardFileView(QWidget *parent)
	: QWidget(parent)
	, d(new MemCardFileViewPrivate(this))
{
	setupUi(this);
}

MemCardFileView::~MemCardFileView()
{
	delete d;
}


/**
 * Get the MemCardFile being displayed.
 * @return MemCardFile.
 */
const MemCardFile *MemCardFileView::file(void) const
	{ return d->file; }

/**
 * Set the MemCardFile being displayed.
 * @param file MemCardFile.
 */
void MemCardFileView::setFile(const MemCardFile *file)
{
	// Disconnect the MemCardFile's destroyed() signal if a MemCardFile is already set.
	if (d->file) {
		disconnect(d->file, SIGNAL(destroyed(QObject*)),
			   this, SLOT(memCardFile_destroyed_slot(QObject*)));
	}

	d->file = file;

	// Connect the MemCardFile's destroyed() signal.
	if (d->file) {
		connect(d->file, SIGNAL(destroyed(QObject*)),
			this, SLOT(memCardFile_destroyed_slot(QObject*)));
	}

	// Update the widget display.
	d->updateWidgetDisplay();
}


/** Slots. **/


/**
 * MemCardFile object was destroyed.
 * @param obj QObject that was destroyed.
 */
void MemCardFileView::memCardFile_destroyed_slot(QObject *obj)
{
	if (obj == d->file) {
		// Our MemCardFile was destroyed.
		d->file = NULL;

		// Update the widget display.
		d->updateWidgetDisplay();
	}
}
