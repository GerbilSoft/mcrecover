/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * TableSelect.hpp: Directory/Block Table select widget.                   *
 *                                                                         *
 * Copyright (c) 2014-2016 by David Korth.                                 *
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

#include <QWidget>

class Card;

class TableSelectPrivate;
class TableSelect : public QWidget
{
	Q_OBJECT
	typedef QWidget super;

	Q_PROPERTY(Card* card READ card WRITE setCard)
	Q_PROPERTY(int activeDatIdx READ activeDatIdx WRITE setActiveDatIdx)
	Q_PROPERTY(int activeBatIdx READ activeBatIdx WRITE setActiveBatIdx)

	public:
		explicit TableSelect(QWidget *parent = 0);
		virtual ~TableSelect();

	protected:
		TableSelectPrivate *const d_ptr;
		Q_DECLARE_PRIVATE(TableSelect)
	private:
		Q_DISABLE_COPY(TableSelect)

	public:
		/**
		 * Get the Card being displayed.
		 * @return Card.
		 */
		Card *card(void) const;

		/**
		 * Set the Card being displayed.
		 * @param file Card.
		 */
		void setCard(Card *card);

	protected:
		/** Events. **/

		// State change event. (Used for switching the UI language at runtime.)
		void changeEvent(QEvent *event);

	public:
		/** Properties. **/

		/**
		 * Get the selected directory table.
		 * @return Selected directory table index, or -1 on error.
		 */
		int activeDatIdx(void) const;

		/**
		 * Get the selected block table.
		 * @return Selected block table index, or -1 on error.
		 */
		int activeBatIdx(void) const;

	protected slots:
		/** Internal slots. **/

		/**
		 * Card object was destroyed.
		 * @param obj QObject that was destroyed.
		 */
		void memCard_destroyed_slot(QObject *obj = 0);

		/**
		 * Card's active Directory Table index was changed.
		 * @param idx New active Directory Table index.
		 */
		void memCard_activeDatIdxChanged_slot(int idx);

		/**
		 * Card's active Block Table index was changed.
		 * @param idx New active Block Table index.
		 */
		void memCard_activeBatIdxChanged_slot(int idx);

	public slots:
		/** Public slots. **/

		/**
		 * Set the active Directory Table index.
		 * NOTE: This function reloads the file list, without lost files.
		 * @param idx Active Directory Table index. (0 or 1)
		 */
		void setActiveDatIdx(int idx);

		/**
		 * Set the active Block Table index.
		 * NOTE: This function reloads the file list, without lost files.
		 * @param idx Active Block Table index. (0 or 1)
		 */
		void setActiveBatIdx(int idx);
};

#endif /* __MCRECOVER_TABLESELECT_HPP__ */
