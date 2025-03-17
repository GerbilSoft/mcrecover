/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * XmlTemplateDialogManager.cpp: XmlTemplateDialog Manager.                *
 *                                                                         *
 * Copyright (c) 2014-2025 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#include "XmlTemplateDialogManager.hpp"

// Classes this class manages.
#include "libmemcard/GcnFile.hpp"
#include "XmlTemplateDialog.hpp"

// Qt includes.
#include <QtCore/QHash>

/** XmlTemplateDialogManagerPrivate **/

class XmlTemplateDialogManagerPrivate
{
	public:
		explicit XmlTemplateDialogManagerPrivate(XmlTemplateDialogManager *q);
		~XmlTemplateDialogManagerPrivate();

	protected:
		XmlTemplateDialogManager *const q_ptr;
		Q_DECLARE_PUBLIC(XmlTemplateDialogManager)
	private:
		Q_DISABLE_COPY(XmlTemplateDialogManagerPrivate)

	public:
		/**
		 * Map GcnFiles to XmlTemplateDialogs and vice-versa.
		 */
		QHash<const GcnFile*, XmlTemplateDialog*> dialogHash;
		QHash<XmlTemplateDialog*, const GcnFile*> dialogHashRev;
};

XmlTemplateDialogManagerPrivate::XmlTemplateDialogManagerPrivate(XmlTemplateDialogManager* q)
	: q_ptr(q)
{ }

XmlTemplateDialogManagerPrivate::~XmlTemplateDialogManagerPrivate()
{
	// Clear the reverse hash to prevent recursive deletion.
	dialogHashRev.clear();

	// Delete all dialogs.
	qDeleteAll(dialogHash);
	dialogHash.clear();
}

/** XmlTemplateDialogManager **/

/**
 * Initialize the XmlTemplateDialog Manager.
 * @param parent Parent object.
 */
XmlTemplateDialogManager::XmlTemplateDialogManager(QObject *parent)
	: super(parent)
	, d_ptr(new XmlTemplateDialogManagerPrivate(this))
{ }

/**
 * Shut down the XmlTemplateDialog Manager.
 */
XmlTemplateDialogManager::~XmlTemplateDialogManager()
{
	delete d_ptr;
}

/**
 * Create an XmlTemplateDialog for a given GcnFile.
 * If a dialog exists for that GcnFile, the existing
 * dialog will be used.
 * @param file GcnFile
 * @param parent Parent
 * @return XmlTemplateDialog
 */
XmlTemplateDialog *XmlTemplateDialogManager::create(const GcnFile *file, QWidget *parent)
{
	Q_D(XmlTemplateDialogManager);
	XmlTemplateDialog *dialog = d->dialogHash.value(file);
	if (dialog) {
		// Dialog already exists.
		// Change its parent.
		dialog->setParent(parent);
	} else {
		// Dialog does not exist. Create it.
		dialog = new XmlTemplateDialog(file, parent);
		d->dialogHash.insert(file, dialog);
		d->dialogHashRev.insert(dialog, file);

		// Make sure we know if either the file or the dialog are destroyed.
		connect(file, &QObject::destroyed,
			this, &XmlTemplateDialogManager::file_destroyed_slot);
		connect(dialog, &QObject::destroyed,
			this, &XmlTemplateDialogManager::xmlTemplateDialog_destroyed_slot);
	}

	return dialog;
}

/**
 * A GcnFile has been destroyed.
 * @param obj GcnFile that was destroyed
 */
void XmlTemplateDialogManager::file_destroyed_slot(QObject *obj)
{
	// NOTE: Can't use qobject_cast<> here because the object
	// is probably destroyed at this point.
	const GcnFile *file = static_cast<const GcnFile*>(obj);

	// GcnFile was destroyed.
	Q_D(XmlTemplateDialogManager);
	XmlTemplateDialog *dialog = d->dialogHash.value(file);
	d->dialogHash.remove(file);
	if (dialog) {
		d->dialogHashRev.remove(dialog);
		delete dialog;
	}
}

/**
 * An XmlTemplateDialog has been destroyed.
 * @param obj XmlTemplateDialog that was destroyed
 */
void XmlTemplateDialogManager::xmlTemplateDialog_destroyed_slot(QObject *obj)
{
	// NOTE: Can't use qobject_cast<> here because the object
	// is probably destroyed at this point.
	XmlTemplateDialog *dialog = static_cast<XmlTemplateDialog*>(obj);

	// XmlTemplateDialog was destroyed.
	Q_D(XmlTemplateDialogManager);
	const GcnFile *file = d->dialogHashRev.value(dialog);
	d->dialogHashRev.remove(dialog);
	if (file) {
		// We don't own the GcnFile, so don't delete it.
		disconnect(file, &QObject::destroyed,
			   this, &XmlTemplateDialogManager::file_destroyed_slot);
		d->dialogHash.remove(file);
	}
}
