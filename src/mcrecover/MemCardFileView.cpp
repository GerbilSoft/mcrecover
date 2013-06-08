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
#include "IconAnimHelper.hpp"

// Qt includes.
#include <QtCore/QTimer>


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

		// Icon animation helper.
		IconAnimHelper helper;

		// Animation timer.
		QTimer animTimer;

		/**
		 * Update the widget display.
		 */
		void updateWidgetDisplay(void);
};

MemCardFileViewPrivate::MemCardFileViewPrivate(MemCardFileView *q)
	: q(q)
	, file(NULL)
{
	// Connect animTimer's timeout() signal.
	QObject::connect(&animTimer, SIGNAL(timeout()),
			 q, SLOT(animTimer_slot()));
}

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
		q->lblModeTitle->setVisible(false);
		q->lblMode->setVisible(false);
		return;
	}

	// Set the widget display.

	// File icon.
	QPixmap icon = file->icon(0);
	if (!icon.isNull())
		q->lblFileIcon->setPixmap(icon);
	else
		q->lblFileIcon->clear();

	// Icon animation.
	helper.setFile(file);
	if (helper.isAnimated())
		animTimer.start(IconAnimHelper::FAST_ANIM_TIMER);

	// File banner.
	QPixmap banner = file->banner();
	if (!banner.isNull())
		q->lblFileBanner->setPixmap(banner);
	else
		q->lblFileBanner->clear();

	// Filename.
	q->lblFilename->setText(file->filename());

	// File permissions.
	q->lblModeTitle->setVisible(true);
	q->lblMode->setText(file->permissionAsString());
	q->lblMode->setVisible(true);

	// Checksum colors.
	// TODO: Better colors?
	static const QString s_chkHtmlUnknown = QLatin1String("<span style='color: #880'>%1</span>");
	static const QString s_chkHtmlGood = QLatin1String("<span style='color: #080'>%1</span>");
	static const QString s_chkHtmlInvalid = QLatin1String("<span style='color: #F00'>%1</span>");

	// Checksums.
	if (file->checksumStatus() == Checksum::CHKST_UNKNOWN) {
		// Unknown checksum.
		q->lblChecksum->setText(s_chkHtmlUnknown.arg(q->tr("Unknown")));
		q->lblChecksumExpectedTitle->setVisible(false);
		q->lblChecksumExpected->setVisible(false);
		return;
	}

	// Checksum is known.
	const QVector<Checksum::ChecksumValue> checksumValues = file->checksumValues();
	const int fieldWidth = file->checksumFieldWidth();

	for (int i = 0; i < checksumValues.size(); i++) {
		const Checksum::ChecksumValue &value = checksumValues.at(i);

		char s_chkActual[12];
		if (fieldWidth <= 4)
			snprintf(s_chkActual, sizeof(s_chkActual), "%04X", value.actual);
		else
			snprintf(s_chkActual, sizeof(s_chkActual), "%08X", value.actual);

		// Check if the checksum is valid.
		if (value.actual == value.expected) {
			// Checksum is valid.
			q->lblChecksum->setText(s_chkHtmlGood.arg(QLatin1String(s_chkActual)));

			// Hide the expected checksum.
			q->lblChecksumExpectedTitle->setVisible(false);
			q->lblChecksumExpected->setVisible(false);
		} else {
			// Checksum is invalid.
			q->lblChecksum->setText(s_chkHtmlInvalid.arg(QLatin1String(s_chkActual)));

			// Set the expected checksum.
			char s_chkExpected[12];
			if (fieldWidth <= 4)
				snprintf(s_chkExpected, sizeof(s_chkExpected), "%04X", value.expected);
			else
				snprintf(s_chkExpected, sizeof(s_chkExpected), "%08X", value.expected);
			q->lblChecksumExpected->setText(QLatin1String(s_chkExpected));

			// Show the expected checksum.
			q->lblChecksumExpectedTitle->setVisible(true);
			q->lblChecksumExpected->setVisible(true);
		}
	}
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


/**
 * Animation timer slot.
 */
void MemCardFileView::animTimer_slot(void)
{
	if (!d->file || !d->helper.isAnimated()) {
		// No file is loaded, or the file doesn't have an animated icon.
		// Stop the animation timer.
		d->animTimer.stop();
		return;
	}

	// Check if the animated icon should be updated.
	bool iconUpdated = d->helper.tick();
	if (iconUpdated) {
		// Icon has been updated.
		lblFileIcon->setPixmap(d->helper.icon());
	}
}
