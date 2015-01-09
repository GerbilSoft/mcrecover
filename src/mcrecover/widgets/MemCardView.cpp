/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * MemCardView.hpp: MemCard view widget.                                   *
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

#include "MemCardView.hpp"

#include "card/MemCard.hpp"
#include "Checksum.hpp"
#include "McRecoverQApplication.hpp"

// C includes.
#include <stdlib.h>

// C++ includes.
#include <string>
#include <vector>
using std::string;
using std::vector;

/** MemCardViewPrivate **/

#include "ui_MemCardView.h"
class MemCardViewPrivate
{
	public:
		MemCardViewPrivate(MemCardView *q);
		~MemCardViewPrivate();

	protected:
		MemCardView *const q_ptr;
		Q_DECLARE_PUBLIC(MemCardView)
	private:
		Q_DISABLE_COPY(MemCardViewPrivate)

	public:
		Ui::MemCardView ui;

		MemCard *card;

		/**
		 * Update the widget display.
		 */
		void updateWidgetDisplay(void);

		/**
		 * Update the block count display.
		 */
		void updateBlockCountDisplay(void);
};

MemCardViewPrivate::MemCardViewPrivate(MemCardView *q)
	: q_ptr(q)
	, card(nullptr)
{ }

MemCardViewPrivate::~MemCardViewPrivate()
{ }

/**
 * Update the block count display.
 */
void MemCardViewPrivate::updateBlockCountDisplay(void)
{
	int sizeInBlocksNoSys = card->sizeInBlocksNoSys();
	ui.lblBlockCount->setText(
		MemCardView::tr("%L1 block(s) (%L2 free)", "", sizeInBlocksNoSys)
			.arg(sizeInBlocksNoSys)
			.arg(card->freeBlocks()));
}

/**
 * Update the widget display.
 */
void MemCardViewPrivate::updateWidgetDisplay(void)
{
	if (!card) {
		// Hide the widget display.
		// TODO: Better method?
		ui.lblBlockCount->setVisible(false);
		ui.lblStatusIcon->setVisible(false);
		ui.lblEncodingTitle->setVisible(false);
		ui.lblEncoding->setVisible(false);
		ui.lblChecksumActualTitle->setVisible(false);
		ui.lblChecksumActual->setVisible(false);
		ui.lblChecksumExpectedTitle->setVisible(false);
		ui.lblChecksumExpected->setVisible(false);
		ui.tableSelect->setCard(nullptr);
		ui.tableSelect->setVisible(false);
		return;
	}

	// Show the widget display.
	ui.lblBlockCount->setVisible(true);
	ui.lblEncodingTitle->setVisible(true);
	ui.lblEncoding->setVisible(true);
	ui.lblChecksumActualTitle->setVisible(true);
	ui.lblChecksumActual->setVisible(true);

	// Update the widget display.
	bool isCardHeaderValid = true;

	// Format the header checksum.
	vector<Checksum::ChecksumValue> checksumValues;
	checksumValues.push_back(card->headerChecksumValue());
	vector<string> checksumValuesFormatted = Checksum::ChecksumValuesFormatted(checksumValues);
	if (checksumValuesFormatted.size() < 1) {
		// No checksum...
		ui.lblChecksumActual->setText(MemCardView::tr("Unknown", "checksum"));
		ui.lblChecksumExpectedTitle->setVisible(false);
		ui.lblChecksumExpected->setVisible(false);
	} else {
		// Set the actual checksum text.
		ui.lblChecksumActual->setText(
			QString::fromStdString(checksumValuesFormatted.at(0)));

		if (checksumValuesFormatted.size() > 1) {
			// At least one checksum is invalid.
			isCardHeaderValid = false;
			// Show the expected checksum.
			ui.lblChecksumExpectedTitle->setVisible(true);
			ui.lblChecksumExpected->setVisible(true);
			ui.lblChecksumExpected->setText(
				QString::fromStdString(checksumValuesFormatted.at(1)));
		} else {
			// Checksums are all valid.
			// Hide the expected checksum.
			ui.lblChecksumExpectedTitle->setVisible(false);
			ui.lblChecksumExpected->setVisible(false);
			ui.lblChecksumExpected->clear();
		}
	}

	// Validate some other aspects of the card header.
	if (isCardHeaderValid) {
		if (card->freeBlocks() > card->sizeInBlocksNoSys()) {
			// Free blocks count is wrong.
			isCardHeaderValid = false;
		}
		// TODO: Other aspects.
	}

	// Block count.
	updateBlockCountDisplay();

	// Status icon.
	if (isCardHeaderValid) {
		// Card header is valid.
		ui.lblStatusIcon->setVisible(false);
	} else {
		// Card header is invalid.
		QIcon icon = McRecoverQApplication::IconFromTheme(QLatin1String("dialog-error"));
		// TODO: What size?
		ui.lblStatusIcon->setPixmap(icon.pixmap(16, 16));
		ui.lblStatusIcon->setToolTip(MemCardView::tr("Memory card header is corrupted."));
		ui.lblStatusIcon->setVisible(true);
	}

	// Encoding.
	QString encoding;
	switch (card->encoding()) {
		case SYS_FONT_ENCODING_SJIS:
			encoding = QLatin1String("Shift-JIS");
			break;

		case SYS_FONT_ENCODING_ANSI:
		default:
			encoding = QLatin1String("cp1252");
			break;
	}
	ui.lblEncoding->setText(encoding);

	// Show the dir/block table select widget.
	ui.tableSelect->setCard(card);
	ui.tableSelect->setVisible(true);
}

/** MemCardFileView **/

MemCardView::MemCardView(QWidget *parent)
	: QWidget(parent)
	, d_ptr(new MemCardViewPrivate(this))
{
	Q_D(MemCardView);
	d->ui.setupUi(this);

	// Set monospace fonts.
	QFont fntMonospace;
	fntMonospace.setFamily(QLatin1String("Monospace"));
	fntMonospace.setStyleHint(QFont::TypeWriter);
	fntMonospace.setBold(true);
	d->ui.lblChecksumActual->setFont(fntMonospace);
	d->ui.lblChecksumExpected->setFont(fntMonospace);

	d->updateWidgetDisplay();
}

MemCardView::~MemCardView()
{
	Q_D(MemCardView);
	delete d;
}

/**
 * Get the MemCard being displayed.
 * @return MemCard.
 */
MemCard *MemCardView::card(void) const
{
	Q_D(const MemCardView);
	return d->card;
}

/**
 * Set the MemCard being displayed.
 * @param card MemCard.
 */
void MemCardView::setCard(MemCard *card)
{
	Q_D(MemCardView);

	// Disconnect the MemCard's destroyed() signal if a MemCard is already set.
	if (d->card) {
		disconnect(d->card, SIGNAL(destroyed(QObject*)),
			   this, SLOT(memCard_destroyed_slot(QObject*)));
		disconnect(d->card, SIGNAL(blockCountChanged(int,int)),
			   this, SLOT(memCard_blockCountChanged_slot()));
	}

	d->card = card;

	// Connect the MemCard's destroyed() signal.
	if (d->card) {
		connect(d->card, SIGNAL(destroyed(QObject*)),
			this, SLOT(memCard_destroyed_slot(QObject*)));
		connect(d->card, SIGNAL(blockCountChanged(int,int)),
			   this, SLOT(memCard_blockCountChanged_slot()));
	}

	// Update the widget display.
	d->updateWidgetDisplay();
}

/**
 * Widget state has changed.
 * @param event State change event.
 */
void MemCardView::changeEvent(QEvent *event)
{
	if (event->type() == QEvent::LanguageChange) {
		// Retranslate the UI.
		Q_D(MemCardView);
		d->ui.retranslateUi(this);
		d->updateWidgetDisplay();
	}

	// Pass the event to the base class.
	this->QWidget::changeEvent(event);
}

/** Slots. **/

/**
 * MemCard object was destroyed.
 * @param obj QObject that was destroyed.
 */
void MemCardView::memCard_destroyed_slot(QObject *obj)
{
	Q_D(MemCardView);

	if (obj == d->card) {
		// Our MemCard was destroyed.
		d->card = nullptr;

		// Update the widget display.
		d->updateWidgetDisplay();
	}
}

/**
 * MemCard's block count has changed.
 */
void MemCardView::memCard_blockCountChanged_slot(void)
{
	Q_D(MemCardView);
	d->updateBlockCountDisplay();
}
