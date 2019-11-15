/***************************************************************************
 * GameCube Memory Card Recovery Program [libmemcard]                      *
 * MemCardModel.cpp: QAbstractListModel for Card.                          *
 *                                                                         *
 * Copyright (c) 2012-2018 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#include "MemCardModel.hpp"

// Card classes.
#include "Card.hpp"
#include "File.hpp"

// TODO: Get correct icon size from the Card object.
#include "card.h"
#include "util/array_size.h"

// Icon animation helper.
#include "IconAnimHelper.hpp"

// C includes. (C++ namespace)
#include <cassert>
#include <climits>

// Qt includes.
#include <QtCore/QHash>
#include <QtCore/QTimer>
#include <QApplication>
#include <QtGui/QColor>
#include <QtGui/QFont>
#include <QtGui/QPalette>
#include <QStyle>


/** MemCardModelPrivate **/

class MemCardModelPrivate
{
	public:
		explicit MemCardModelPrivate(MemCardModel *q);
		~MemCardModelPrivate();

	protected:
		MemCardModel *const q_ptr;
		Q_DECLARE_PUBLIC(MemCardModel)
	private:
		Q_DISABLE_COPY(MemCardModelPrivate)

	public:
		Card *card;

		QHash<const File*, IconAnimHelper*> animState;

		/**
		 * Initialize the animation state for all files.
		 */
		void initAnimState(void);

		/**
		 * Initialize the animation state for a given file.
		 * @param file File.
		 */
		void initAnimState(const File *file);

		/**
		 * Update the animation timer state.
		 * Starts the timer if animated icons are present; stops the timer if not.
		 */
		void updateAnimTimerState(void);

		// Animation timer.
		QTimer *animTimer;
		// Pause count. If >0, animation is paused.
		int pauseCounter;

		// Style variables.
		struct style_t {
			/**
			 * Initialize the style variables.
			 */
			void init(void);

			// Background colors for "lost" files.
			QBrush brush_lostFile;
			QBrush brush_lostFile_alt;

			// Icon IDs.
			enum IconID {
				ICON_UNKNOWN,	// Checksum is unknown
				ICON_INVALID,	// Checksum is invalid
				ICON_GOOD,	// Checksum is good

				ICON_MAX
			};

			/**
			 * Load an icon.
			 * @param id Icon ID.
			 * @return Icon.
			 */
			QIcon getIcon(IconID id) const;

		private:
			/**
			 * Load an icon from the Qt resources.
			 * @param dir Base directory.
			 * @param name Icon name.
			 */
			static QIcon loadIcon(const QString &dir, const QString &name);

			// Icons for COL_TYPE.
			mutable QIcon m_icons[ICON_MAX];
		};
		style_t style;

		/**
		 * Cached copy of card->fileCount().
		 * This value is needed after the card is destroyed,
		 * so we need to cache it here, since the destroyed()
		 * slot might be run *after* the Card is deleted.
		 */
		int fileCount;

		// Row insert start/end indexes.
		int insertStart;
		int insertEnd;
};

MemCardModelPrivate::MemCardModelPrivate(MemCardModel *q)
	: q_ptr(q)
	, card(nullptr)
	, animTimer(new QTimer(q))
	, pauseCounter(0)
	, fileCount(0)
	, insertStart(-1)
	, insertEnd(-1)
{
	// Connect animTimer's timeout() signal.
	QObject::connect(animTimer, &QTimer::timeout,
			 q, &MemCardModel::animTimerSlot);

	// Initialize the style variables.
	style.init();
}

/**
 * Initialize the style variables.
 */
void MemCardModelPrivate::style_t::init(void)
{
	// TODO: Call this function if the UI style changes.

	// Initialize the background colors for "lost" files.
	QPalette pal = QApplication::palette("QTreeView");
	QColor bgColor_lostFile = pal.base().color();
	QColor bgColor_lostFile_alt = pal.alternateBase().color();

	// Adjust the colors to have a yellow hue.
	int h, s, v;

	// "Lost" file. (Main)
	bgColor_lostFile.getHsv(&h, &s, &v, nullptr);
	h = 60;
	s = (255 - s);
	bgColor_lostFile.setHsv(h, s, v);

	// "Lost" file. (Alternate)
	bgColor_lostFile_alt.getHsv(&h, &s, &v, nullptr);
	h = 60;
	s = (255 - s);
	bgColor_lostFile_alt.setHsv(h, s, v);

	// Save the background colors in QBrush objects.
	brush_lostFile = QBrush(bgColor_lostFile);
	brush_lostFile_alt = QBrush(bgColor_lostFile_alt);
}

/**
 * Load an icon.
 * @param id Icon ID.
 * @return Icon.
 */
QIcon MemCardModelPrivate::style_t::getIcon(IconID id) const
{
	assert(id >= 0);
	assert(id < ICON_MAX);
	if (id < 0 || id >= ICON_MAX) {
		return QIcon();
	}

	if (m_icons[id].isNull()) {
		static const char *const names[] = {
			"dialog-question",
			"dialog-error",
			"dialog-ok-apply"
		};
		static_assert(ARRAY_SIZE(names) == ICON_MAX, "names[] needs to be updated!");
		m_icons[id] = loadIcon(QLatin1String("oxygen"), QLatin1String(names[id]));
		assert(!m_icons[id].isNull());
	}

	return m_icons[id];
}

/**
 * Load an icon from the Qt resources.
 * @param dir Base directory.
 * @param name Icon name.
 */
QIcon MemCardModelPrivate::style_t::loadIcon(const QString &dir, const QString &name)
{
	// Icon sizes.
	// NOTE: Not including 256x256 here.
	static const unsigned int icoSz[] = {16, 22, 24, 32, 48, 64, 128, 0};

	QIcon icon;
	for (const unsigned int *p = icoSz; *p != 0; p++) {
		const QString s_sz = QString::number(*p);
		QString full_path = QLatin1String(":/") +
			dir + QChar(L'/') +
			s_sz + QChar(L'x') + s_sz + QChar(L'/') +
			name + QLatin1String(".png");;
		QPixmap pxm(full_path);
		if (!pxm.isNull()) {
			icon.addPixmap(pxm);
		}
	}

	return icon;
}

MemCardModelPrivate::~MemCardModelPrivate()
{
	animTimer->stop();
	delete animTimer;

	// TODO: Check for race conditions.
	qDeleteAll(animState);
	animState.clear();
}

/**
 * Initialize the animation state.
 */
void MemCardModelPrivate::initAnimState(void)
{
	animTimer->stop();

	// TODO: Check for race conditions.
	qDeleteAll(animState);
	animState.clear();

	if (!card)
		return;

	// Initialize the animation state.
	for (int i = 0; i < fileCount; i++) {
		const File *file = card->getFile(i);
		initAnimState(file);
	}

	// Start the timer if animated icons are present.
	updateAnimTimerState();
}

/**
 * Initialize the animation state for a given file.
 * @param file File.
 */
void MemCardModelPrivate::initAnimState(const File *file)
{
	int numIcons = file->iconCount();
	if (numIcons <= 1) {
		// Not an animated icon.
		animState.remove(file);
		return;
	}

	IconAnimHelper *helper = new IconAnimHelper(file);
	animState.insert(file, helper);
}

/**
 * Update the animation timer state.
 * Starts the timer if animated icons are present; stops the timer if not.
 */
void MemCardModelPrivate::updateAnimTimerState(void)
{
	if (pauseCounter <= 0 && !animState.isEmpty()) {
		// Animation is not paused, and we have animated icons.
		// Start the timer.
		animTimer->start(IconAnimHelper::FAST_ANIM_TIMER);
	} else {
		// Either animation is paused, or we don't have animated icons.
		// Stop the timer.
		animTimer->stop();
	}
}

/** MemCardModel **/

MemCardModel::MemCardModel(QObject *parent)
	: super(parent)
	, d_ptr(new MemCardModelPrivate(this))
{
	// Connect the "themeChanged" signal.
	connect(qApp, SIGNAL(themeChanged()),
		this, SLOT(themeChanged_slot()));
}

MemCardModel::~MemCardModel()
{
	Q_D(MemCardModel);
	delete d;
}


int MemCardModel::rowCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent);
	Q_D(const MemCardModel);
	return (d->fileCount);
}

int MemCardModel::columnCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent);
	Q_D(const MemCardModel);
	if (d->card) {
		return COL_MAX;
	}
	return 0;
}

QVariant MemCardModel::data(const QModelIndex& index, int role) const
{
	Q_D(const MemCardModel);
	if (!d->card || !index.isValid())
		return QVariant();
	if (index.row() >= rowCount())
		return QVariant();

	// Get the memory card file.
	const File *file = d->card->getFile(index.row());

	// TODO: Move some of this to MemCardItemDelegate?
	switch (role) {
		case Qt::DisplayRole:
			switch (index.column()) {
				case COL_DESCRIPTION:
					return file->description();
				case COL_SIZE:
					return file->size();
				case COL_MTIME:
					return file->mtime();
				case COL_MODE:
					return file->modeAsString();
				case COL_GAMEID:
					return file->gameID();
				case COL_FILENAME:
					return file->filename();
				default:
					break;
			}
			break;

		case Qt::DecorationRole:
			// Images must use Qt::DecorationRole.
			switch (index.column()) {
				case COL_ICON:
					// Check if this is an animated icon.
					if (d->animState.contains(file)) {
						// Animated icon.
						IconAnimHelper *helper = d->animState.value(file);
						return helper->icon();
					} else {
						// Not an animated icon.
						// Return the first icon.
						return file->icon(0);
					}

				case COL_BANNER:
					return file->banner();

				case COL_ISVALID:
					switch (file->checksumStatus()) {
						default:
						case Checksum::CHKST_UNKNOWN:
							return d->style.getIcon(MemCardModelPrivate::style_t::ICON_UNKNOWN);
						case Checksum::CHKST_INVALID:
							return  d->style.getIcon(MemCardModelPrivate::style_t::ICON_INVALID);
						case Checksum::CHKST_GOOD:
							return  d->style.getIcon(MemCardModelPrivate::style_t::ICON_GOOD);
					}

				default:
					break;
			}
			break;

		case Qt::TextAlignmentRole:
			switch (index.column()) {
				case COL_SIZE:
				case COL_MODE:
				case COL_GAMEID:
				case COL_ISVALID:
					// These columns should be center-aligned horizontally.
					return (int)(Qt::AlignHCenter | Qt::AlignVCenter);

				default:
					// Everything should be center-aligned vertically.
					return Qt::AlignVCenter;
			}
			break;

		case Qt::FontRole:
			switch (index.column()) {
				case COL_SIZE:
				case COL_MODE:
				case COL_GAMEID: {
					// These columns should be monospaced.
					QFont fntMonospace(QLatin1String("Monospace"));
					fntMonospace.setStyleHint(QFont::TypeWriter);
					return fntMonospace;
				}

				default:
					break;
			}
			break;

		case Qt::BackgroundRole:
			// "Lost" files should be displayed using a different color.
			if (file->isLostFile()) {
				// TODO: Check if the item view is using alternating row colors before using them.
				if (index.row() & 1)
					return d->style.brush_lostFile_alt;
				else
					return d->style.brush_lostFile;
			}
			break;

		case Qt::SizeHintRole: {
			// Increase row height by 4px.
			// HACK: Increase icon/banner width on Windows.
			// Figure out a better method later.
			// TODO: Get correct icon size from the Card object.
		#ifdef Q_OS_WIN
			static const int iconWadj = 8;
			static const int bannerWadj = 8;
		#else
			static const int iconWadj = 0;
			static const int bannerWadj = 8;
		#endif
			switch (index.column()) {
				case COL_ICON:
					return QSize(CARD_ICON_W + iconWadj, (CARD_ICON_H + 4));
				case COL_BANNER:
					return QSize(CARD_BANNER_W + bannerWadj, (CARD_BANNER_H + 4));
				case COL_ISVALID: {
					// Using 32x32 icons.
					// (Hi-DPI is handled by Qt automatically.)
					return QSize(32, 32+4);
				}
				default:
					break;
			}
			break;
		}

		default:
			break;
	}

	// Default value.
	return QVariant();
}

QVariant MemCardModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	Q_UNUSED(orientation);

	switch (role) {
		case Qt::DisplayRole:
			switch (section) {
				case COL_ICON:		return tr("Icon");
				case COL_BANNER:	return tr("Banner");
				case COL_DESCRIPTION:	return tr("Description");
				case COL_SIZE:		return tr("Size");
				case COL_MTIME:		return tr("Last Modified");
				//: File permissions. (Known as "mode" on Unix systems.)
				case COL_MODE:		return tr("Mode");
				//: 6-digit game ID, e.g. GALE01.
				case COL_GAMEID:	return tr("Game ID");
				case COL_FILENAME:	return tr("Filename");

				// NOTE: Don't use a column header for COL_ISVALID.
				// Otherwise, the column will be too wide,
				// and the icon won't be aligned correctly.
				//case COL_ISVALID:	return tr("Valid?");

				default:
					break;
			}
			break;

		case Qt::TextAlignmentRole:
			switch (section) {
				case COL_ICON:
				case COL_SIZE:
				case COL_MODE:
				case COL_GAMEID:
				case COL_ISVALID:
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
void MemCardModel::setCard(Card *card)
{
	Q_D(MemCardModel);

	// Disconnect the Card's changed() signal if a Card is already set.
	if (d->card) {
		// Notify the view that we're about to remove all rows.
		// TODO: fileCount should already be cached...
		const int fileCount = d->card->fileCount();
		if (fileCount > 0)
			beginRemoveRows(QModelIndex(), 0, (fileCount - 1));

		// Disconnect the Card's signals.
		disconnect(d->card, &QObject::destroyed,
			   this, &MemCardModel::card_destroyed_slot);
		disconnect(d->card, &Card::filesAboutToBeInserted,
			   this, &MemCardModel::card_filesAboutToBeInserted_slot);
		disconnect(d->card, &Card::filesInserted,
			   this, &MemCardModel::card_filesInserted_slot);
		disconnect(d->card, &Card::filesAboutToBeRemoved,
			   this, &MemCardModel::card_filesAboutToBeRemoved_slot);
		disconnect(d->card, &Card::filesRemoved,
			   this, &MemCardModel::card_filesRemoved_slot);

		d->card = nullptr;

		// Done removing rows.
		d->fileCount = 0;
		if (fileCount > 0)
			endRemoveRows();
	}

	if (card) {
		// Notify the view that we're about to add rows.
		const int fileCount = card->fileCount();
		if (fileCount > 0)
			beginInsertRows(QModelIndex(), 0, (fileCount - 1));

		// Set the card.
		d->card = card;

		// Initialize the animation state.
		// NOTE: fileCount must be set here.
		d->fileCount = fileCount;
		d->initAnimState();

		// Connect the Card's signals.
		connect(d->card, &QObject::destroyed,
			this, &MemCardModel::card_destroyed_slot);
		connect(d->card, &Card::filesAboutToBeInserted,
			this, &MemCardModel::card_filesAboutToBeInserted_slot);
		connect(d->card, &Card::filesInserted,
			this, &MemCardModel::card_filesInserted_slot);
		connect(d->card, &Card::filesAboutToBeRemoved,
			this, &MemCardModel::card_filesAboutToBeRemoved_slot);
		connect(d->card, &Card::filesRemoved,
			this, &MemCardModel::card_filesRemoved_slot);

		// Done adding rows.
		if (fileCount > 0)
			endInsertRows();
	}
}

/** Public slots. **/

/**
 * Pause animation.
 * Should be used if e.g. the window is minimized.
 * NOTE: This uses an internal counter; the number of resumes
 * must match the number of pauses to resume animation.
 */
void MemCardModel::pauseAnimation(void)
{
	Q_D(MemCardModel);
	d->pauseCounter++;
	d->updateAnimTimerState();
}

/**
 * Resume animation.
 * Should be used if e.g. the window is un-minimized.
 * NOTE: This uses an internal counter; the number of resumes
 * must match the number of pauses to resume animation.
 */
void MemCardModel::resumeAnimation(void)
{
	Q_D(MemCardModel);
	if (d->pauseCounter > 0) {
		d->pauseCounter--;
	} else {
		// Not paused...
		d->pauseCounter = 0; // TODO: Probably not needed.
	}

	d->updateAnimTimerState();
}

/** Private slots. **/

/**
 * Animation timer slot.
 */
void MemCardModel::animTimerSlot(void)
{
	Q_D(MemCardModel);
	if (!d->card) {
		d->animTimer->stop();
		return;
	}

	// Check for icon animations.
	for (int i = 0; i < d->fileCount; i++) {
		const File *file = d->card->getFile(i);
		IconAnimHelper *helper = d->animState.value(file);
		if (!helper)
			continue;

		// Tell the IconAnimHelper that a timer tick has occurred.
		// TODO: Connect the timer to the IconAnimHelper directly?
		bool iconUpdated = helper->tick();
		if (iconUpdated) {
			// Icon has been updated.
			// Notify the UI that the icon has changed.
			QModelIndex iconIndex = createIndex(i, MemCardModel::COL_ICON);
			emit dataChanged(iconIndex, iconIndex);
		}
	}
}

/**
 * Card object was destroyed.
 * @param obj QObject that was destroyed.
 */
void MemCardModel::card_destroyed_slot(QObject *obj)
{
	Q_D(MemCardModel);

	if (obj == d->card) {
		// Our Card was destroyed.
		d->card = nullptr;
		int old_fileCount = d->fileCount;
		if (old_fileCount > 0)
			beginRemoveRows(QModelIndex(), 0, (old_fileCount - 1));
		d->fileCount = 0;
		if (old_fileCount > 0)
			endRemoveRows();
	}
}

/**
 * Files are about to be added to the Card.
 * @param start First file index.
 * @param end Last file index.
 */
void MemCardModel::card_filesAboutToBeInserted_slot(int start, int end)
{
	// Start adding rows.
	beginInsertRows(QModelIndex(), start, end);

	// Save the start/end indexes.
	Q_D(MemCardModel);
	d->insertStart = start;
	d->insertEnd = end;
}

/**
 * Files have been added to the Card.
 */
void MemCardModel::card_filesInserted_slot(void)
{
	Q_D(MemCardModel);

	// If these files have animated icons, add them.
	if (d->insertStart >= 0 && d->insertEnd >= 0) {
		for (int i = d->insertStart; i <= d->insertEnd; i++) {
			const File *file = d->card->getFile(i);
			d->initAnimState(file);
		}

		// Reset the row insert start/end indexes.
		d->insertStart = -1;
		d->insertEnd = -1;
	}

	// Update the animation timer state.
	d->updateAnimTimerState();

	// Update the file count.
	if (d->card)
		d->fileCount = d->card->fileCount();

	// Done adding rows.
	endInsertRows();
}

/**
 * Files are about to be removed from the Card.
 * @param start First file index.
 * @param end Last file index.
 */
void MemCardModel::card_filesAboutToBeRemoved_slot(int start, int end)
{
	// Start removing rows.
	beginRemoveRows(QModelIndex(), start, end);

	// Remove animation states for these files.
	Q_D(MemCardModel);
	for (int i = start; i <= end; i++) {
		const File *file = d->card->getFile(i);
		d->animState.remove(file);
	}
}

/**
 * Files have been removed from the Card.
 */
void MemCardModel::card_filesRemoved_slot(void)
{
	// Update the file count.
	Q_D(MemCardModel);
	if (d->card)
		d->fileCount = d->card->fileCount();

	// Done removing rows.
	endRemoveRows();
}

/** Slots. **/

/**
 * The system theme has changed.
 */
void MemCardModel::themeChanged_slot(void)
{
	// Reinitialize the style.
	Q_D(MemCardModel);
	d->style.init();

	// TODO: Force an update?
}
