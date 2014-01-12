/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * XmlTemplateDialog.hpp: XML template dialog.                             *
 *                                                                         *
 * Copyright (c) 2014 by David Korth.                                      *
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

#ifndef __MCRECOVER_XMLTEMPLATEDIALOG_HPP__
#define __MCRECOVER_XMLTEMPLATEDIALOG_HPP__

#include <QtGui/QDialog>
#include "ui_XmlTemplateDialog.h"

// MemCard classes.
class MemCardFile;

class XmlTemplateDialogPrivate;

class XmlTemplateDialog : public QDialog, public Ui::XmlTemplateDialog
{
	Q_OBJECT

	public:
		XmlTemplateDialog(QWidget *parent = nullptr);
		XmlTemplateDialog(const MemCardFile *file, QWidget *parent = nullptr);
		~XmlTemplateDialog();
	private:
		void init(void);

	protected:
		XmlTemplateDialogPrivate *const d_ptr;
		Q_DECLARE_PRIVATE(XmlTemplateDialog)
	private:
		Q_DISABLE_COPY(XmlTemplateDialog)

	protected:
		// State change event. (Used for switching the UI language at runtime.)
		void changeEvent(QEvent *event);
};

#endif /* __MCRECOVER_XMLTEMPLATEDIALOG_HPP__ */
