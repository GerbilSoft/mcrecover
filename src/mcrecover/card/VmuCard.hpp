/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * VmuCard.hpp: Dreamcast VMU memory card class.                           *
 *                                                                         *
 * Copyright (c) 2015-2016 by David Korth.                                 *
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

#ifndef __MCRECOVER_CARD_VMUCARD_HPP__
#define __MCRECOVER_CARD_VMUCARD_HPP__

#include "Card.hpp"

class VmuCardPrivate;
class VmuCard : public Card
{
	Q_OBJECT
	typedef Card super;

	protected:
		VmuCard(QObject *parent = 0);
	public:
		virtual ~VmuCard();

	protected:
		Q_DECLARE_PRIVATE(VmuCard)
	private:
		Q_DISABLE_COPY(VmuCard)

	public:
		/**
		 * Open an existing VMU image.
		 * @param filename VMU image filename.
		 * @param parent Parent object.
		 * @return VmuCard object, or nullptr on error.
		 */
		static VmuCard *open(const QString& filename, QObject *parent);

		/**
		 * Format a new VMU image.
		 * @param filename VMU image filename.
		 * @param parent Parent object.
		 * @return VmuCard object, or nullptr on error.
		 */
		static VmuCard *format(const QString& filename, QObject *parent);

	public:
		/** File system **/

		/**
		 * Set the active Directory Table index.
		 * NOTE: This function reloads the file list, without lost files.
		 * @param idx Active Directory Table index.
		 */
		virtual void setActiveDatIdx(int idx) override;

		/**
		 * Set the active Block Table index.
		 * NOTE: This function reloads the file list, without lost files.
		 * @param idx Active Block Table index.
		 */
		virtual void setActiveBatIdx(int idx) override;

	public:
		/**
		 * Get the product name of this memory card.
		 * This refers to the class in general,
		 * and does not change based on size.
		 * @return Product name.
		 */
		virtual QString productName(void) const override;
};

#endif /* __MCRECOVER_CARD_VMUCARD_HPP__ */
