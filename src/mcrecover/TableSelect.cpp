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
#include "McRecoverQApplication.hpp"

// Qt includes.
#include <QtCore/QSignalMapper>

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

		// Signal mappers.
		QSignalMapper *mapperDirTable;
		QSignalMapper *mapperBlockTable;
};

TableSelectPrivate::TableSelectPrivate(TableSelect *q)
	: q_ptr(q)
	, card(nullptr)
	, mapperDirTable(new QSignalMapper(q))
	, mapperBlockTable(new QSignalMapper(q))
{
	// Connect the QSignalMapper slots.
	QObject::connect(mapperDirTable, SIGNAL(mapped(int)),
			 q, SLOT(setActiveDatIdx_slot(int)));
	QObject::connect(mapperBlockTable, SIGNAL(mapped(int)),
			 q, SLOT(setActiveBatIdx_slot(int)));
}

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

	// Update the widget state.
	// TODO: Consolidate this code.

	// Active table CSS.
	// Used to indicate which table is active according to the card header.
	// TODO: Use a better indicator.
	static const QString cssActiveHdr =
		QLatin1String("QFrame { border: 2px solid rgb(0,255,0); }");
	static const QString cssInactiveHdr =
		QLatin1String("QFrame { margin: 2px; }");

	// Icon size.
	static const QSize iconSz(16, 16);

	// Check which Directory Table is currently active.
	switch (card->activeDatIdx()) {
		case 0:
			ui.btnDirA->setChecked(true);
			break;
		case 1:
			ui.btnDirB->setChecked(true);
			break;
		default:
			// No active directory table?
			ui.btnDirA->setChecked(false);
			ui.btnDirB->setChecked(false);
			break;
	}

	// Check which Directory Table is active according to the card header.
	switch (card->activeDatHdrIdx()) {
		case 0:
			ui.lblDirAStatus->setStyleSheet(cssActiveHdr);
			ui.lblDirBStatus->setStyleSheet(cssInactiveHdr);
			break;
		case 1:
			ui.lblDirAStatus->setStyleSheet(cssInactiveHdr);
			ui.lblDirBStatus->setStyleSheet(cssActiveHdr);
			break;
		default:
			// No active directory table?
			ui.lblDirAStatus->setStyleSheet(cssInactiveHdr);
			ui.lblDirBStatus->setStyleSheet(cssInactiveHdr);
			break;
	}

	// Check which Directory Tables are valid.
	QStyle::StandardPixmap spDirA = (card->isDatValid(0)
					? QStyle::SP_DialogApplyButton
					: QStyle::SP_MessageBoxCritical);
	QStyle::StandardPixmap spDirB = (card->isDatValid(1)
					? QStyle::SP_DialogApplyButton
					: QStyle::SP_MessageBoxCritical);
	QIcon iconDirA = McRecoverQApplication::StandardIcon(spDirA, nullptr, ui.lblDirAStatus);
	QIcon iconDirB = McRecoverQApplication::StandardIcon(spDirB, nullptr, ui.lblDirBStatus);
	ui.lblDirAStatus->setPixmap(iconDirA.pixmap(iconSz));
	ui.lblDirBStatus->setPixmap(iconDirB.pixmap(iconSz));

	// Block table.
	switch (card->activeBatIdx()) {
		case 0:
			ui.btnBlockA->setChecked(true);
			break;
		case 1:
			ui.btnBlockB->setChecked(true);
			break;
		default:
			// No active block table?
			ui.btnBlockA->setChecked(false);
			ui.btnBlockB->setChecked(false);
			break;
	}

	// Check which Block Table is active according to the card header.
	switch (card->activeBatHdrIdx()) {
		case 0:
			ui.lblBlockAStatus->setStyleSheet(cssActiveHdr);
			ui.lblBlockBStatus->setStyleSheet(cssInactiveHdr);
			break;
		case 1:
			ui.lblBlockAStatus->setStyleSheet(cssInactiveHdr);
			ui.lblBlockBStatus->setStyleSheet(cssActiveHdr);
			break;
		default:
			// No active block table?
			ui.lblBlockAStatus->setStyleSheet(cssInactiveHdr);
			ui.lblBlockBStatus->setStyleSheet(cssInactiveHdr);
			break;
	}

	// Check which Directory Tables are valid.
	QStyle::StandardPixmap spBlockA = (card->isBatValid(0)
					   ? QStyle::SP_DialogApplyButton
					   : QStyle::SP_MessageBoxCritical);
	QStyle::StandardPixmap spBlockB = (card->isBatValid(1)
					   ? QStyle::SP_DialogApplyButton
					   : QStyle::SP_MessageBoxCritical);
	QIcon iconBlockA = McRecoverQApplication::StandardIcon(spBlockA, nullptr, ui.lblBlockAStatus);
	QIcon iconBlockB = McRecoverQApplication::StandardIcon(spBlockB, nullptr, ui.lblBlockBStatus);
	ui.lblBlockAStatus->setPixmap(iconBlockA.pixmap(iconSz));
	ui.lblBlockBStatus->setPixmap(iconBlockB.pixmap(iconSz));

	// TODO: Set tooltips.

	// Show the widgets.
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

	// Set the icons.
	// TODO: Where to determine icon size?
	static const QSize iconSz(16, 16);

	QIcon iconDirTable = McRecoverQApplication::StandardIcon(
		QStyle::SP_DirClosedIcon, nullptr, d->ui.lblDirImage);
	d->ui.lblDirImage->setPixmap(iconDirTable.pixmap(iconSz));

	// TODO: Windows: Get icon from defrag.exe.
	QIcon iconBlockTable;
#ifdef Q_OS_WIN
	iconBlockTable = McRecoverQApplication::Win32Icon(
		McRecoverQApplication::W32ICON_DEFRAG, iconSz);
#endif /* Q_OS_WIN */
	if (iconBlockTable.isNull()) {
		iconBlockTable = McRecoverQApplication::IconFromTheme(
			QLatin1String("partitionmanager"));
	}
	d->ui.lblBlockImage->setPixmap(iconBlockTable.pixmap(iconSz));

	// Connect QAction signals to the QSignalMappers.
	QObject::connect(d->ui.btnDirA, SIGNAL(clicked()),
			 d->mapperDirTable, SLOT(map()));
	QObject::connect(d->ui.btnDirB, SIGNAL(clicked()),
			 d->mapperDirTable, SLOT(map()));
	QObject::connect(d->ui.btnBlockA, SIGNAL(clicked()),
			 d->mapperBlockTable, SLOT(map()));
	QObject::connect(d->ui.btnBlockB, SIGNAL(clicked()),
			 d->mapperBlockTable, SLOT(map()));

	// Set the mappings in the QSignalMappers.
	d->mapperDirTable->setMapping(d->ui.btnDirA, 0);
	d->mapperDirTable->setMapping(d->ui.btnDirB, 1);
	d->mapperBlockTable->setMapping(d->ui.btnBlockA, 0);
	d->mapperBlockTable->setMapping(d->ui.btnBlockB, 1);
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

/** Events. **/

/**
 * Widget state has changed.
 * @param event State change event.
 */
void TableSelect::changeEvent(QEvent *event)
{
	if (event->type() == QEvent::LanguageChange) {
		// Retranslate the UI.
		Q_D(TableSelect);
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
void TableSelect::memCard_destroyed_slot(QObject *obj)
{
	Q_D(TableSelect);

	if (obj == d->card) {
		// Our MemCard was destroyed.
		d->card = nullptr;

		// Update the widget display.
		d->updateWidgetDisplay();
	}
}

/**
 * Set the active Directory Table index.
 * NOTE: This function reloads the file list, without lost files.
 * @param idx Active Directory Table index. (0 or 1)
 */
void TableSelect::setActiveDatIdx_slot(int idx)
{
	if (idx < 0 || idx >= 2)
		return;
	Q_D(TableSelect);
	d->card->setActiveDatIdx(idx);
}

/**
 * Set the active Block Table index.
 * NOTE: This function reloads the file list, without lost files.
 * @param idx Active Block Table index. (0 or 1)
 */
void TableSelect::setActiveBatIdx_slot(int idx)
{
	if (idx < 0 || idx >= 2)
		return;
	Q_D(TableSelect);
	d->card->setActiveBatIdx(idx);
}
