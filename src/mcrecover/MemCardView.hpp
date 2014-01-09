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

#ifndef __MCRECOVER_MEMCARDVIEW_HPP__
#define __MCRECOVER_MEMCARDVIEW_HPP__

#include <QtGui/QWidget>
#include "ui_MemCardView.h"

// MemCard class.
class MemCard;

class MemCardViewPrivate;

class MemCardView : public QWidget, public Ui::MemCardView
{
	Q_OBJECT

	Q_PROPERTY(const MemCard* card READ card WRITE setCard)

	public:
		MemCardView(QWidget *parent = 0);
		~MemCardView();

	protected:
		MemCardViewPrivate *const d_ptr;
		Q_DECLARE_PRIVATE(MemCardView)
	private:
		Q_DISABLE_COPY(MemCardView)

	public:
		/**
		 * Get the MemCard being displayed.
		 * @return MemCard.
		 */
		const MemCard *card(void) const;

		/**
		 * Set the MemCard being displayed.
		 * @param file MemCard.
		 */
		void setCard(const MemCard *card);

	protected:
		// State change event. (Used for switching the UI language at runtime.)
		void changeEvent(QEvent *event);

	protected slots:
		/**
		 * MemCard object was destroyed.
		 * @param obj QObject that was destroyed.
		 */
		void memCard_destroyed_slot(QObject *obj = 0);
};

#endif /* __MCRECOVER_MEMCARDVIEW_HPP__ */
