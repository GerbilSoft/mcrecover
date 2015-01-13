/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * SlotSelector.hpp: Slot selection widget.                                *
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

#ifndef __MCRECOVER_EDIT_SONICADVENTURE_SLOTSELECTOR_HPP__
#define __MCRECOVER_EDIT_SONICADVENTURE_SLOTSELECTOR_HPP__

#include <QtGui/QWidget>

class SlotSelectorPrivate;
class SlotSelector : public QWidget
{
	Q_OBJECT

	Q_PROPERTY(int slotCount READ slotCount WRITE setSlotCount NOTIFY slotCountChanged)
	Q_PROPERTY(int slot READ slot WRITE setSlot NOTIFY slotChanged)

	public:
		SlotSelector(QWidget *parent = 0);
		~SlotSelector();

	protected:
		SlotSelectorPrivate *const d_ptr;
		Q_DECLARE_PRIVATE(SlotSelector)
	private:
		Q_DISABLE_COPY(SlotSelector)

	signals:
		/**
		 * Slot count has changed.
		 * @param slotCount New slot count.
		 */
		void slotCountChanged(int slotCount);

		/**
		 * Selected slot has changed.
		 * @param slot New slot.
		 */
		void slotChanged(int slot);

	public:
		/** Public functions. **/

		/**
		 * Get the number of slots.
		 * @return Number of slots.
		 */
		int slotCount(void) const;

		/**
		 * Set the number of slots.
		 * @param slotCount Number of slots.
		 */
		void setSlotCount(int slotCount);

		/**
		 * Get the current slot.
		 * @return Current slot.
		 */
		int slot(void) const;

		/**
		 * Set the current slot.
		 * @param slot Current slot.
		 */
		void setSlot(int slot);

	protected slots:
		/** UI widget slots. **/

		/**
		 * SignalMapper mapped() signal for slot buttons.
		 * @param slot New slot.
		 */
		void on_signalMapper_mapped(int slot);
};

#endif /* __MCRECOVER_EDIT_SONICADVENTURE_SLOTSELECTOR_HPP__ */
