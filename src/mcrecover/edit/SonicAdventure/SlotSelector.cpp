/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * SlotSelector.cpp: Slot selection widget.                                *
 *                                                                         *
 * Copyright (c) 2015 by David Korth.                                      *
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

#include "SlotSelector.hpp"

// Qt widgets.
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QButtonGroup>
#include <QtCore/QSignalMapper>

/** SlotSelectorPrivate **/

class SlotSelectorPrivate
{
	public:
		SlotSelectorPrivate(SlotSelector *q);
		~SlotSelectorPrivate();

	protected:
		SlotSelector *const q_ptr;
		Q_DECLARE_PUBLIC(SlotSelector)
	private:
		Q_DISABLE_COPY(SlotSelectorPrivate)

	public:
		struct Ui_SlotSelector {
			QHBoxLayout *hboxMain;
			QLabel *lblSlotTitle;
			QButtonGroup *buttonGroup;
			QSignalMapper *signalMapper;

			void setupUi(QWidget *SlotSelector);
		};
		Ui_SlotSelector ui;

		// Maximum slot number.
		// TODO: More than 99?
		static const int MAX_SLOTS = 99;

		// Slot buttons.
		// TODO: QToolButton instead?
		QVector<QPushButton*> btnSlots;

		// Selected slot number.
		int slot;
};

SlotSelectorPrivate::SlotSelectorPrivate(SlotSelector *q)
	: q_ptr(q)
	, slot(0)
{ }

SlotSelectorPrivate::~SlotSelectorPrivate()
{
	// TODO: Is this necessary?
	qDeleteAll(btnSlots);
	btnSlots.clear();
}

/**
 * Initialize the UI.
 * @param SlotSelector SlotSelector.
 */
void SlotSelectorPrivate::Ui_SlotSelector::setupUi(QWidget *SlotSelector)
{
	if (SlotSelector->objectName().isEmpty())
		SlotSelector->setObjectName(QLatin1String("SlotSelector"));

	hboxMain = new QHBoxLayout(SlotSelector);
	hboxMain->setContentsMargins(2, 2, 2, 2);
	hboxMain->setObjectName(QLatin1String("hboxMain"));

	lblSlotTitle = new QLabel(SlotSelector);
	lblSlotTitle->setObjectName(QLatin1String("lblSlotTitle"));
	// TODO: Buddy widget?
	lblSlotTitle->setText(SlotSelector::tr("Slots:"));
	hboxMain->addWidget(lblSlotTitle);

	buttonGroup = new QButtonGroup(SlotSelector);
	buttonGroup->setObjectName(QLatin1String("buttonGroup"));

	signalMapper = new QSignalMapper(SlotSelector);
	signalMapper->setObjectName(QLatin1String("signalMapper"));

	QMetaObject::connectSlotsByName(SlotSelector);
}

/** SlotSelector **/

SlotSelector::SlotSelector(QWidget *parent)
	: QWidget(parent)
	, d_ptr(new SlotSelectorPrivate(this))
{
	Q_D(SlotSelector);
	d->ui.setupUi(this);

	// Initialize to a single slot.
	setSlotCount(1);
	d->btnSlots[0]->setChecked(true);
}

SlotSelector::~SlotSelector()
{
	Q_D(SlotSelector);
	delete d;
}

/** Public functions. **/

/**
 * Get the number of slots.
 * @return Number of slots.
 */
int SlotSelector::slotCount(void) const
{
	Q_D(const SlotSelector);
	return d->btnSlots.size();
}

/**
 * Set the number of slots.
 * @param slotCount Number of slots.
 */
void SlotSelector::setSlotCount(int slotCount)
{
	// Clamp the slot count.
	Q_D(SlotSelector);
	if (slotCount < 1)
		slotCount = 1;
	else if (slotCount > d->MAX_SLOTS)
		slotCount = d->MAX_SLOTS;

	const int oldSlotCount = d->btnSlots.size();
	if (oldSlotCount == slotCount) {
		// No change.
		return;
	} else if (d->btnSlots.size() < slotCount) {
		// Add more buttons.
		for (int i = d->btnSlots.size(); i < slotCount; i++) {
			QPushButton *btn = new QPushButton(this);
			btn->setCheckable(true);
			d->ui.buttonGroup->addButton(btn);
			d->ui.hboxMain->insertWidget(i + 1, btn);
			d->btnSlots.append(btn);

			// TODO: Allow navigation using the Left and Right keys?

			// Set the label.
			// Slots 1-9 will have accelerators.
			QString txt;
			if (i < 9)
				txt = QLatin1String("&");
			txt += QString::number(i + 1);
			btn->setText(txt);

			// Add a signal mapping.
			d->ui.signalMapper->setMapping(btn, i);
			connect(btn, SIGNAL(clicked(bool)),
				d->ui.signalMapper, SLOT(map()));
		}
	} else /*if (d->btnSlots.size() > slotCount)*/ {
		// Remove buttons.
		for (int i = (d->btnSlots.size() - 1); i >= slotCount; i--) {
			delete d->btnSlots.at(i);
		}
		d->btnSlots.resize(slotCount);

		// If the current slot is too high, lower it.
		if (d->slot >= slotCount)
			setSlot(slotCount - 1);
	}

	// NOTE: Setting individual widget visibility here
	// doesn't work too well, since the parent layout
	// spacing isn't removed. The parent widget will
	// need to hide this widget if slotCount <= 1.

	// Slot count has changed.
	emit slotCountChanged(slotCount);
}

/**
 * Get the current slot.
 * @return Current slot.
 */
int SlotSelector::slot(void) const
{
	Q_D(const SlotSelector);
	return d->slot;
}

/**
 * Set the current slot.
 * @param slot Current slot.
 */
void SlotSelector::setSlot(int slot)
{
	// Clamp the slot number.
	Q_D(SlotSelector);
	if (d->btnSlots.size() <= 0)
		return;
	else if (slot < 0)
		slot = 0;
	else if (slot >= d->btnSlots.size())
		slot = (d->btnSlots.size() - 1);

	// If the slot is already set, don't do anything.
	if (d->slot == slot)
		return;

	// Set the slot.
	d->slot = slot;
	d->btnSlots[slot]->setChecked(true);
	emit slotChanged(slot);
}

/** Widget slots. **/

/**
 * SignalMapper mapped() signal for slot buttons.
 * @param slot New slot.
 */
void SlotSelector::on_signalMapper_mapped(int slot)
{
	// Clamp the slot number.
	Q_D(SlotSelector);
	if (d->btnSlots.size() <= 0)
		return;
	else if (slot < 0)
		slot = 0;
	else if (slot >= d->btnSlots.size())
		slot = (d->btnSlots.size() - 1);

	// If the slot is already set, don't do anything.
	if (d->slot == slot)
		return;

	// Slot has changed.
	d->slot = slot;
	emit slotChanged(slot);
}
