/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * CardView.hpp: Card view widget.                                         *
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

#include "CardView.hpp"
#include "McRecoverQApplication.hpp"

#include "card/Card.hpp"
#include "card/GcnCard.hpp"
#include "GcnDateTime.hpp"
#include "Checksum.hpp"

// C includes.
#include <stdlib.h>

// C++ includes.
#include <string>
#include <vector>
using std::string;
using std::vector;

// Qt includes.
#include <QtGui/QPainter>

/** CardViewPrivate **/

#include "ui_CardView.h"
class CardViewPrivate
{
	public:
		CardViewPrivate(CardView *q);
		~CardViewPrivate();

	protected:
		CardView *const q_ptr;
		Q_DECLARE_PUBLIC(CardView)
	private:
		Q_DISABLE_COPY(CardViewPrivate)

	public:
		Ui::CardView ui;
		QMargins origMargins;

		Card *card;

		// Format time.
		GcnDateTime formatTime;
		// Card color (border).
		QColor color;
		int cardBorder;

		/**
		 * Update the widget display.
		 */
		void updateWidgetDisplay(void);

		/**
		 * Update the block count display.
		 */
		void updateBlockCountDisplay(void);
};

CardViewPrivate::CardViewPrivate(CardView *q)
	: q_ptr(q)
	, card(nullptr)
	, cardBorder(0)
{ }

CardViewPrivate::~CardViewPrivate()
{ }

/**
 * Update the block count display.
 */
void CardViewPrivate::updateBlockCountDisplay(void)
{
	const int totalUserBlocks = card->totalUserBlocks();
	ui.lblBlockCount->setText(
		CardView::tr("%L1 block(s) (%L2 free)", "", totalUserBlocks)
			.arg(totalUserBlocks)
			.arg(card->freeBlocks()));
}

/**
 * Update the widget display.
 */
void CardViewPrivate::updateWidgetDisplay(void)
{
	Q_Q(CardView);
	if (!card) {
		// Hide the widget display.
		// TODO: Better method?
		//ui.fraBorder->setStyleSheet(QString());
		ui.formLayout->setContentsMargins(0, 0, 0, 0);
		ui.lblBlockCount->setVisible(false);
		ui.lblStatusIcon->setVisible(false);
		ui.lblFormatTimeTitle->setVisible(false);
		ui.lblFormatTime->setVisible(false);
		ui.lblEncodingTitle->setVisible(false);
		ui.lblEncoding->setVisible(false);
		ui.lblChecksumActualTitle->setVisible(false);
		ui.lblChecksumActual->setVisible(false);
		ui.lblChecksumExpectedTitle->setVisible(false);
		ui.lblChecksumExpected->setVisible(false);
		ui.tableSelect->setCard(nullptr);
		ui.tableSelect->setVisible(false);

		// Update the widget.
		// This is needed in order to clear the painted border
		// if a card with a border color was just closed.
		if (color.isValid()) {
			// Clear the color and update the widget.
			color = QColor();
			q->update();
		}
		return;
	}

	// Show the widget display.
	ui.lblBlockCount->setVisible(true);

	// Update the format time.
	if (this->formatTime != card->formatTime()) {
		// Card's format time has changed.
		this->formatTime = card->formatTime();

		if (formatTime.unixTimestamp() == 0) {
			// Invalid format time.
			ui.lblFormatTimeTitle->setVisible(false);
			ui.lblFormatTime->setVisible(false);
		} else {
			// Format time is valid.
			ui.lblFormatTimeTitle->setVisible(true);
			ui.lblFormatTime->setVisible(true);
			ui.lblFormatTime->setText(formatTime.toString(Qt::DefaultLocaleShortDate));
		}
	}

	// TODO: Hide encoding if the card doesn't have it.
	ui.lblEncodingTitle->setVisible(true);
	ui.lblEncoding->setVisible(true);

	// Check if the card's color has changed.
	if (this->color != card->color()) {
		// Card's color has changed.
		this->color = card->color();
		if (color.isValid()) {
			// Valid color.
			ui.formLayout->setContentsMargins(16, 16, 16, 16);
			this->cardBorder = 12;
		} else {
			// Invalid color.
			ui.formLayout->setContentsMargins(origMargins);
			this->cardBorder = 0;
		}

		// Update the widget.
		// This is needed in order to redraw the border color.
		q->update();
	}

	// Update the widget display.
	bool isCardHeaderValid = true;

	// FIXME: Move checksums down to Card.
	// For now, only show checksums if this is a GcnCard.
	GcnCard *gcnCard = qobject_cast<GcnCard*>(card);
	if (gcnCard) {
		ui.lblChecksumActualTitle->setVisible(true);
		ui.lblChecksumActual->setVisible(true);

		// Format the header checksum.
		vector<Checksum::ChecksumValue> checksumValues;
		checksumValues.push_back(gcnCard->headerChecksumValue());
		vector<string> checksumValuesFormatted = Checksum::ChecksumValuesFormatted(checksumValues);
		if (checksumValuesFormatted.size() < 1) {
			// No checksum...
			ui.lblChecksumActual->setText(CardView::tr("Unknown", "checksum"));
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
	} else {
		// Not a GCN card. No checksums (yet).
		ui.lblChecksumActualTitle->setVisible(false);
		ui.lblChecksumActual->setVisible(false);
		ui.lblChecksumExpectedTitle->setVisible(false);
		ui.lblChecksumExpected->setVisible(false);
	}

	// Validate some other aspects of the card header.
	if (isCardHeaderValid) {
		if (card->freeBlocks() > card->totalUserBlocks()) {
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
		ui.lblStatusIcon->setToolTip(CardView::tr("Memory card header is corrupted."));
		ui.lblStatusIcon->setVisible(true);
	}

	// Encoding.
	QString encoding;
	switch (card->encoding()) {
		case Card::ENCODING_UNKNOWN:
		default:
			// Unknown encoding.
			// TODO: If the card doesn't support an encoding value,
			// hide the label entirely.
			encoding = QLatin1String("Unknown");
			break;

		case Card::ENCODING_CP1252:
			encoding = QLatin1String("cp1252");
			break;

		case Card::ENCODING_SHIFTJIS:
			encoding = QLatin1String("Shift-JIS");
			break;
	}
	ui.lblEncoding->setText(encoding);

	// FIXME: Move dir/block table selection down to Card,
	// and add a way for each Card subclass to indicate
	// how many of each table it has.
	if (gcnCard) {
		// Show the dir/block table select widget.
		ui.tableSelect->setCard(gcnCard);
		ui.tableSelect->setVisible(true);
	} else {
		// Not a GCN card.
		ui.tableSelect->setVisible(false);
		ui.tableSelect->setCard(nullptr);
	}
}

/** MemCardFileView **/

CardView::CardView(QWidget *parent)
	: QWidget(parent)
	, d_ptr(new CardViewPrivate(this))
{
	Q_D(CardView);
	d->ui.setupUi(this);

	// Get the original content margins.
	// These are used when there's no card color.
	d->origMargins = d->ui.formLayout->contentsMargins();

	// Set monospace fonts.
	QFont fntMonospace;
	fntMonospace.setFamily(QLatin1String("Monospace"));
	fntMonospace.setStyleHint(QFont::TypeWriter);
	fntMonospace.setBold(true);
	d->ui.lblChecksumActual->setFont(fntMonospace);
	d->ui.lblChecksumExpected->setFont(fntMonospace);

	d->updateWidgetDisplay();
}

CardView::~CardView()
{
	Q_D(CardView);
	delete d;
}

/**
 * Get the Card being displayed.
 * @return Card.
 */
Card *CardView::card(void) const
{
	Q_D(const CardView);
	return d->card;
}

/**
 * Set the Card being displayed.
 * @param card Card.
 */
void CardView::setCard(Card *card)
{
	Q_D(CardView);

	// Disconnect the Card's destroyed() signal if a Card is already set.
	if (d->card) {
		disconnect(d->card, SIGNAL(destroyed(QObject*)),
			   this, SLOT(card_destroyed_slot(QObject*)));
		disconnect(d->card, SIGNAL(blockCountChanged(int,int,int)),
			   this, SLOT(card_blockCountChanged_slot()));
		disconnect(d->card, SIGNAL(colorChanged(QColor)),
			   this, SLOT(card_colorChanged_slot(QColor)));
	}

	d->card = card;

	// Connect the Card's destroyed() signal.
	if (d->card) {
		connect(d->card, SIGNAL(destroyed(QObject*)),
			this, SLOT(card_destroyed_slot(QObject*)));
		connect(d->card, SIGNAL(blockCountChanged(int,int,int)),
			   this, SLOT(card_blockCountChanged_slot()));
		connect(d->card, SIGNAL(colorChanged(QColor)),
			   this, SLOT(card_colorChanged_slot(QColor)));
	}

	// Update the widget display.
	d->updateWidgetDisplay();
}

/**
 * Widget state has changed.
 * @param event State change event.
 */
void CardView::changeEvent(QEvent *event)
{
	if (event->type() == QEvent::LanguageChange) {
		// Retranslate the UI.
		Q_D(CardView);
		d->ui.retranslateUi(this);
		d->updateWidgetDisplay();
	}

	// Pass the event to the base class.
	this->QWidget::changeEvent(event);
}

/**
 * Paint event.
 * @param event Paint event.
 */
void CardView::paintEvent(QPaintEvent *event)
{
	// NOTE: QWidget::paintEvent() does nothing by default.
	// Hence, we don't have to call it.
	Q_UNUSED(event)

	Q_D(const CardView);
	if (!d->card || !d->color.isValid()) {
		// Invalid color.
		// Don't draw a border.
		return;
	}

	QPainter painter(this);
	// NOTE: Not enabling antialiasing for now.
	// (It makes the borders look weird.)
	//painter.setRenderHint(QPainter::Antialiasing);

	// Draw the filled rectangle portion.
	painter.setPen(Qt::NoPen);
	painter.setBrush(d->color);
	const int w = this->width(), h = d->ui.formLayout->sizeHint().height();
	const int border = d->cardBorder;
	painter.drawRect(0, 0, w, border);
	painter.drawRect(w-border, 0, border, h);
	painter.drawRect(0, h-border, w, border);
	painter.drawRect(0, 0, border, h-border);

	// Draw the inner and outer rectangles.
	// (Only if color isn't black, because drawing
	// black on black is a waste of CPU time.)
	if (d->color != Qt::black) {
		painter.setPen(Qt::black);
		painter.setBrush(QBrush());
		painter.drawRect(0, 0, w-1, h-1);
		painter.drawRect(0+border-1, 0+border-1, w-(border*2)+1, h-(border*2)+1);
	}
}

/** Slots. **/

/**
 * Card object was destroyed.
 * @param obj QObject that was destroyed.
 */
void CardView::card_destroyed_slot(QObject *obj)
{
	Q_D(CardView);

	if (obj == d->card) {
		// Our Card was destroyed.
		d->card = nullptr;

		// Update the widget display.
		d->updateWidgetDisplay();
	}
}

/**
 * Card's block count has changed.
 */
void CardView::card_blockCountChanged_slot(void)
{
	Q_D(CardView);
	d->updateBlockCountDisplay();
}

/**
 * Card's color has changed.
 * @param color New color.
 */
void CardView::card_colorChanged_slot(const QColor &color)
{
	Q_D(CardView);
	if (d->color != color) {
		d->color = color;
		this->update();
	}
}
