/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * FormatNewMemCardDialog.cpp: Format New Memory Card Image dialog.        *
 *                                                                         *
 * Copyright (c) 2015-2016 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#include "FormatNewMemCardDialog.hpp"

/** FormatNewMemCardDialogPrivate **/

#include "ui_FormatNewMemCardDialog.h"
class FormatNewMemCardDialogPrivate
{
	public:
		explicit FormatNewMemCardDialogPrivate(FormatNewMemCardDialog *q);

	protected:
		FormatNewMemCardDialog *const q_ptr;
		Q_DECLARE_PUBLIC(FormatNewMemCardDialog)
	private:
		Q_DISABLE_COPY(FormatNewMemCardDialogPrivate)

	public:
		Ui::FormatNewMemCardDialog ui;

		/**
		 * Update the slider's size display.
		 * @param value Slider value.
		 */
		void updateSldSizeDisplay(int value);

		/**
		 * Update the slider's size display.
		 * Uses the current sliderPosition().
		 */
		void updateSldSizeDisplay(void);
};

FormatNewMemCardDialogPrivate::FormatNewMemCardDialogPrivate(FormatNewMemCardDialog* q)
	: q_ptr(q)
{ }

/**
 * Update the slider's size display.
 * @param value Slider value.
 */
void FormatNewMemCardDialogPrivate::updateSldSizeDisplay(int value)
{
	const int sz_blocks = ((64 << value) - 5);
	ui.lblSizeValue->setText(FormatNewMemCardDialog::tr("%Ln block(s)", "", sz_blocks));

	// Show a warning for >251 blocks.
	if (value > 2) {
		ui.lblSizeValue->setStyleSheet(QLatin1String(
			"QLabel { font-weight: bold; color: red; }"));
	} else {
		ui.lblSizeValue->setStyleSheet(QString());
	}
}

/**
 * Update the slider's size display.
 * Uses the current sliderPosition().
 */
void FormatNewMemCardDialogPrivate::updateSldSizeDisplay(void)
{
	updateSldSizeDisplay(ui.sldSize->sliderPosition());
}

/** FormatNewMemCardDialog **/

/**
 * Initialize the Format New Memory Card Image dialog.
 * @param parent Parent widget.
 */
FormatNewMemCardDialog::FormatNewMemCardDialog(QWidget *parent)
	: super(parent,
		Qt::Dialog |
		Qt::CustomizeWindowHint |
		Qt::WindowTitleHint |
		Qt::WindowSystemMenuHint |
		Qt::WindowCloseButtonHint)
	, d_ptr(new FormatNewMemCardDialogPrivate(this))
{
	init();
}

/**
 * Common initialization function for all constructors.
 */
void FormatNewMemCardDialog::init(void)
{
	Q_D(FormatNewMemCardDialog);
	d->ui.setupUi(this);

	// Make sure the window is deleted on close.
	this->setAttribute(Qt::WA_DeleteOnClose, true);

#ifdef Q_OS_MAC
	// Remove the window icon. (Mac "proxy icon")
	this->setWindowIcon(QIcon());
#endif

	// Update the slider's size display.
	d->updateSldSizeDisplay();
}

/**
 * Shut down the Format New Memory Card Image dialog.
 */
FormatNewMemCardDialog::~FormatNewMemCardDialog()
{
	delete d_ptr;
}

/**
 * Widget state has changed.
 * @param event State change event.
 */
void FormatNewMemCardDialog::changeEvent(QEvent *event)
{
	if (event->type() == QEvent::LanguageChange) {
		// Retranslate the UI.
		Q_D(FormatNewMemCardDialog);
		d->ui.retranslateUi(this);

		// Update the slider's size display.
		d->updateSldSizeDisplay();
	}

	// Pass the event to the base class.
	super::changeEvent(event);
}

void FormatNewMemCardDialog::on_sldSize_sliderMoved(int value)
{
	Q_D(FormatNewMemCardDialog);
	d->updateSldSizeDisplay(value);
}

void FormatNewMemCardDialog::on_sldSize_valueChanged(int value)
{
	Q_D(FormatNewMemCardDialog);
	d->updateSldSizeDisplay(value);
}
