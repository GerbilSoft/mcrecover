/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * AboutDialog.hpp: About dialog.                                          *
 *                                                                         *
 * Copyright (c) 2013 by David Korth.                                      *
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

#ifndef __MCRECOVER_ABOUTDIALOG_HPP__
#define __MCRECOVER_ABOUTDIALOG_HPP__

#include <QtGui/QDialog>
#include "ui_AboutDialog.h"

// Qt classes.
class QWidget;

class AboutDialogPrivate;

class AboutDialog : public QDialog, public Ui::AboutDialog
{
	Q_OBJECT
	
	public:
		static void ShowSingle(QWidget *parent = NULL);

	protected:
		AboutDialog(QWidget *parent = NULL);
		~AboutDialog();

	private:
		friend class AboutDialogPrivate;
		AboutDialogPrivate *const d;
		Q_DISABLE_COPY(AboutDialog);

	protected:
		// State change event. (Used for switching the UI language at runtime.)
		void changeEvent(QEvent *event);
};

#endif /* __MCRECOVER_ABOUTDIALOG_HPP__ */
