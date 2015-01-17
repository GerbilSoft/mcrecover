/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * SAEventFlagsModel.hpp: QAbstractListModel for SAEventFlags.             *
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

#ifndef __MCRECOVER_EDIT_SONICADVENTURE_SAEVENTFLAGSMODEL_HPP__
#define __MCRECOVER_EDIT_SONICADVENTURE_SAEVENTFLAGSMODEL_HPP__

// Qt includes.
#include <QtCore/QAbstractListModel>

class SAEventFlagsModelPrivate;
class SAEventFlagsModel : public QAbstractListModel
{
	Q_OBJECT
	
	public:
		SAEventFlagsModel(QObject *parent = 0);
		virtual ~SAEventFlagsModel();

	protected:
		SAEventFlagsModelPrivate *const d_ptr;
		Q_DECLARE_PRIVATE(SAEventFlagsModel)
	private:
		Q_DISABLE_COPY(SAEventFlagsModel)

	public:
		// Qt Model/View interface.
		int rowCount(const QModelIndex& parent = QModelIndex()) const;
		int columnCount(const QModelIndex& parent = QModelIndex()) const;

		QVariant data(const QModelIndex& index, int role) const;
		QVariant headerData(int section, Qt::Orientation orientation, int role) const;
};

#endif /* __MCRECOVER_EDIT_SONICADVENTURE_SAEVENTFLAGSMODEL_HPP__ */
