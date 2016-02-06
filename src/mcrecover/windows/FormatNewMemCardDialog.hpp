/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * FormatNewMemCardDialog.cpp: Format New Memory Card Image dialog.        *
 *                                                                         *
 * Copyright (c) 2015-2016 by David Korth.                                 *
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

#ifndef __MCRECOVER_FORMATNEWMEMCARDDIALOG_HPP__
#define __MCRECOVER_FORMATNEWMEMCARDDIALOG_HPP__

#include <QtGui/QDialog>

// MemCard classes.
class MemCardFile;

class FormatNewMemCardDialogPrivate;
class FormatNewMemCardDialog : public QDialog
{
	Q_OBJECT
	typedef QDialog super;

	public:
		FormatNewMemCardDialog(QWidget *parent = nullptr);
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
		virtual void changeEvent(QEvent *event) final;

	private slots:
		void on_sldSize_sliderMoved(int value);
		void on_sldSize_valueChanged(int value);
};

#endif /* __MCRECOVER_FORMATNEWMEMCARDDIALOG_HPP__ */
