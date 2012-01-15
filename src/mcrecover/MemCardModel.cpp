/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * MemCardModel.cpp: QAbstractListModel for MemCard.                       *
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

#include "MemCardModel.hpp"

// MemCard classes.
#include "MemCard.hpp"
#include "MemCardFile.hpp"

// Qt includes.
#include <QtGui/QFont>

/** MemCardModelPrivate **/

class MemCardModelPrivate
{
	public:
		MemCardModelPrivate(MemCardModel *q);
	
	private:
		MemCardModel *const q;
		Q_DISABLE_COPY(MemCardModelPrivate);
	
	public:
		MemCard *card;
};

MemCardModelPrivate::MemCardModelPrivate(MemCardModel *q)
	: q(q)
{ }


/** MemCardModel **/

MemCardModel::MemCardModel(QObject *parent)
	: QAbstractListModel(parent)
	, d(new MemCardModelPrivate(this))
{ }

MemCardModel::~MemCardModel()
	{ delete d; }


int MemCardModel::rowCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent);
	return (d->card != NULL ? d->card->numFiles() : 0);
}

int MemCardModel::columnCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent);
	return COL_MAX;
}


QVariant MemCardModel::data(const QModelIndex& index, int role) const
{
	if (!d->card || !index.isValid())
		return QVariant();
	if (index.row() >= d->card->numFiles())
		return QVariant();
	
	// Get the memory card file.
	MemCardFile *file = d->card->getFile(index.row());
	const int section = index.column();
	
	switch (role)
	{
		case Qt::DisplayRole:
			switch (section)
			{
				case COL_ICON:
				case COL_BANNER:
					// TODO
					return QVariant();
				
				case COL_DESCRIPTION:
					return file->gameDesc() + QChar(L'\n') + file->fileDesc();
				
				case COL_SIZE:
					return file->size();
				
				case COL_MTIME:
					return file->lastModified().toString(Qt::DefaultLocaleShortDate);
				
				case COL_PERMISSION:
					return file->permissionAsString();
				
				case COL_GAMECODE:
					return (file->gamecode() + file->company());
				
				case COL_FILENAME:
					return file->filename();
				
				default:
					break;
			}
			break;
		
		case Qt::TextAlignmentRole:
			switch (section)
			{
				case COL_SIZE:
				case COL_PERMISSION:
				case COL_GAMECODE:
					// These columns should be center-aligned horizontally.
					return (int)(Qt::AlignHCenter | Qt::AlignVCenter);
				
				default:
					// Everything should be center-aligned vertically.
					return Qt::AlignVCenter;
			}
			break;
		
		case Qt::FontRole:
			switch (section)
			{
				case COL_SIZE:
				case COL_PERMISSION:
				case COL_GAMECODE:
				{
					// These columns should be monospaced.
					QFont font(QLatin1String("Monospace"));
					font.setStyleHint(QFont::TypeWriter); // or QFont::Monospace?
					return font;
				}
				
				default:
					break;
			}
			break;
		
		default:
			break;
	}
	
	// Default value.
	return QVariant();
}


QVariant MemCardModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	switch (role)
	{
		case Qt::DisplayRole:
			switch (section)
			{
				case COL_ICON:		return tr("Icon");
				case COL_BANNER:	return tr("Banner");
				case COL_DESCRIPTION:	return tr("Description");
				case COL_SIZE:		return tr("Size");
				case COL_MTIME:		return tr("Last Modified");
				case COL_PERMISSION:	return tr("Permission");
				case COL_GAMECODE:	return tr("Game ID");
				case COL_FILENAME:	return tr("Filename");
				default:
					break;
			}
			break;
		
		case Qt::TextAlignmentRole:
			switch (section)
			{
				case COL_ICON:
				case COL_SIZE:
				case COL_PERMISSION:
				case COL_GAMECODE:
					// Center-align the text.
					return Qt::AlignHCenter;
				
				default:
					break;
			}
			break;
	}
	
	// Default value.
	return QVariant();
}


/**
 * Set the memory card to use in this model.
 * @param card Memory card.
 */
void MemCardModel::setMemCard(MemCard *card)
{
	d->card = card;
	
	QModelIndex topLeft;
	QModelIndex bottomRight;
	
	if (card == NULL)
	{
		// No memory card. Use blank indexes.
		topLeft = createIndex(0, 0, 0);
		bottomRight = createIndex(0, COL_MAX, 0);
	}
	else
	{
		// Memory card specified.
		topLeft = createIndex(0, 0, 0);
		
		int lastFile = (card->numFiles() - 1);
		if (lastFile < 0)
			lastFile++;
		bottomRight = createIndex(lastFile, COL_MAX, 0);
	}
	
	emit dataChanged(topLeft, bottomRight);
}
