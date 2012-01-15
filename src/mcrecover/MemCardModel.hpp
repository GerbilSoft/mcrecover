/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * MemCardModel.hpp: QAbstractListModel for MemCard.                       *
 *                                                                         *
 * Copyright (c) 2011 by David Korth.                                      *
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

#ifndef __MCRECOVER_MEMCARDMODEL_HPP__
#define __MCRECOVER_MEMCARDMODEL_HPP__

// MemCard class.
class MemCard;

// Qt includes.
#include <QtCore/QAbstractListModel>

class MemCardModelPrivate;

class MemCardModel : public QAbstractListModel
{
	Q_OBJECT
	
	public:
		MemCardModel(QObject *parent = 0);
		~MemCardModel();
		
	private:
		friend class MemCardModelPrivate;
		MemCardModelPrivate *const d;
		Q_DISABLE_COPY(MemCardModel);
	
	public:
		enum Column
		{
			COL_ICON,		// Icon.
			COL_BANNER,		// Banner.
			COL_DESCRIPTION,	// Description. (both fields)
			COL_SIZE,		// Size (in blocks)
			COL_MTIME,		// Last modified time.
			COL_PERMISSION,		// Permission string.
			COL_GAMECODE,		// Game code. (including company)
			COL_FILENAME,		// Filename.
			
			COL_MAX
		};
		
		// Qt Model/View interface.
		int rowCount(const QModelIndex& parent = QModelIndex()) const;
		int columnCount(const QModelIndex& parent = QModelIndex()) const;
		
		QVariant data(const QModelIndex& index, int role) const;
		QVariant headerData(int section, Qt::Orientation orientation, int role) const;
		
		/**
		 * Set the memory card to use in this model.
		 * @param card Memory card.
		 */
		void setMemCard(MemCard *card);
};

#endif /* __MCRECOVER_MEMCARDMODEL_HPP__ */
