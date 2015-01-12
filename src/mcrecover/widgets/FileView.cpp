/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * FileView.cpp: File view widget.                                         *
 *                                                                         *
 * Copyright (c) 2012-2015 by David Korth.                                 *
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

#include "FileView.hpp"

#include "card/File.hpp"
#include "card/GcnFile.hpp" /* FIXME: Remove later */
#include "IconAnimHelper.hpp"

// XML template dialog.
#include "../windows/XmlTemplateDialog.hpp"
#include "../windows/XmlTemplateDialogManager.hpp"

// Testing...
#include "../edit/EditorWindow.hpp"

// Qt includes.
#include <QtCore/QTimer>

/** FileViewPrivate **/

#include "ui_FileView.h"
class FileViewPrivate
{
	public:
		FileViewPrivate(FileView *q);
		~FileViewPrivate();

	protected:
		FileView *const q_ptr;
		Q_DECLARE_PUBLIC(FileView)
	private:
		Q_DISABLE_COPY(FileViewPrivate)

	public:
		// UI
		Ui::FileView ui;

		const File *file;

		// Icon animation helper.
		IconAnimHelper helper;

		// Animation timer.
		QTimer animTimer;

		/**
		 * Update the widget display.
		 */
		void updateWidgetDisplay(void);

		/**
		 * XmlTemplateDialog manager.
		 */
		XmlTemplateDialogManager *xmlTemplateDialogManager;
};

FileViewPrivate::FileViewPrivate(FileView *q)
	: q_ptr(q)
	, file(nullptr)
	, xmlTemplateDialogManager(new XmlTemplateDialogManager(q))
{
	// Connect animTimer's timeout() signal.
	QObject::connect(&animTimer, SIGNAL(timeout()),
			 q, SLOT(animTimer_slot()));
}

FileViewPrivate::~FileViewPrivate()
{
	delete xmlTemplateDialogManager;
}

/**
 * Update the widget display.
 */
void FileViewPrivate::updateWidgetDisplay(void)
{
	Q_Q(FileView);

	if (!file) {
		// Clear the widget display.
		ui.lblFileIcon->clear();
		ui.lblFileBanner->clear();
		ui.btnXML->setVisible(false);
		ui.lblFilename->clear();
		ui.lblModeTitle->setVisible(false);
		ui.lblMode->setVisible(false);
		ui.lblChecksumAlgorithmTitle->setVisible(false);
		ui.lblChecksumAlgorithm->setVisible(false);
		ui.lblChecksumActualTitle->setVisible(false);
		ui.lblChecksumActual->setVisible(false);
		ui.lblChecksumExpectedTitle->setVisible(false);
		ui.lblChecksumExpected->setVisible(false);
		return;
	}

	// Set the widget display.

	// File icon.
	QPixmap icon = file->icon(0);
	if (!icon.isNull())
		ui.lblFileIcon->setPixmap(icon);
	else
		ui.lblFileIcon->clear();

	// Icon animation.
	helper.setFile(file);
	if (helper.isAnimated())
		animTimer.start(IconAnimHelper::FAST_ANIM_TIMER);

	// File banner.
	QPixmap banner = file->banner();
	if (!banner.isNull())
		ui.lblFileBanner->setPixmap(banner);
	else
		ui.lblFileBanner->clear();

	// XML button.
	ui.btnXML->setVisible(true);

	// Filename.
	ui.lblFilename->setText(file->filename());

	// File permissions.
	ui.lblModeTitle->setVisible(true);
	ui.lblMode->setText(file->modeAsString());
	ui.lblMode->setVisible(true);

	// Checksum algorithm is always visible.
	ui.lblChecksumAlgorithmTitle->setVisible(true);
	ui.lblChecksumAlgorithm->setVisible(true);

	// Is the checksum known?
	if (file->checksumStatus() == Checksum::CHKST_UNKNOWN) {
		// Unknown checksum.
		ui.lblChecksumAlgorithm->setText(FileView::tr("Unknown", "checksum"));
		ui.lblChecksumActualTitle->setVisible(false);
		ui.lblChecksumActual->setVisible(false);
		ui.lblChecksumExpectedTitle->setVisible(false);
		ui.lblChecksumExpected->setVisible(false);
		return;
	}

	// Checksum is known.

	// Display the algorithm.
	// TODO: Also the polynomial / parameter?
	Checksum::ChkAlgorithm algorithm = file->checksumAlgorithm();
	ui.lblChecksumAlgorithm->setText(
		QString::fromStdString(Checksum::ChkAlgorithmToStringFormatted(algorithm)));

	// Show the actual checksum labels.
	ui.lblChecksumActualTitle->setVisible(true);
	ui.lblChecksumActual->setVisible(true);

	// Format the checksum values.
	QVector<QString> checksumValuesFormatted = file->checksumValuesFormatted();
	if (checksumValuesFormatted.size() < 1) {
		// No checksum...
		ui.lblChecksumAlgorithm->setText(FileView::tr("Unknown", "checksum"));
		ui.lblChecksumActualTitle->setVisible(false);
		ui.lblChecksumActual->setVisible(false);
		ui.lblChecksumExpectedTitle->setVisible(false);
		ui.lblChecksumExpected->setVisible(false);
		return;
	}

	// Set the actual checksum text.
	ui.lblChecksumActual->setText(checksumValuesFormatted.at(0));

	if (checksumValuesFormatted.size() > 1) {
		// At least one checksum is invalid.
		// Show the expected checksum.
		ui.lblChecksumExpectedTitle->setVisible(true);
		ui.lblChecksumExpected->setVisible(true);
		ui.lblChecksumExpected->setText(checksumValuesFormatted.at(1));
	} else {
		// Checksums are all valid.
		// Hide the expected checksum.
		ui.lblChecksumExpectedTitle->setVisible(false);
		ui.lblChecksumExpected->setVisible(false);
		ui.lblChecksumExpected->clear();
	}
}

/** FileView **/

FileView::FileView(QWidget *parent)
	: QWidget(parent)
	, d_ptr(new FileViewPrivate(this))
{
	Q_D(FileView);
	d->ui.setupUi(this);

	// Fix alignment of the images.
	d->ui.hboxFileImages->setAlignment(d->ui.lblFileIcon, Qt::AlignTop);
	d->ui.hboxFileImages->setAlignment(d->ui.lblFileBanner, Qt::AlignTop);

	// Set monospace fonts.
	QFont fntMonospace;
	fntMonospace.setFamily(QLatin1String("Monospace"));
	fntMonospace.setStyleHint(QFont::TypeWriter);
	d->ui.lblMode->setFont(fntMonospace);

	fntMonospace.setBold(true);
	d->ui.lblChecksumActual->setFont(fntMonospace);
	d->ui.lblChecksumExpected->setFont(fntMonospace);

	d->updateWidgetDisplay();
}

FileView::~FileView()
{
	Q_D(FileView);
	delete d;
}

/**
 * Get the File being displayed.
 * @return File.
 */
const File *FileView::file(void) const
{
	Q_D(const FileView);
	return d->file;
}

/**
 * Set the File being displayed.
 * @param file File.
 */
void FileView::setFile(const File *file)
{
	Q_D(FileView);

	// Disconnect the File's destroyed() signal if a File is already set.
	if (d->file) {
		disconnect(d->file, SIGNAL(destroyed(QObject*)),
			   this, SLOT(file_destroyed_slot(QObject*)));
	}

	d->file = file;

	// Connect the File's destroyed() signal.
	if (d->file) {
		connect(d->file, SIGNAL(destroyed(QObject*)),
			this, SLOT(file_destroyed_slot(QObject*)));
	}

	// Update the widget display.
	d->updateWidgetDisplay();
}


/**
 * Widget state has changed.
 * @param event State change event.
 */
void FileView::changeEvent(QEvent *event)
{
	if (event->type() == QEvent::LanguageChange) {
		// Retranslate the UI.
		Q_D(FileView);
		d->ui.retranslateUi(this);
		d->updateWidgetDisplay();
	}

	// Pass the event to the base class.
	this->QWidget::changeEvent(event);
}

/** Slots. **/

/**
 * File object was destroyed.
 * @param obj QObject that was destroyed.
 */
void FileView::file_destroyed_slot(QObject *obj)
{
	Q_D(FileView);

	if (obj == d->file) {
		// Our File was destroyed.
		d->file = nullptr;

		// Update the widget display.
		d->updateWidgetDisplay();
	}
}


/**
 * Animation timer slot.
 */
void FileView::animTimer_slot(void)
{
	Q_D(FileView);

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
		d->ui.lblFileIcon->setPixmap(d->helper.icon());
	}
}

/**
 * XML button was pressed.
 */
void FileView::on_btnXML_clicked(void)
{
	// TODO: Handle other types of files?
	Q_D(FileView);
	const GcnFile *gcnFile = qobject_cast<const GcnFile*>(d->file);
	if (gcnFile) {
		XmlTemplateDialog *dialog = d->xmlTemplateDialogManager->create(gcnFile, this);
		dialog->show();
		dialog->activateWindow();
	}
}

/**
 * Edit button was pressed.
 */
void FileView::on_btnEdit_clicked(void)
{
	// Testing...
	Q_D(FileView);
	const GcnFile *gcnFile = qobject_cast<const GcnFile*>(d->file);
	if (gcnFile) {
		// FIXME: File has to be changed to non-const...
		EditorWindow *editor = EditorWindow::editGcnFile((GcnFile*)gcnFile);
		if (editor) {
			editor->exec();
			delete editor;
		}
	}
}
