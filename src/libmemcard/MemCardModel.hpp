/***************************************************************************
 * GameCube Memory Card Recovery Program [libmemcard]                      *
 * MemCardModel.hpp: QAbstractListModel for Card.                          *
 *                                                                         *
 * Copyright (c) 2012-2018 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#ifndef __MCRECOVER_MEMCARDMODEL_HPP__
#define __MCRECOVER_MEMCARDMODEL_HPP__

class Card;

// Qt includes.
#include <QtCore/QAbstractListModel>

class MemCardModelPrivate;

class MemCardModel : public QAbstractListModel
{
	Q_OBJECT
	typedef QAbstractListModel super;
	
	public:
		explicit MemCardModel(QObject *parent = 0);
		virtual ~MemCardModel();

	protected:
		MemCardModelPrivate *const d_ptr;
		Q_DECLARE_PRIVATE(MemCardModel)
	private:
		Q_DISABLE_COPY(MemCardModel)

	public:
		enum Column {
			COL_ICON,		// Icon.
			COL_BANNER,		// Banner.
			COL_DESCRIPTION,	// Description. (both fields)
			COL_SIZE,		// Size (in blocks)
			COL_MTIME,		// Last modified time.
			COL_MODE,		// Mode. (permission, attributes)
			COL_GAMEID,		// Game ID.
			COL_FILENAME,		// Filename.
			COL_ISVALID,		// Is the file valid? (Checksum status)
			
			COL_MAX
		};

		// Qt Model/View interface.
		int rowCount(const QModelIndex& parent = QModelIndex()) const final;
		int columnCount(const QModelIndex& parent = QModelIndex()) const final;

		QVariant data(const QModelIndex& index, int role) const final;
		QVariant headerData(int section, Qt::Orientation orientation, int role) const final;

		/**
		 * Set the memory card to use in this model.
		 * @param card Memory card.
		 */
		void setCard(Card *card);

	public slots:
		/**
		 * Pause animation.
		 * Should be used if e.g. the window is minimized.
		 * NOTE: This uses an internal counter; the number of resumes
		 * must match the number of pauses to resume animation.
		 */
		void pauseAnimation(void);

		/**
		 * Resume animation.
		 * Should be used if e.g. the window is un-minimized.
		 * NOTE: This uses an internal counter; the number of resumes
		 * must match the number of pauses to resume animation.
		 */
		void resumeAnimation(void);

	private slots:
		/**
		 * Animation timer slot.
		 * Wrapper for MemCardModelPrivate::animTimerSlot().
		 */
		void animTimerSlot(void);

		/**
		 * Card object was destroyed.
		 * @param obj QObject that was destroyed.
		 */
		void card_destroyed_slot(QObject *obj = 0);

		/**
		 * Files are about to be added to the Card.
		 * @param start First file index.
		 * @param end Last file index.
		 */
		void card_filesAboutToBeInserted_slot(int start, int end);

		/**
		 * Files have been added to the Card.
		 */
		void card_filesInserted_slot(void);

		/**
		 * Files are about to be removed from the Card.
		 * @param start First file index.
		 * @param end Last file index.
		 */
		void card_filesAboutToBeRemoved_slot(int start, int end);

		/**
		 * Files have been removed from the Card.
		 */
		void card_filesRemoved_slot(void);

		/**
		 * The system theme has changed.
		 */
		void themeChanged_slot(void);
};

#endif /* __MCRECOVER_MEMCARDMODEL_HPP__ */
