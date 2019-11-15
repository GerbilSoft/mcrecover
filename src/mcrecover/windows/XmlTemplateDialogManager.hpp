/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * XmlTemplateDialogManager.hpp: XmlTemplateDialog Manager.                *
 *                                                                         *
 * Copyright (c) 2014-2016 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
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
	typedef QObject super;

	public:
		explicit XmlTemplateDialogManager(QObject *parent = nullptr);
		virtual ~XmlTemplateDialogManager();

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
