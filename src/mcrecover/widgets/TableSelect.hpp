/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * TableSelect.hpp: Directory/Block Table select widget.                   *
 *                                                                         *
 * Copyright (c) 2014-2025 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#pragma once

#include <QtWidgets/QWidget>

#include "Card.hpp"

class TableSelectPrivate;
class TableSelect : public QWidget
{
	Q_OBJECT
	typedef QWidget super;

	Q_PROPERTY(Card* card READ card WRITE setCard)
	Q_PROPERTY(int activeDatIdx READ activeDatIdx WRITE setActiveDatIdx)
	Q_PROPERTY(int activeBatIdx READ activeBatIdx WRITE setActiveBatIdx)

public:
	explicit TableSelect(QWidget *parent = 0);
	virtual ~TableSelect();

protected:
	TableSelectPrivate *const d_ptr;
	Q_DECLARE_PRIVATE(TableSelect)
private:
	Q_DISABLE_COPY(TableSelect)

public:
	/**
	 * Get the Card being displayed.
	 * @return Card
	 */
	Card *card(void) const;

	/**
	 * Set the Card being displayed.
	 * @param file Card
	 */
	void setCard(Card *card);

protected:
	/** Events **/

	// State change event. (Used for switching the UI language at runtime.)
	void changeEvent(QEvent *event);

public:
	/** Properties **/

	/**
	 * Get the selected directory table.
	 * @return Selected directory table index, or -1 on error.
	 */
	int activeDatIdx(void) const;

	/**
	 * Get the selected block table.
	 * @return Selected block table index, or -1 on error.
	 */
	int activeBatIdx(void) const;

protected slots:
	/** Internal slots **/

	/**
	 * Card object was destroyed.
	 * @param obj QObject that was destroyed
	 */
	void memCard_destroyed_slot(QObject *obj = 0);

	/**
	 * Card's active Directory Table index was changed.
	 * @param idx New active Directory Table index
	 */
	void memCard_activeDatIdxChanged_slot(int idx);

	/**
	 * Card's active Block Table index was changed.
	 * @param idx New active Block Table index
	 */
	void memCard_activeBatIdxChanged_slot(int idx);

public slots:
	/** Public slots **/

	/**
	 * Set the active Directory Table index.
	 * NOTE: This function reloads the file list, without lost files.
	 * @param idx Active Directory Table index (0 or 1)
	 */
	void setActiveDatIdx(int idx);

	/**
	 * Set the active Block Table index.
	 * NOTE: This function reloads the file list, without lost files.
	 * @param idx Active Block Table index (0 or 1)
	 */
	void setActiveBatIdx(int idx);
};
