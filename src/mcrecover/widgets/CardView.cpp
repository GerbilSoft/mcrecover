/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * CardView.hpp: Card view widget.                                         *
 *                                                                         *
 * Copyright (c) 2012-2025 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#include "CardView.hpp"
#include "McRecoverQApplication.hpp"

#include "libmemcard/Card.hpp"
#include "libmemcard/GcnCard.hpp"
#include "Checksum.hpp"

// C includes
#include <stdlib.h>

// C++ includes
#include <string>
#include <vector>
using std::string;
using std::vector;

// Qt includes
#include <QtGui/QPainter>

/** CardViewPrivate **/

#include "ui_CardView.h"
class CardViewPrivate
{
public:
	explicit CardViewPrivate(CardView *q);
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

	// Format time
	QDateTime formatTime;
	// Card color (border)
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

	// Free Block count status.
	// TODO: Check if the card supports this.
	if (card->isFreeBlockCountValid(card->activeBatIdx())) {
		// Free Block count is valid.
		ui.lblCardHeaderStatus->setVisible(false);
	} else {
		// Free Block count is invalid.
		// TODO: Get the actual free block count?
		QIcon icon = McRecoverQApplication::IconFromTheme(QLatin1String("dialog-error"));
		// TODO: What size?
		ui.lblFreeBlockStatus->setPixmap(icon.pixmap(16, 16));
		ui.lblFreeBlockStatus->setToolTip(CardView::tr("Free block count is incorrect."));
		ui.lblFreeBlockStatus->setVisible(true);
	}
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
		ui.lblCardIcon->setVisible(false);
		ui.lblCardType->setVisible(false);
		ui.lblCardHeaderStatus->setVisible(false);
		ui.lblBlockCount->setVisible(false);
		ui.lblFreeBlockStatus->setVisible(false);
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

	// Card icon and type
	QPixmap icon = card->icon();
	if (!icon.isNull()) {
		ui.lblCardIcon->setPixmap(icon);
		ui.lblCardIcon->setVisible(true);
	} else {
		ui.lblCardIcon->setVisible(false);
	}
	// TODO: Shortened product name, with line breaks.
	//ui.lblCardType->setText(card->productName());
	ui.lblCardType->setVisible(true);

	// Block count
	ui.lblBlockCount->setVisible(true);

	// Update the format time
	if (this->formatTime != card->formatTime()) {
		// Card's format time has changed.
		this->formatTime = card->formatTime();

		if (!formatTime.isValid() || formatTime.toMSecsSinceEpoch() == 0) {
			// Invalid format time
			ui.lblFormatTimeTitle->setVisible(false);
			ui.lblFormatTime->setVisible(false);
		} else {
			// Format time is valid
			const QLocale locale = QLocale::system();
			ui.lblFormatTimeTitle->setVisible(true);
			ui.lblFormatTime->setVisible(true);
			ui.lblFormatTime->setText(locale.toString(formatTime, locale.dateTimeFormat(QLocale::ShortFormat)));
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

	// Block count.
	updateBlockCountDisplay();

	// Card header status.
	if (isCardHeaderValid) {
		// Card header is valid.
		ui.lblCardHeaderStatus->setVisible(false);
	} else {
		// Card header is invalid.
		QIcon icon = McRecoverQApplication::IconFromTheme(QLatin1String("dialog-error"));
		// TODO: What size?
		ui.lblCardHeaderStatus->setPixmap(icon.pixmap(16, 16));
		ui.lblCardHeaderStatus->setToolTip(CardView::tr("Memory card header is corrupted."));
		ui.lblCardHeaderStatus->setVisible(true);
	}

	// Encoding.
	QString encoding;
	switch (card->encoding()) {
		case Card::Encoding::Unknown:
		default:
			// Unknown encoding.
			// TODO: If the card doesn't support an encoding value,
			// hide the label entirely.
			encoding = CardView::tr("Unknown");
			break;

		case Card::Encoding::CP1252:
			encoding = QLatin1String("cp1252");
			break;

		case Card::Encoding::Shift_JIS:
			encoding = QLatin1String("Shift-JIS");
			break;
	}
	ui.lblEncoding->setText(encoding);

	// Show the dir/block table select widget.
	ui.tableSelect->setCard(card);
	ui.tableSelect->setVisible(true);
}

/** MemCardFileView **/

CardView::CardView(QWidget *parent)
	: super(parent)
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
 * @return Card
 */
Card *CardView::card(void) const
{
	Q_D(const CardView);
	return d->card;
}

/**
 * Set the Card being displayed.
 * @param card Card
 */
void CardView::setCard(Card *card)
{
	Q_D(CardView);

	// Disconnect the Card's destroyed() signal if a Card is already set.
	if (d->card) {
		disconnect(d->card, &QObject::destroyed,
			   this, &CardView::card_destroyed_slot);
		disconnect(d->card, &Card::blockCountChanged,
			   this, &CardView::card_blockCountChanged_slot);
		disconnect(d->card, &Card::colorChanged,
			   this, &CardView::card_colorChanged_slot);
	}

	d->card = card;

	// Connect the Card's destroyed() signal.
	if (d->card) {
		connect(d->card, &QObject::destroyed,
			this, &CardView::card_destroyed_slot);
		connect(d->card, &Card::blockCountChanged,
			this, &CardView::card_blockCountChanged_slot);
		connect(d->card, &Card::colorChanged,
			this, &CardView::card_colorChanged_slot);
	}

	// Update the widget display.
	d->updateWidgetDisplay();
}

/**
 * Widget state has changed.
 * @param event State change event
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
	super::changeEvent(event);
}

/**
 * Paint event.
 * @param event Paint event
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

/** Slots **/

/**
 * Card object was destroyed.
 * @param obj QObject that was destroyed
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
 * @param color New color
 */
void CardView::card_colorChanged_slot(const QColor &color)
{
	Q_D(CardView);
	if (d->color != color) {
		d->color = color;
		this->update();
	}
}
