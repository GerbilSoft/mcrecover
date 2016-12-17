/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * AboutDialog.hpp: About Dialog.                                          *
 *                                                                         *
 * Copyright (c) 2013-2016 by David Korth.                                 *
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

#include <QDialog>

// Qt classes.
class QWidget;

class AboutDialogPrivate;
class AboutDialog : public QDialog
{
	Q_OBJECT
	typedef QDialog super;
	
	public:
		static void ShowSingle(QWidget *parent = nullptr);

	protected:
		explicit AboutDialog(QWidget *parent = nullptr);
		virtual ~AboutDialog();

	protected:
		AboutDialogPrivate *const d_ptr;
		Q_DECLARE_PRIVATE(AboutDialog)
	private:
		Q_DISABLE_COPY(AboutDialog)

	protected:
		// State change event. (Used for switching the UI language at runtime.)
		virtual void changeEvent(QEvent *event) final;
};

#endif /* __MCRECOVER_ABOUTDIALOG_HPP__ */
