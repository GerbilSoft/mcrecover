/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * ByteFlagsModel.hpp: QAbstractListModel for ByteFlags.                   *
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

#ifndef __MCRECOVER_EDIT_SONICADVENTURE_BYTEFLAGSMODEL_HPP__
#define __MCRECOVER_EDIT_SONICADVENTURE_BYTEFLAGSMODEL_HPP__

// Qt includes.
#include <QtCore/QAbstractListModel>

class ByteFlags;
class ByteFlagsModelPrivate;
class ByteFlagsModel : public QAbstractListModel
{
	Q_OBJECT

	// TODO: Q_PROPERTY() for eventFlags.

	public:
		ByteFlagsModel(QObject *parent = 0);
		virtual ~ByteFlagsModel();

	protected:
		ByteFlagsModelPrivate *const d_ptr;
		Q_DECLARE_PRIVATE(ByteFlagsModel)
	private:
		Q_DISABLE_COPY(ByteFlagsModel)

	public:
		/** Qt Model/View interface. **/
		int rowCount(const QModelIndex& parent = QModelIndex()) const;
		int columnCount(const QModelIndex& parent = QModelIndex()) const;

		QVariant data(const QModelIndex& index, int role) const;
		QVariant headerData(int section, Qt::Orientation orientation, int role) const;
		Qt::ItemFlags flags(const QModelIndex &index) const;

		bool setData(const QModelIndex &index, const QVariant &value, int role);

		// Columns.
		enum Column {
			// Object ID
			COL_ID = 0,

			// Character
			COL_CHARACTER,

			// Bits
			COL_BIT0, COL_BIT1, COL_BIT2, COL_BIT3,
			COL_BIT4, COL_BIT5, COL_BIT6, COL_BIT7,

			// Description.
			// Needs to go after the bitflags because
			// Qt doesn't like making anything but the
			// final column an expanding column.
			COL_DESCRIPTION,

			COL_MAX
		};

	public:
		/** Data access. **/

		/**
		 * Get the ByteFlags this model is showing.
		 * @return ByteFlags this model is showing.
		 */
		ByteFlags *byteFlags(void) const;

		/**
		 * Set the ByteFlags for this model to show.
		 * @param byteFlags ByteFlags to show.
		 */
		void setByteFlags(ByteFlags *byteFlags);

	protected slots:
		/**
		 * ByteFlags object was destroyed.
		 * @param obj QObject that was destroyed.
		 */
		void byteFlags_destroyed_slot(QObject *obj = 0);

		/**
		 * ByteFlags: An object's flags have been changed.
		 * @param id Object ID.
		 */
		void byteFlags_flagChanged_slot(int id);

		/**
		 * ByteFlags: Multiple objects' flags have been changed.
		 * @param firstID ID of first object whose flags have changed.
		 * @param lastID ID of last object whose flags have changed.
		 */
		void byteFlags_flagsChanged_slot(int firstID, int lastID);
};

#endif /* __MCRECOVER_EDIT_SONICADVENTURE_BYTEFLAGSMODEL_HPP__ */
