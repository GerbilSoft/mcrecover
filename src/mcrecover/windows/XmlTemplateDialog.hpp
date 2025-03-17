/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * XmlTemplateDialog.hpp: XML template dialog.                             *
 *                                                                         *
 * Copyright (c) 2014-2025 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#pragma once

#include <QDialog>

class GcnFile;

class XmlTemplateDialogPrivate;
class XmlTemplateDialog : public QDialog
{
	Q_OBJECT
	typedef QDialog super;

public:
	explicit XmlTemplateDialog(QWidget *parent = nullptr);
	XmlTemplateDialog(const GcnFile *file, QWidget *parent = nullptr);
	virtual ~XmlTemplateDialog();
private:
	void init(void);

protected:
	XmlTemplateDialogPrivate *const d_ptr;
	Q_DECLARE_PRIVATE(XmlTemplateDialog)
private:
	Q_DISABLE_COPY(XmlTemplateDialog)

protected:
	// State change event (Used for switching the UI language at runtime.)
	void changeEvent(QEvent *event) final;
};
