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
	, file(nullptr)
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
		q->lblChecksumAlgorithmTitle->setVisible(false);
		q->lblChecksumAlgorithm->setVisible(false);
		q->lblChecksumActualTitle->setVisible(false);
		q->lblChecksumActual->setVisible(false);
		q->lblChecksumExpectedTitle->setVisible(false);
		q->lblChecksumExpected->setVisible(false);
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

	// Checksum algorithm is always visible.
	q->lblChecksumAlgorithmTitle->setVisible(true);
	q->lblChecksumAlgorithm->setVisible(true);

	// Is the checksum known?
	if (file->checksumStatus() == Checksum::CHKST_UNKNOWN) {
		// Unknown checksum.
		q->lblChecksumAlgorithm->setText(q->tr("Unknown", "checksum"));
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
	q->lblChecksumAlgorithm->setText(Checksum::ChkAlgorithmToStringFormatted(algorithm));

	// Show the actual checksum labels.
	q->lblChecksumActualTitle->setVisible(true);
	q->lblChecksumActual->setVisible(true);

	// Format the checksum values.
	QVector<QString> checksumValuesFormatted = file->checksumValuesFormatted();
	if (checksumValuesFormatted.size() < 1) {
		// No checksum...
		q->lblChecksumAlgorithm->setText(q->tr("Unknown", "checksum"));
		q->lblChecksumActualTitle->setVisible(false);
		q->lblChecksumActual->setVisible(false);
		q->lblChecksumExpectedTitle->setVisible(false);
		q->lblChecksumExpected->setVisible(false);
		return;
	}

	// Set the actual checksum text.
	q->lblChecksumActual->setText(checksumValuesFormatted.at(0));

	if (checksumValuesFormatted.size() > 1) {
		// At least one checksum is invalid.
		// Show the expected checksum.
		q->lblChecksumExpectedTitle->setVisible(true);
		q->lblChecksumExpected->setVisible(true);
		q->lblChecksumExpected->setText(checksumValuesFormatted.at(1));
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

	// Set monospace fonts.
	QFont fntMonospace;
	fntMonospace.setFamily(QLatin1String("Monospace"));
	fntMonospace.setStyleHint(QFont::TypeWriter);
	lblMode->setFont(fntMonospace);

	fntMonospace.setBold(true);
	lblChecksumActual->setFont(fntMonospace);
	lblChecksumExpected->setFont(fntMonospace);

	d->updateWidgetDisplay();
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


/**
 * Widget state has changed.
 * @param event State change event.
 */
void MemCardFileView::changeEvent(QEvent *event)
{
	if (event->type() == QEvent::LanguageChange) {
		// Retranslate the UI.
		retranslateUi(this);
		d->updateWidgetDisplay();
	}

	// Pass the event to the base class.
	this->QWidget::changeEvent(event);
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
		d->file = nullptr;

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
