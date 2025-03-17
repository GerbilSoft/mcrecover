/***************************************************************************
 * GameCube Memory Card Recovery Program [libsaveedit]                     *
 * ByteFlagsView.hpp: Byte Flags editor.                                   *
 *                                                                         *
 * Copyright (c) 2015-2025 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#pragma once

#include <QWidget>

class ByteFlagsModel;

class ByteFlagsViewPrivate;
class ByteFlagsView : public QWidget
{
	Q_OBJECT

	Q_PROPERTY(ByteFlagsModel* byteFlagsModel READ byteFlagsModel WRITE setByteFlagsModel)
	Q_PROPERTY(int pageSize READ pageSize)

public:
	explicit ByteFlagsView(QWidget *parent = 0);
	~ByteFlagsView();

private:
	typedef QWidget super;
	ByteFlagsViewPrivate *const d_ptr;
	Q_DECLARE_PRIVATE(ByteFlagsView)
	Q_DISABLE_COPY(ByteFlagsView)

protected:
	// State change event (Used for switching the UI language at runtime.)
	void changeEvent(QEvent *event) final;

public:
	/** Model access **/

	/**
	 * Get the ByteFlagsModel this widget is editing.
	 * @return ByteFlagsModel
	 */
	ByteFlagsModel *byteFlagsModel(void) const;

	/**
	 * Set the ByteFlagsModel to edit.
	 * @param byteFlagsModel ByteFlagsModel
	 */
	void setByteFlagsModel(ByteFlagsModel *model);

	/** Data access **/

	/**
	 * Get the page size.
	 * @return Page size
	 */
	int pageSize(void) const;

	// TODO: Page count?
};
