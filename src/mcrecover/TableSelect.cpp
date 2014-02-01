/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * TableSelect.cpp: Directory/Block Table select widget.                   *
 *                                                                         *
 * Copyright (c) 2014 by David Korth.                                      *
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

#include "TableSelect.hpp"

#include "MemCard.hpp"

/** TableSelectPrivate **/

#include "ui_TableSelect.h"
class TableSelectPrivate
{
	public:
		TableSelectPrivate(TableSelect *q);
		~TableSelectPrivate();

	protected:
		TableSelect *const q_ptr;
		Q_DECLARE_PUBLIC(TableSelect)
	private:
		Q_DISABLE_COPY(TableSelectPrivate)

	public:
		// UI
		Ui::TableSelect ui;

		MemCard *card;

		/**
		 * Update the widget display.
		 */
		void updateWidgetDisplay(void);
};

TableSelectPrivate::TableSelectPrivate(TableSelect *q)
	: q_ptr(q)
	, card(nullptr)
{ }

TableSelectPrivate::~TableSelectPrivate()
{ }

/**
 * Update the widget display.
 */
void TableSelectPrivate::updateWidgetDisplay(void)
{
	if (!card) {
		// Hide the widget display.
		// TODO: Better method?
		ui.fraDirTable->setVisible(false);
		ui.fraBlockTable->setVisible(false);
		return;
	}

	// TODO: Get information from the MemCard.
	ui.fraDirTable->setVisible(true);
	ui.fraBlockTable->setVisible(true);
}

/** TableSelect **/

TableSelect::TableSelect(QWidget *parent)
	: QWidget(parent)
	, d_ptr(new TableSelectPrivate(this))
{
	Q_D(TableSelect);
	d->ui.setupUi(this);
}

TableSelect::~TableSelect()
{
	Q_D(TableSelect);
	delete d;
}

/**
 * Get the MemCard being displayed.
 * @return MemCard.
 */
MemCard *TableSelect::card(void) const
{
	Q_D(const TableSelect);
	return d->card;
}

/**
 * Set the MemCard being displayed.
 * @param card MemCard.
 */
void TableSelect::setCard(MemCard *card)
{
	Q_D(TableSelect);

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
