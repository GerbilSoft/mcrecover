/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * XmlTemplateDialogManager.hpp: XmlTemplateDialog Manager.                *
 *                                                                         *
 * Copyright (c) 2014-2015 by David Korth.                                 *
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

#ifndef __MCRECOVER_XMLTEMPLATEDIALOGMANAGER_HPP__
#define __MCRECOVER_XMLTEMPLATEDIALOGMANAGER_HPP__

// Qt includes.
#include <QtCore/QObject>

class GcnFile;
class XmlTemplateDialog;

class XmlTemplateDialogManagerPrivate;
class XmlTemplateDialogManager : public QObject
{
	Q_OBJECT

	public:
		XmlTemplateDialogManager(QObject *parent = nullptr);
		~XmlTemplateDialogManager();

	protected:
		XmlTemplateDialogManagerPrivate *const d_ptr;
		Q_DECLARE_PRIVATE(XmlTemplateDialogManager)
	private:
		Q_DISABLE_COPY(XmlTemplateDialogManager)

	public:
		/**
		 * Create an XmlTemplateDialog for a given GcnFile.
		 * If a dialog exists for that GcnFile, the existing
		 * dialog will be used.
		 * @param file GcnFile.
		 * @param parent Parent.
		 * @return XmlTemplateDialog;
		 */
		XmlTemplateDialog *create(const GcnFile *file, QWidget *parent = nullptr);

	protected slots:
		/**
		 * A GcnFile has been destroyed.
		 * @param obj GcnFile that was destroyed.
		 */
		void file_destroyed_slot(QObject *obj);

		/**
		 * An XmlTemplateDialog has been destroyed.
		 * @param obj XmlTemplateDialog that was destroyed.
		 */
		void xmlTemplateDialog_destroyed_slot(QObject *obj);
};

#endif /* __MCRECOVER_XMLTEMPLATEDIALOGMANAGER_HPP__ */
