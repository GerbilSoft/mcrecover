/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * CardView.hpp: Card view widget.                                         *
 *                                                                         *
 * Copyright (c) 2012-2025 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#ifndef __MCRECOVER_WIDGETS_CARDVIEW_HPP__
#define __MCRECOVER_WIDGETS_CARDVIEW_HPP__

#include <QWidget>

class Card;

class CardViewPrivate;
class CardView : public QWidget
{
	Q_OBJECT
	typedef QWidget super;

	// FIXME: Card* isn't working on Qt6 moc.
	//Q_PROPERTY(Card* card READ card WRITE setCard)

public:
	explicit CardView(QWidget *parent = 0);
	virtual ~CardView();

protected:
	CardViewPrivate *const d_ptr;
	Q_DECLARE_PRIVATE(CardView)
private:
	Q_DISABLE_COPY(CardView)

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
	// State change event (Used for switching the UI language at runtime.)
	void changeEvent(QEvent *event);

	// Paint event
	void paintEvent(QPaintEvent *event);

protected slots:
	/**
	 * Card object was destroyed.
	 * @param obj QObject that was destroyed
	 */
	void card_destroyed_slot(QObject *obj = 0);

	/**
	 * Card's block count has changed.
	 */
	void card_blockCountChanged_slot(void);

	/**
	 * Card's color has changed.
	 * @param color New color
	 */
	void card_colorChanged_slot(const QColor &color);
};

#endif /* __MCRECOVER_WIDGETS_CARDVIEW_HPP__ */
