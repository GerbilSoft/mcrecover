/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * FormatNewMemCardDialog.cpp: Format New Memory Card Image dialog.        *
 *                                                                         *
 * Copyright (c) 2015-2025 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#pragma once

#include <QDialog>

// MemCard classes.
class MemCardFile;

class FormatNewMemCardDialogPrivate;
class FormatNewMemCardDialog : public QDialog
{
	Q_OBJECT
	typedef QDialog super;

public:
	explicit FormatNewMemCardDialog(QWidget *parent = nullptr);
	virtual ~FormatNewMemCardDialog();
private:
	void init(void);

protected:
	FormatNewMemCardDialogPrivate *const d_ptr;
	Q_DECLARE_PRIVATE(FormatNewMemCardDialog)
private:
	Q_DISABLE_COPY(FormatNewMemCardDialog)

protected:
	// State change event. (Used for switching the UI language at runtime.)
	void changeEvent(QEvent *event) final;

private slots:
	void on_sldSize_sliderMoved(int value);
	void on_sldSize_valueChanged(int value);
};
