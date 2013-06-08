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
	static const QString s_chkHtmlGood = QLatin1String("<span style='color: #080'>%1</span>");
	static const QString s_chkHtmlInvalid = QLatin1String("<span style='color: #F00'>%1</span>");
	static const QString s_chkHtmlLinebreak = QLatin1String("<br/>");

	// Checksums.
	if (file->checksumStatus() == Checksum::CHKST_UNKNOWN) {
		// Unknown checksum.
		q->lblChecksumAlgorithm->setText(q->tr("Unknown"));
		q->lblChecksumActualTitle->setVisible(false);
		q->lblChecksumActual->setVisible(false);
		q->lblChecksumExpectedTitle->setVisible(false);
		q->lblChecksumExpected->setVisible(false);
		return;
	}

	// Checksum is known.

	// Display the algorithm.
	// TODO: Also the polynomial / parameter?
	Checksum::ChkAlgorithm algorithm = file->checksumAlgorithm();
	q->lblChecksumAlgorithm->setText(Checksum::ChkAlgorithmToString(algorithm));

	// Show the actual checksum labels.
	q->lblChecksumActualTitle->setVisible(true);
	q->lblChecksumActual->setVisible(true);

	// Get the checksum values.
	const QVector<Checksum::ChecksumValue> checksumValues = file->checksumValues();
	const int fieldWidth = file->checksumFieldWidth();
	const int reserveSize = ((s_chkHtmlGood.length() + fieldWidth + 5) * checksumValues.size());

	QString s_chkActual_all; s_chkActual_all.reserve(reserveSize);
	QString s_chkExpected_all;
	if (file->checksumStatus() == Checksum::CHKST_INVALID)
		s_chkExpected_all.reserve(reserveSize);

	for (int i = 0; i < checksumValues.size(); i++) {
		const Checksum::ChecksumValue &value = checksumValues.at(i);

		if (i > 0) {
			// Add linebreaks or spaces to the checksum strings.
			if ((i % 2) && fieldWidth <= 4) {
				// Odd checksum index, 16-bit checksum.
				// Add a space.
				s_chkActual_all += QChar(L' ');
				s_chkExpected_all += QChar(L' ');
			} else {
				// Add a linebreak.
				s_chkActual_all += s_chkHtmlLinebreak;
				s_chkExpected_all += s_chkHtmlLinebreak;
			}
		}

		char s_chkActual[12];
		char s_chkExpected[12];
		if (fieldWidth <= 4) {
			snprintf(s_chkActual, sizeof(s_chkActual), "%04X", value.actual);
			snprintf(s_chkExpected, sizeof(s_chkExpected), "%04X", value.expected);
		} else {
			snprintf(s_chkActual, sizeof(s_chkActual), "%08X", value.actual);
			snprintf(s_chkExpected, sizeof(s_chkExpected), "%08X", value.expected);
		}

		// Check if the checksum is valid.
		if (value.actual == value.expected) {
			// Checksum is valid.
			s_chkActual_all += s_chkHtmlGood.arg(QLatin1String(s_chkActual));
			if (file->checksumStatus() == Checksum::CHKST_INVALID)
				s_chkExpected_all += s_chkHtmlGood.arg(QLatin1String(s_chkExpected));
		} else {
			// Checksum is invalid.
			s_chkActual_all += s_chkHtmlInvalid.arg(QLatin1String(s_chkActual));
			if (file->checksumStatus() == Checksum::CHKST_INVALID)
				s_chkExpected_all += s_chkHtmlInvalid.arg(QLatin1String(s_chkExpected));
		}
	}

	// Set the actual checksum text.
	q->lblChecksumActual->setText(s_chkActual_all);

	if (file->checksumStatus() == Checksum::CHKST_INVALID) {
		// At least one checksum is invalid.
		// Show the expected checksum.
		q->lblChecksumExpectedTitle->setVisible(true);
		q->lblChecksumExpected->setVisible(true);
		q->lblChecksumExpected->setText(s_chkExpected_all);
	} else {
		// Checksums are all valid.
		// Hide the expected checksum.
		q->lblChecksumExpectedTitle->setVisible(false);
		q->lblChecksumExpected->setVisible(false);
		q->lblChecksumExpected->clear();
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
