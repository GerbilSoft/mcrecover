/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * SAEventFlagsModel.cpp: QAbstractListModel for SAEventFlags.             *
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

// Reference: http://programmingexamples.net/wiki/Qt/ModelView/AbstractListModelCheckable

#include "SAEventFlagsModel.hpp"
#include "sa_defs.h"

// C includes.
#include <limits.h>

// Qt includes.
#include <QtCore/QHash>

// Event flags.
#include "SAEventFlags.hpp"

/** SAEventFlagsModelPrivate **/

class SAEventFlagsModelPrivate
{
	public:
		SAEventFlagsModelPrivate(SAEventFlagsModel *q);

	protected:
		SAEventFlagsModel *const q_ptr;
		Q_DECLARE_PUBLIC(SAEventFlagsModel)
	private:
		Q_DISABLE_COPY(SAEventFlagsModelPrivate)

	public:
		// Sonic Adeventure event flags.
		// TODO: Signals for data changes?
		SAEventFlags *eventFlags;
};

SAEventFlagsModelPrivate::SAEventFlagsModelPrivate(SAEventFlagsModel *q)
	: q_ptr(q)
	, eventFlags(nullptr)
{ }

/** SAEventFlagsModel **/

SAEventFlagsModel::SAEventFlagsModel(QObject *parent)
	: QAbstractListModel(parent)
	, d_ptr(new SAEventFlagsModelPrivate(this))
{ }

SAEventFlagsModel::~SAEventFlagsModel()
{
	Q_D(SAEventFlagsModel);
	delete d;
}

/** Qt Model/View interface. **/

int SAEventFlagsModel::rowCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent);
	Q_D(const SAEventFlagsModel);
	// TODO: Bitmask.
	return (d->eventFlags ? d->eventFlags->count() : 0);
}

int SAEventFlagsModel::columnCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent);
	Q_D(const SAEventFlagsModel);
	// Only one column: Event name.
	return (d->eventFlags ? 1 : 0);
}

// FIXME: Backport some stuff to MemCardModel.
QVariant SAEventFlagsModel::data(const QModelIndex& index, int role) const
{
	Q_D(const SAEventFlagsModel);
	if (!d->eventFlags)
		return QVariant();
	if (!index.isValid())
		return QVariant();
	if (index.row() < 0 || index.row() >= rowCount())
		return QVariant();
	if (index.column() != 0)
		return QVariant();

	// TODO: Map the row number to the currently displayed list.
	const int event = index.row();

	switch (role) {
		case Qt::DisplayRole:
			return d->eventFlags->description(event);

		case Qt::CheckStateRole:
			// TODO
			return (d->eventFlags->flag(event) ? Qt::Checked : Qt::Unchecked);

		default:
			break;
	}

	// Default value.
	return QVariant();
}

QVariant SAEventFlagsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	Q_UNUSED(orientation);
	Q_D(const SAEventFlagsModel);
	if (!d->eventFlags)
		return QVariant();
	if (section != 0)
		return QVariant();

	switch (role) {
		case Qt::DisplayRole:
			if (section == 0)
				return tr("Event");
			break;

		default:
			break;
	}

	// Default value.
	return QVariant();
}

Qt::ItemFlags SAEventFlagsModel::flags(const QModelIndex &index) const
{
	Q_D(const SAEventFlagsModel);
	if (!d->eventFlags)
		return Qt::NoItemFlags;
	if (!index.isValid())
		return Qt::NoItemFlags;
	if (index.row() < 0 || index.row() >= rowCount())
		return Qt::NoItemFlags;

	return (Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
}

bool SAEventFlagsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if (!index.isValid())
		return false;
	const int row = index.row();
	if (row < 0 || row >= rowCount())
		return false;
	if (index.column() != 0)
		return false;

	// TODO: Map the row number to the currently displayed list.
	Q_D(SAEventFlagsModel);
	const int event = row;

	switch (role) {
		case Qt::CheckStateRole:
			// Event flag value has changed.
			// TODO: Map row to event ID.
			d->eventFlags->setFlag(event, (value.toUInt() == Qt::Checked));
			break;

		default:
			// Unsupported.
			return false;
	}

	// Data has changed.
	emit dataChanged(index, index);
	return true;
}

/** Data access. **/

/**
 * Get the SAEventFlags this model is showing.
 * @return SAEventFlags this model is showing.
 */
SAEventFlags *SAEventFlagsModel::eventFlags(void) const
{
	Q_D(const SAEventFlagsModel);
	return d->eventFlags;
}

/**
 * Set the SAEventFlags for this model to show.
 * @param eventFlags SAEventFlags to show.
 */
void SAEventFlagsModel::setEventFlags(SAEventFlags *eventFlags)
{
	Q_D(SAEventFlagsModel);
	// TODO: Better signals?
	emit layoutAboutToBeChanged();
	d->eventFlags = eventFlags;
	emit layoutChanged();
}
