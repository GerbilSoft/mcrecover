/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * Card_p.hpp: Memory Card physical layer. [base class] (PRIVATE)          *
 *                                                                         *
 * Copyright (c) 2012-2015 by David Korth.                                 *
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

#ifndef __MCRECOVER_CARD_CARD_P_HPP__
#define __MCRECOVER_CARD_CARD_P_HPP__

#include "Card.hpp"

// Qt includes.
#include <QtCore/QFile>
#include <QtCore/QFlags>
#include <QtCore/QString>

class CardPrivate
{
	public:
		CardPrivate(Card *q, uint32_t blockSize, int minBlocks, int maxBlocks);
		virtual ~CardPrivate();

	protected:
		Card *const q_ptr;
		Q_DECLARE_PUBLIC(Card)
	private:
		Q_DISABLE_COPY(CardPrivate)

	public:
		// Card errors.
		QString errorString;
		QFlags<Card::Error> errors;

		// File information.
		QString filename;
		QFile *file;
		quint64 filesize;

		// Card size information.
		const uint32_t blockSize;	// must be a power of 2
		const int minBlocks;	// smallest usable card, in blocks
		const int maxBlocks;	// largest usable card, in blocks

		// Block counts. [cached]
		int totalPhysBlocks;	// set by open()
		int totalUserBlocks;	// must be set by subclass
		int freeBlocks;		// must be set by subclass

		// TODO: Move usedBlockMap here?

		/**
		 * Check if a number is a power of 2.
		 * Reference: http://stackoverflow.com/questions/108318/whats-the-simplest-way-to-test-whether-a-number-is-a-power-of-2-in-c
		 * @param n Number.
		 * @return True if this number is a power of 2.
		 */
		template<typename T>
		static inline bool isPow2(T n) {
			return !(n == 0) && !(n & (n - 1));
		}

		/** Convenience functions for Card subclasses. **/

		/**
		 * Open a Memory Card image.
		 * totalPhysBlocks is initialized after the file is opened.
		 * totalUserBlocks and freeBlocks must be initialized by the subclass.
		 * @param filename Memory Card image filename.
		 * @param mode File open mode.
		 * @return 0 on success; non-zero on error. (also check errorString)
		 */
		int open(const QString &filename, QIODevice::OpenModeFlag mode);

		/**
		 * Close the currently-opened Memory Card image.
		 * This will clear all cached file information.
		 */
		void close(void);
};

#endif /* __MCRECOVER_CARD_CARD_P_HPP__ */
