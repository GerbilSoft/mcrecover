/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * XmlTemplateDialog.hpp: XML template dialog.                             *
 *                                                                         *
 * Copyright (c) 2014-2016 by David Korth.                                 *
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

class GcnFile;

class XmlTemplateDialogPrivate;
class XmlTemplateDialog : public QDialog
{
	Q_OBJECT
	typedef QDialog super;

	public:
		XmlTemplateDialog(QWidget *parent = nullptr);
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
		// State change event. (Used for switching the UI language at runtime.)
		virtual void changeEvent(QEvent *event) final;
};

#endif /* __MCRECOVER_XMLTEMPLATEDIALOG_HPP__ */
