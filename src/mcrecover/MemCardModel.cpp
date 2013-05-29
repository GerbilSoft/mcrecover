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

// C includes.
#include <limits.h>

// Qt includes.
#include <QtGui/QFont>
#include <QtCore/QHash>
#include <QtCore/QTimer>

/** MemCardModelPrivate **/

class MemCardModelPrivate
{
	public:
		MemCardModelPrivate(MemCardModel *q);
		~MemCardModelPrivate();

	private:
		MemCardModel *const q;
		Q_DISABLE_COPY(MemCardModelPrivate);

	public:
		MemCard *card;

		struct AnimData {
			uint8_t frame;		// Current frame.
			uint8_t lastValidFrame;	// Last valid frame.
			bool frameHasIcon;	// If false, use previous frame.
			uint8_t delayCnt;	// Delay counter.
			uint8_t delayLen;	// Delay length.
			uint8_t mode;		// Animation mode.
			bool direction;		// Current direction for CARD_ANIM_BOUNCE.
		};
		QHash<MemCardFile*, AnimData*> animState;

		/**
		 * Initialize the animation state.
		 */
		void initAnimState(void);

		// Animation timer.
		QTimer animTimer;

		/**
		 * Animation timer "slot".
		 */
		void animTimerSlot(void);

		// Visible columns.
		uint32_t bfColumnsVisible;	// bitfield
		QVector<int> vIndirectCols;	// indirect columns
		bool vIndirectCols_dirty;
		void refreshVisibleColumns(void);
};

MemCardModelPrivate::MemCardModelPrivate(MemCardModel *q)
	: q(q)
	, card(NULL)
	, animTimer(new QTimer(q))

	// Default to all columns visible.
	, bfColumnsVisible((1 << (MemCardModel::COL_MAX + 1)) - 1)
	, vIndirectCols_dirty(true)
{
	// Connect animTimer's timeout() signal.
	QObject::connect(&animTimer, SIGNAL(timeout()),
			 q, SLOT(animTimerSlot()));
}

MemCardModelPrivate::~MemCardModelPrivate()
{
	animTimer.stop();

	// TODO: Check for race conditions.
	qDeleteAll(animState);
	animState.clear();
}


/**
 * Initialize the animation state.
 */
void MemCardModelPrivate::initAnimState(void)
{
	animTimer.stop();

	// TODO: Check for race conditions.
	qDeleteAll(animState);
	animState.clear();

	if (!card)
		return;

	// Initialize the animation state.
	for (int i = 0; i < card->numFiles(); i++) {
		MemCardFile *file = card->getFile(i);

		int numIcons = file->numIcons();
		if (numIcons <= 1)
			continue;

		// Get the file data.
		AnimData *animData = new AnimData();
		animData->frame = 0;
		animData->lastValidFrame = 0;
		animData->frameHasIcon = !(file->icon(animData->frame).isNull());
		animData->delayCnt = 0;
		animData->delayLen = file->iconDelay(0);
		animData->mode = file->iconAnimMode();
		animData->direction = false;
		animState.insert(file, animData);
	}

	// TODO: Figure out the correct timer interval.
	// This will use 125ms for 'fast' icons.
	if (!animState.isEmpty())
		animTimer.start(125);
}


/**
 * Animation timer "slot".
 */
void MemCardModelPrivate::animTimerSlot(void)
{
	if (!card) {
		animTimer.stop();
		return;
	}

	// Check for icon animations.
	for (int i = 0; i < card->numFiles(); i++) {
		MemCardFile *file = card->getFile(i);
		if (!animState.contains(file))
			continue;

		AnimData *animData = animState.value(file);

		// Check the delay counter.
		animData->delayCnt++;
		if (animData->delayCnt < animData->delayLen) {
			// Animation delay hasn't expired yet.
			continue;
		}

		// Animation delay has expired.
		// Go to the next frame.
		if (!animData->direction) {
			// Animation is moving forwards.
			// Check if we're at the last frame.
			if (animData->frame == (CARD_MAXICONS - 1) ||
			    (file->iconDelay(animData->frame + 1) == CARD_SPEED_END))
			{
				// Last frame.
				if (animData->mode == CARD_ANIM_BOUNCE) {
					// "Bounce" animation. Start playing backwards.
					animData->direction = true;
					animData->frame--;	// Go to the previous frame.
				} else {
					// "Looping" animation.
					// Reset to frame 0.
					animData->frame = 0;
				}
			} else {
				// Not the last frame.
				// Go to the next frame.
				animData->frame++;
			}
		} else {
			// Animation is moving backwards. ("Bounce" animation only.)
			// Check if we're at the first frame.
			if (animData->frame == 0) {
				// First frame. Start playing forwards.
				animData->direction = false;
				animData->frame++;	// Go to the next frame.
			} else {
				// Not the first frame.
				// Go to the previous frame.
				animData->frame--;
			}
		}

		// Update the frame delay data.
		animData->delayCnt = 0;
		animData->delayLen = file->iconDelay(animData->frame);

		// Check if this frame has an icon.
		animData->frameHasIcon = !file->icon(animData->frame).isNull();
		if (animData->frameHasIcon) {
			// Frame has an icon. Save this frame as the last valid frame.
			animData->lastValidFrame = animData->frame;
		}

		// Notify the UI that the icon has changed.
		QModelIndex iconIndex = q->createIndex(i, MemCardModel::COL_ICON, 0);
		emit q->dataChanged(iconIndex, iconIndex);
	}
}


/**
 * Refresh the indirect columns vector.
 */
void MemCardModelPrivate::refreshVisibleColumns(void)
{
	vIndirectCols.clear();
	vIndirectCols.reserve(MemCardModel::COL_MAX);

	int col = 0;
	for (uint32_t bf = bfColumnsVisible; bf != 0; bf >>= 1, col++) {
		if (bf & 1)
			vIndirectCols.push_back(col);
	}

	// vIndirectCols is no longer dirty.
	vIndirectCols_dirty = false;

	// Notify the UI that the layout has changed.
	emit q->layoutChanged();
}


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
	
	// Make sure vIndirectCols is up to date.
	// NOTE: This is a const function, but it modifies the private class!
	if (d->vIndirectCols_dirty)
		d->refreshVisibleColumns();
	
	return d->vIndirectCols.size();
}


QVariant MemCardModel::data(const QModelIndex& index, int role) const
{
	if (!d->card || !index.isValid())
		return QVariant();
	if (index.row() >= rowCount())
		return QVariant();

	// Get the memory card file.
	MemCardFile *file = d->card->getFile(index.row());

	// Make sure vIndirectCols is up to date.
	// NOTE: This is a const function, but it modifies the private class!
	if (d->vIndirectCols_dirty)
		d->refreshVisibleColumns();

	// Get the column identifier.
	int section = index.column();
	if (section >= d->vIndirectCols.size())
		return QVariant();
	section = d->vIndirectCols.at(section);

	switch (role) {
		case Qt::DisplayRole:
			switch (section) {
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

		case Qt::DecorationRole:
			// Images must use Qt::DecorationRole.
			switch (section) {
				case COL_ICON:
					// Check if this is an animated icon.
					if (d->animState.contains(file)) {
						// Animated icon.
						MemCardModelPrivate::AnimData *animData = d->animState.value(file);
						if (animData->frameHasIcon)
							return file->icon(animData->frame);
						else
							return file->icon(animData->lastValidFrame);
					} else {
						// Not an animated icon.
						// Return the first icon.
						return file->icon(0);
					}

				case COL_BANNER:
					return file->banner();

				default:
					break;
			}
			break;

		case Qt::TextAlignmentRole:
			switch (section) {
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
			switch (section) {
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
	Q_UNUSED(orientation);

	// Make sure vIndirectCols is up to date.
	// NOTE: This is a const function, but it modifies the private class!
	if (d->vIndirectCols_dirty)
		d->refreshVisibleColumns();

	// Get the column identifier.
	if (section >= d->vIndirectCols.size())
		return QVariant();
	section = d->vIndirectCols.at(section);

	switch (role) {
		case Qt::DisplayRole:
			switch (section) {
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
			switch (section) {
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
	emit layoutAboutToBeChanged();

	// Disconnect the MemCard's changed() signal if a MemCard is already set.
	if (d->card) {
		// TODO: More fine-grained changed() for the specific files.
		disconnect(d->card, SIGNAL(changed()),
			   this, SLOT(memCardChangedSlot()));
	}

	d->card = card;

	if (card != NULL) {
		// Initialize the animation state.
		d->initAnimState();

		// Connect the MemCard's changed() signal.
		// TODO: More fine-grained changed() for the specific files.
		connect(card, SIGNAL(changed()),
			this, SLOT(memCardChangedSlot()));
	}

	// Layout has changed.
	emit layoutChanged();
}


/**
 * Animation timer slot.
 * Wrapper for MemCardModelPrivate::animTimerSlot().
 */
void MemCardModel::animTimerSlot(void)
	{ d->animTimerSlot(); }


/**
 * MemCard has changed.
 * TODO: More fine-grained slot for specific files.
 */
void MemCardModel::memCardChangedSlot(void)
{
	// NOTE: Not sure if layoutAboutToBeChanged() should be emitted here...
	// TODO: Emit "aboutToChange()" from MemCard?
	emit layoutAboutToBeChanged();
	emit layoutChanged();
}


/**
 * Check if a column is visible.
 * @param column Column number.
 * @return True if the column is visible; false if not.
 */
bool MemCardModel::isColumnVisible(int column)
{
	if (column < 0 || column >= COL_MAX)
		return false;

	return !!(d->bfColumnsVisible & (1 << column));
}


/**
 * Set a column's visibility status.
 * @param column Column number.
 * @param visible True to show the column; false to hide it.
 */
void MemCardModel::setColumnVisible(int column, bool visible)
{
	if (column < 0 || column >= COL_MAX)
		return;
	if (isColumnVisible(column) == visible)
		return;

	// Change the visibility of this column.
	if (visible)
		d->bfColumnsVisible |= (1 << column);
	else
		d->bfColumnsVisible &= ~(1 << column);

	// d->vIndirectCols needs to be updated.
	d->vIndirectCols_dirty = true;

	// Layout has changed.
	emit layoutChanged();
}
