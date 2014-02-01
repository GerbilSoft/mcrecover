/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * TableSelect.hpp: Directory/Block Table select widget.                   *
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

#ifndef __MCRECOVER_TABLESELECT_HPP__
#define __MCRECOVER_TABLESELECT_HPP__

#include <QtGui/QWidget>

class MemCard;

class TableSelectPrivate;

class TableSelect : public QWidget
{
	Q_OBJECT

	Q_PROPERTY(MemCard* card READ card WRITE setCard)

	public:
		TableSelect(QWidget *parent = 0);
		~TableSelect();

	protected:
		TableSelectPrivate *const d_ptr;
		Q_DECLARE_PRIVATE(TableSelect)
	private:
		Q_DISABLE_COPY(TableSelect)

	public:
		/**
		 * Get the MemCard being displayed.
		 * @return MemCard.
		 */
		MemCard *card(void) const;

		/**
		 * Set the MemCard being displayed.
		 * @param file MemCard.
		 */
		void setCard(MemCard *card);

		/** Events. **/

	protected:
		// State change event. (Used for switching the UI language at runtime.)
		void changeEvent(QEvent *event);

		/** Slots. **/

	protected slots:
		/**
		 * MemCard object was destroyed.
		 * @param obj QObject that was destroyed.
		 */
		void memCard_destroyed_slot(QObject *obj = 0);
};

#endif /* __MCRECOVER_TABLESELECT_HPP__ */
