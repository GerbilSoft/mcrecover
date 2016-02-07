/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * ByteFlagsModel.cpp: QAbstractListModel for ByteFlags.                   *
 *                                                                         *
 * Copyright (c) 2012-2016 by David Korth.                                 *
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

#include "ByteFlagsModel.hpp"

// C includes.
#include <limits.h>

// Qt includes.
#include <QtCore/QHash>
#include <QtGui/QApplication>
#include <QtGui/QStyle>

// Event flags.
#include "ByteFlags.hpp"

/** ByteFlagsModelPrivate **/

class ByteFlagsModelPrivate
{
	public:
		ByteFlagsModelPrivate(ByteFlagsModel *q);

	protected:
		ByteFlagsModel *const q_ptr;
		Q_DECLARE_PUBLIC(ByteFlagsModel)
	private:
		Q_DISABLE_COPY(ByteFlagsModelPrivate)

	public:
		// ByteFlags.
		ByteFlags *byteFlags;

		/**
		 * Cached copy of byteFlags->count().
		 * This value is needed after the card is destroyed,
		 * so we need to cache it here, since the destroyed()
		 * slot might be run *after* the Card is deleted.
		 */
		int flagCount;
};

ByteFlagsModelPrivate::ByteFlagsModelPrivate(ByteFlagsModel *q)
	: q_ptr(q)
	, byteFlags(nullptr)
	, flagCount(0)
{ }

/** ByteFlagsModel **/

ByteFlagsModel::ByteFlagsModel(QObject *parent)
	: super(parent)
	, d_ptr(new ByteFlagsModelPrivate(this))
{ }

ByteFlagsModel::~ByteFlagsModel()
{
	Q_D(ByteFlagsModel);
	delete d;
}

/** Qt Model/View interface. **/

int ByteFlagsModel::rowCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent)
	Q_D(const ByteFlagsModel);
	return (d->byteFlags ? d->byteFlags->count() : 0);
}

int ByteFlagsModel::columnCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent);
	Q_D(const ByteFlagsModel);

	return (d->byteFlags ? COL_MAX : 0);
}

// FIXME: Backport some stuff to MemCardModel.
QVariant ByteFlagsModel::data(const QModelIndex& index, int role) const
{
	Q_D(const ByteFlagsModel);
	if (!d->byteFlags)
		return QVariant();
	if (!index.isValid())
		return QVariant();
	if (index.row() < 0 || index.row() >= rowCount())
		return QVariant();

	switch (role) {
		case Qt::DisplayRole:
			switch (index.column()) {
				case COL_ID:
					// TODO: Customizable ID base.
					return (index.row() + 1);
				case COL_DESCRIPTION:
					return d->byteFlags->description(index.row());
				default:
					break;
			}
			break;

		case Qt::DecorationRole:
			if (index.column() == COL_CHARACTER)
				return d->byteFlags->icon(index.row());
			break;

		case Qt::CheckStateRole:
			if (index.column() >= COL_BIT0 && index.column() <= COL_BIT7) {
				const int bit = (index.column() - COL_BIT0);
				if (!d->byteFlags->flagType(bit).isEmpty()) {
					return ((d->byteFlags->flag(index.row()) & (1 << bit))
						? Qt::Checked
						: Qt::Unchecked);
				} else {
					// Not a valid bit.
					break;
				}
			}
			break;

		case Qt::TextAlignmentRole:
			// Checkboxes should be horizontally centered.
			if (index.column() >= COL_BIT0 && index.column() <= COL_BIT7)
				return Qt::AlignHCenter;
			break;

		case Qt::SizeHintRole:
			if (index.column() >= COL_BIT0 && index.column() <= COL_BIT7) {
				// Checkbox.
				QStyle *style = qApp->style();
				return QSize(style->pixelMetric(QStyle::PM_IndicatorWidth),
					     style->pixelMetric(QStyle::PM_IndicatorHeight));
			} else if (index.column() == COL_CHARACTER) {
				// HACK: Increase icon/banner width on Windows.
				// Figure out a better method later.
				// TODO: Get correct icon size from the ByteFlags object.
				#ifdef Q_OS_WIN
					static const int iconWadj = 8;
				#else
					static const int iconWadj = 0;
				#endif
				return QSize(16 + iconWadj, 16);
			}
			break;

		default:
			break;
	}

	// Default value.
	return QVariant();
}

QVariant ByteFlagsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	Q_UNUSED(orientation);
	Q_D(const ByteFlagsModel);
	if (!d->byteFlags)
		return QVariant();

	switch (role) {
		case Qt::DisplayRole:
			switch (section) {
				case COL_ID:
					return tr("ID");
				case COL_DESCRIPTION:
					return d->byteFlags->objectType();
				case COL_CHARACTER:
					//: Abbreviation of "character".
					return tr("Char");

				// Bits
				case COL_BIT0: case COL_BIT1: case COL_BIT2: case COL_BIT3:
				case COL_BIT4: case COL_BIT5: case COL_BIT6: case COL_BIT7:
				{
					const int bit = (section - COL_BIT0);
					return d->byteFlags->flagType(bit);
				}

				default:
					break;
			}
			break;

		case Qt::TextAlignmentRole:
			if (section >= COL_BIT0 && section <= COL_BIT7) {
				// Center-align the text.
				return Qt::AlignHCenter;
			}
			break;

		default:
			break;
	}

	// Default value.
	return QVariant();
}

Qt::ItemFlags ByteFlagsModel::flags(const QModelIndex &index) const
{
	Q_D(const ByteFlagsModel);
	if (!d->byteFlags)
		return Qt::NoItemFlags;
	else if (!index.isValid())
		return Qt::NoItemFlags;
	else if (index.row() < 0 || index.row() >= rowCount())
		return Qt::NoItemFlags;

	if (index.column() >= COL_BIT0 && index.column() <= COL_BIT7) {
		// Check if this bit is valid.
		const int bit = (index.column() - COL_BIT0);
		if (!d->byteFlags->flagType(bit).isEmpty()) {
			// Flag has a defined type.
			return Qt::ItemIsEnabled | Qt::ItemIsUserCheckable;
		} else {
			// Flag is not defined.
			return Qt::NoItemFlags;
		}
	}

	// Items are enabled by default.
	return Qt::ItemIsEnabled;
}

bool ByteFlagsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	Q_D(ByteFlagsModel);
	if (!d->byteFlags)
		return false;
	if (!index.isValid())
		return false;
	if (index.row() < 0 || index.row() >= rowCount())
		return false;

	switch (role) {
		case Qt::CheckStateRole:
			if (index.column() >= COL_BIT0 && index.column() <= COL_BIT7) {
				// Flag value has changed.
				uint8_t flag = d->byteFlags->flag(index.row());
				const int bit = (index.column() - COL_BIT0);
				if (value.toUInt() == Qt::Checked)
					flag |= (1 << bit);
				else
					flag &= ~(1 << bit);
				d->byteFlags->setFlag(index.row(), flag);
			} else {
				// Unsupported column.
				return false;
			}
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
 * Get the ByteFlags this model is showing.
 * @return ByteFlags this model is showing.
 */
ByteFlags *ByteFlagsModel::byteFlags(void) const
{
	Q_D(const ByteFlagsModel);
	return d->byteFlags;
}

/**
 * Set the ByteFlags for this model to show.
 * @param byteFlags ByteFlags to show.
 */
void ByteFlagsModel::setByteFlags(ByteFlags *byteFlags)
{
	Q_D(ByteFlagsModel);

	// Disconnect the ByteFlags's destroyed() signal if ByteFlags is already set.
	if (d->byteFlags) {
		// Notify the view that we're about to remove all rows.
		// TODO: flagCount should already be cached...
		const int flagCount = d->byteFlags->count();
		if (flagCount > 0)
			beginRemoveRows(QModelIndex(), 0, (flagCount - 1));

		// Disconnect the ByteFlags's signals.
		disconnect(d->byteFlags, SIGNAL(destroyed(QObject*)),
			   this, SLOT(byteFlags_destroyed_slot(QObject*)));
		disconnect(d->byteFlags, SIGNAL(flagChanged(int,uint8_t)),
			   this, SLOT(byteFlags_flagChanged_slot(int)));
		disconnect(d->byteFlags, SIGNAL(flagsChanged(int,int)),
			   this, SLOT(byteFlags_flagsChanged_slot(int,int)));

		d->byteFlags = nullptr;

		// Done removing rows.
		d->flagCount = 0;
		if (flagCount > 0)
			endRemoveRows();
	}

	// Connect the byteFlags's destroyed() signal.
	if (byteFlags) {
		// Notify the view that we're about to add rows.
		const int flagCount = byteFlags->count();
		if (flagCount > 0)
			beginInsertRows(QModelIndex(), 0, (flagCount - 1));

		// Set the ByteFlags.
		d->byteFlags = byteFlags;

		// Connect the ByteFlags's signals.
		connect(d->byteFlags, SIGNAL(destroyed(QObject*)),
			this, SLOT(byteFlags_destroyed_slot(QObject*)));
		connect(d->byteFlags, SIGNAL(flagChanged(int,uint8_t)),
			this, SLOT(byteFlags_flagChanged_slot(int)));
		connect(d->byteFlags, SIGNAL(flagsChanged(int,int)),
			this, SLOT(byteFlags_flagsChanged_slot(int,int)));

		// Done adding rows.
		if (flagCount > 0) {
			d->flagCount = flagCount;
			endInsertRows();
		}
	}
}

/** Slots. **/

/**
 * ByteFlags object was destroyed.
 * @param obj QObject that was destroyed.
 */
void ByteFlagsModel::byteFlags_destroyed_slot(QObject *obj)
{
	Q_D(ByteFlagsModel);

	if (obj == d->byteFlags) {
		// Our Card was destroyed.
		d->byteFlags = nullptr;
		int old_flagCount = d->flagCount;
		if (old_flagCount > 0)
			beginRemoveRows(QModelIndex(), 0, (old_flagCount - 1));
		d->flagCount = 0;
		if (old_flagCount > 0)
			endRemoveRows();
	}
}

/**
 * ByteFlags: An object's flags have been changed.
 * @param id Object ID.
 */
void ByteFlagsModel::byteFlags_flagChanged_slot(int id)
{
	Q_D(ByteFlagsModel);
	if (!d->byteFlags)
		return;
	else if (id < 0 || id >= d->byteFlags->count())
		return;

	// COL_BIT0 through COL_BIT7 have checkboxes.
	emit dataChanged(createIndex(id, COL_BIT0), createIndex(id, COL_BIT7));
}

/**
 * ByteFlags: Multiple objects' flags have been changed.
 * @param firstID ID of first object whose flags have changed.
 * @param lastID ID of last object whose flags have changed.
 */
void ByteFlagsModel::byteFlags_flagsChanged_slot(int firstID, int lastID)
{
	Q_D(ByteFlagsModel);
	if (!d->byteFlags)
		return;
	else if (firstID < 0 || firstID >= d->byteFlags->count())
		return;
	else if (lastID < 0 || lastID >= d->byteFlags->count())
		return;

	// COL_BIT0 through COL_BIT7 have checkboxes.
	emit dataChanged(createIndex(firstID, COL_BIT0), createIndex(lastID, COL_BIT7));
}

/**
 * Get the desired page size from the ByteFlags.
 * @return Page size.
 */
int ByteFlagsModel::pageSize(void) const
{
       Q_D(const ByteFlagsModel);
       if (d->byteFlags) {
               return d->byteFlags->pageSize();
       }
       return 0;
}

/**
 * Get the name for a given page of data.
 *
 * If pagination is enabled (pageSize > 0), this function is
 * used to determine the text for the corresponding tab.
 *
 * @param page Page number.
 * @return Page name.
 */
QString ByteFlagsModel::pageName(int page) const
{
       Q_D(const ByteFlagsModel);
       if (d->byteFlags) {
               return d->byteFlags->pageName(page);
       }
       return QString();
}
