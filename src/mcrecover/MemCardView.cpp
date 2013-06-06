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

#include "MemCard.hpp"


/** MemCardViewPrivate **/

class MemCardViewPrivate
{
	public:
		MemCardViewPrivate(MemCardView *q);
		~MemCardViewPrivate();

	private:
		MemCardView *const q;
		Q_DISABLE_COPY(MemCardViewPrivate);

	public:
		const MemCard *card;

		/**
		 * Update the widget display.
		 */
		void updateWidgetDisplay(void);
};

MemCardViewPrivate::MemCardViewPrivate(MemCardView *q)
	: q(q)
	, card(NULL)
{ }

MemCardViewPrivate::~MemCardViewPrivate()
{ }


/**
 * Update the widget display.
 */
void MemCardViewPrivate::updateWidgetDisplay(void)
{
	if (!card) {
		// Hide the widget display.
		// TODO: Better method?
		q->lblBlockCount->setVisible(false);
		q->lblSerialNumberTitle->setVisible(false);
		q->lblSerialNumber->setVisible(false);
		q->lblEncodingTitle->setVisible(false);
		q->lblEncoding->setVisible(false);
		return;
	}

	// Show the widget display.
	q->lblBlockCount->setVisible(true);
	q->lblSerialNumberTitle->setVisible(true);
	q->lblSerialNumber->setVisible(true);
	q->lblEncodingTitle->setVisible(true);
	q->lblEncoding->setVisible(true);

	// Update the widget display.
	q->lblBlockCount->setText(
		q->tr("%L1 block(s) (%L2 free)")
		.arg(card->sizeInBlocks() - 5)
		.arg(card->freeBlocks()));

	// Serial number.
	QString serial_text = card->serialNumber();

	// Split into lines of 8 characters each.
	// TODO: Optimize using QString substrings?
	QString serial_text_split;
	serial_text_split.reserve(serial_text.size() * 4 / 3);
	for (int i = 0; i < serial_text.size(); i++) {
		if (i > 0 && !(i % 8))
			serial_text_split.append(QChar(L'\n'));
		serial_text_split.append(serial_text.at(i));
	}
		 
	q->lblSerialNumber->setText(serial_text_split);

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
	q->lblEncoding->setText(encoding);
}


/** McRecoverWindow **/

MemCardView::MemCardView(QWidget *parent)
	: QWidget(parent)
	, d(new MemCardViewPrivate(this))
{
	setupUi(this);
}

MemCardView::~MemCardView()
{
	delete d;
}


/**
 * Get the MemCard being displayed.
 * @return MemCard.
 */
const MemCard *MemCardView::card(void) const
	{ return d->card; }

/**
 * Set the MemCard being displayed.
 * @param card MemCard.
 */
void MemCardView::setCard(const MemCard *card)
{
	// Disconnect the MemCard's destroyed() signal if a MemCard is already set.
	if (d->card) {
		disconnect(d->card, SIGNAL(destroyed(QObject*)),
			   this, SLOT(memCard_destroyed_slot(QObject*)));
	}

	d->card = card;

	// Connect the MemCard's destroyed() signal.
	if (d->card) {
		connect(d->card, SIGNAL(destroyed(QObject*)),
			this, SLOT(memCard_destroyed_slot(QObject*)));
	}

	// Update the widget display.
	d->updateWidgetDisplay();
}


/** Slots. **/


/**
 * MemCard object was destroyed.
 * @param obj QObject that was destroyed.
 */
void MemCardView::memCard_destroyed_slot(QObject *obj)
{
	if (obj == d->card) {
		// Our MemCard was destroyed.
		d->card = NULL;

		// Update the widget display.
		d->updateWidgetDisplay();
	}
}
