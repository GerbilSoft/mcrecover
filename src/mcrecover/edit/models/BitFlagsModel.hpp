/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * BitFlagsModel.hpp: QAbstractListModel for BitFlags.                     *
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

#ifndef __MCRECOVER_EDIT_MODELS_BITFLAGSMODEL_HPP__
#define __MCRECOVER_EDIT_MODELS_BITFLAGSMODEL_HPP__

// Qt includes.
#include <QtCore/QAbstractListModel>

class BitFlags;

class BitFlagsModelPrivate;
class BitFlagsModel : public QAbstractListModel
{
	Q_OBJECT
	typedef QAbstractListModel super;

	// TODO: Q_PROPERTY() for eventFlags.
	Q_PROPERTY(int pageSize READ pageSize)

	public:
		explicit BitFlagsModel(QObject *parent = 0);
		virtual ~BitFlagsModel();

	protected:
		BitFlagsModelPrivate *const d_ptr;
		Q_DECLARE_PRIVATE(BitFlagsModel)
	private:
		Q_DISABLE_COPY(BitFlagsModel)

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
			COL_CHECKBOX,		// Checkbox
			COL_ID,			// Flag ID
			COL_DESCRIPTION,	// Description

			COL_MAX
		};

	public:
		/** Data access. **/

		/**
		 * Get the BitFlags this model is showing.
		 * @return BitFlags this model is showing.
		 */
		BitFlags *bitFlags(void) const;

		/**
		 * Set the BitFlags for this model to show.
		 * @param bitFlags BitFlags to show.
		 */
		void setBitFlags(BitFlags *bitFlags);

		/**
		 * Get the desired page size from the BitFlags.
		 * @return Page size.
		 */
		int pageSize(void) const;

		/**
		 * Get the name for a given page of data.
		 *
		 * If pagination is enabled (pageSize > 0), this function is
		 * used to determine the text for the corresponding tab.
		 *
		 * @param page Page number.
		 * @return Page name.
		 */
		QString pageName(int page) const;

	protected slots:
		/**
		 * BitFlags object was destroyed.
		 * @param obj QObject that was destroyed.
		 */
		void bitFlags_destroyed_slot(QObject *obj = 0);

		/**
		 * BitFlags: A flag has been changed.
		 * @param flag Flag ID.
		 */
		void bitFlags_flagChanged_slot(int flag);

		/**
		 * BitFlags: Multiple flags have been changed.
		 * @param firstFlag First flag that has changed.
		 * @param lastFlag Last flag that has changed.
		 */
		void bitFlags_flagsChanged_slot(int firstFlag, int lastFlag);
};

#endif /* __MCRECOVER_EDIT_MODELS_BITFLAGSMODEL_HPP__ */
