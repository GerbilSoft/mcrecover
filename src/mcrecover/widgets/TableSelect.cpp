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

		// Icon size.
		static const int iconSz = 16;

	protected:
		/**
		 * Update the display for a set of items.
		 * @param btnA Button for table A.
		 * @param btnB Button for table B.
		 * @param lblA Label for table A status.
		 * @param lblB Label for table B status.
		 * @param activeIdx Active index, selected by the user.
		 * @param activeHdrIdx Active index, according to the card header.
		 * @param isValidA Is table A valid?
		 * @param isValidB Is table B valid?
		 * @param description Table description for the tooltips.
		 */
		void updateSetDisplay(
				QAbstractButton *btnA, QAbstractButton *btnB,
				QLabel *lblStatusA, QLabel *lblStatusB,
				int activeIdx, int activeHdrIdx,
				bool isValidA, bool isValidB,
				const QString &description);

	public:
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
 * Update the display for a set of items.
 * @param btnA Button for table A.
 * @param btnB Button for table B.
 * @param lblA Label for table A status.
 * @param lblB Label for table B status.
 * @param activeIdx Active index, selected by the user.
 * @param activeHdrIdx Active index, according to the card header.
 * @param isValidA Is table A valid?
 * @param isValidB Is table B valid?
 * @param description Table description for the tooltips.
 */
void TableSelectPrivate::updateSetDisplay(
		QAbstractButton *btnA, QAbstractButton *btnB,
		QLabel *lblA, QLabel *lblB,
		int activeIdx, int activeHdrIdx,
		bool isValidA, bool isValidB,
		const QString &description)
{
	// Active table CSS.
	// Used to indicate which table is active according to the card header.
	// TODO: Use a better indicator.
	static const QString cssActiveHdr =
		QLatin1String("QFrame { border: 2px solid rgb(0,255,0); }");
	static const QString cssInactiveHdr =
		QLatin1String("QFrame { margin: 2px; }");

	// Check which table is currently active, selected by the user.
	switch (activeIdx) {
		case 0:
			btnA->setChecked(true);
			btnB->setChecked(false);
			break;
		case 1:
			btnA->setChecked(false);
			btnB->setChecked(true);
			break;
		default:
			// No active table?
			btnA->setChecked(false);
			btnB->setChecked(false);
			break;
	}

	// Check which table is active, according to the card header.
	switch (activeHdrIdx) {
		case 0:
			lblA->setStyleSheet(cssActiveHdr);
			lblB->setStyleSheet(cssInactiveHdr);
			break;
		case 1:
			lblA->setStyleSheet(cssInactiveHdr);
			lblB->setStyleSheet(cssActiveHdr);
			break;
		default:
			// No active table?
			lblA->setStyleSheet(cssInactiveHdr);
			lblB->setStyleSheet(cssInactiveHdr);
			break;
	}

	// Check which tables are valid.
	QStyle::StandardPixmap spA = (isValidA
					? QStyle::SP_DialogApplyButton
					: QStyle::SP_MessageBoxCritical);
	QStyle::StandardPixmap spB = (isValidB
					? QStyle::SP_DialogApplyButton
					: QStyle::SP_MessageBoxCritical);
	QIcon iconA = McRecoverQApplication::StandardIcon(spA, nullptr, lblA);
	QIcon iconB = McRecoverQApplication::StandardIcon(spB, nullptr, lblB);
	lblA->setPixmap(iconA.pixmap(iconSz, iconSz));
	lblB->setPixmap(iconB.pixmap(iconSz, iconSz));

	// Set the status tooltips.
	QString tipA, tipB;
	if (isValidA) {
		if (activeHdrIdx == 0) {
			//: %1 == "Block Table" or "Directory Table"; %2 == 'A' or 'B'.
			tipA = TableSelect::tr("%1 %2 is valid, and is the active table on the card.");
		} else {
			//: %1 == "Block Table" or "Directory Table"; %2 == 'A' or 'B'.
			tipA = TableSelect::tr("%1 %2 is valid.");
		}
	} else {
		//: %1 == "Block Table" or "Directory Table"; %2 == 'A' or 'B'.
		tipA = TableSelect::tr("%1 %2 is invalid.");
	}
	if (isValidB) {
		if (activeHdrIdx == 1) {
			//: %1 == "Block Table" or "Directory Table"; %2 == 'A' or 'B'.
			tipB = TableSelect::tr("%1 %2 is valid, and is the active table on the card.");
		} else {
			//: %1 == "Block Table" or "Directory Table"; %2 == 'A' or 'B'.
			tipB = TableSelect::tr("%1 %2 is valid.");
		}
	} else {
		//: %1 == "Block Table" or "Directory Table"; %2 == 'A' or 'B'.
		tipB = TableSelect::tr("%1 %2 is invalid.");
	}

	lblA->setToolTip(tipA.arg(description).arg('A'));
	lblB->setToolTip(tipB.arg(description).arg('B'));
}

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

	// Directory Table.
	updateSetDisplay(ui.btnDirA, ui.btnDirB,
			ui.lblDirAStatus, ui.lblDirBStatus,
			card->activeDatIdx(), card->activeDatHdrIdx(),
			card->isDatValid(0), card->isDatValid(1),
			TableSelect::tr("Directory Table"));

	// Block Table.
	updateSetDisplay(ui.btnBlockA, ui.btnBlockB,
			ui.lblBlockAStatus, ui.lblBlockBStatus,
			card->activeBatIdx(), card->activeBatHdrIdx(),
			card->isBatValid(0), card->isBatValid(1),
			TableSelect::tr("Block Table"));

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
	QSize qIconSz(TableSelectPrivate::iconSz, TableSelectPrivate::iconSz);

	QIcon iconDirTable = McRecoverQApplication::StandardIcon(
		QStyle::SP_DirClosedIcon, nullptr, d->ui.lblDirImage);
	d->ui.lblDirImage->setPixmap(iconDirTable.pixmap(qIconSz));

	QIcon iconBlockTable;
#ifdef Q_OS_WIN
	// Win32: Get the icon from Windows Defragmenter.
	iconBlockTable = McRecoverQApplication::Win32Icon(
		McRecoverQApplication::W32ICON_DEFRAG, qIconSz);
#endif /* Q_OS_WIN */
	if (iconBlockTable.isNull()) {
		iconBlockTable = McRecoverQApplication::IconFromTheme(
			QLatin1String("partitionmanager"));
	}
	d->ui.lblBlockImage->setPixmap(iconBlockTable.pixmap(qIconSz));

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
