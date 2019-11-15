/***************************************************************************
 * GameCube Memory Card Recovery Program [libmemcard]                      *
 * VmuCard.hpp: Dreamcast VMU memory card class.                           *
 *                                                                         *
 * Copyright (c) 2015-2018 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#ifndef __LIBMEMCARD_VMUCARD_HPP__
#define __LIBMEMCARD_VMUCARD_HPP__

#include "Card.hpp"

class VmuCardPrivate;
class VmuCard : public Card
{
	Q_OBJECT
	typedef Card super;

	protected:
		explicit VmuCard(QObject *parent = 0);
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
		void setActiveDatIdx(int idx) final;

		/**
		 * Set the active Block Table index.
		 * NOTE: This function reloads the file list, without lost files.
		 * @param idx Active Block Table index.
		 */
		void setActiveBatIdx(int idx) final;

	public:
		/**
		 * Get the product name of this memory card.
		 * This refers to the class in general,
		 * and does not change based on size.
		 * @return Product name.
		 */
		QString productName(void) const final;
};

#endif /* __LIBMEMCARD_VMUCARD_HPP__ */
