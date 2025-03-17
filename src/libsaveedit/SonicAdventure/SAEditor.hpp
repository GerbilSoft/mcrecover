/***************************************************************************
 * GameCube Memory Card Recovery Program [libsaveedit]                     *
 * SAEditor.hpp: Sonic Adventure - save file editor.                       *
 *                                                                         *
 * Copyright (c) 2015-2025 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#pragma once

#include "../EditorWidget.hpp"

class File;

class SAEditorPrivate;
class SAEditor : public EditorWidget
{
	Q_OBJECT

public:
	explicit SAEditor(QWidget *parent = nullptr);

private:
	typedef EditorWidget super;
	Q_DECLARE_PRIVATE(SAEditor)
	Q_DISABLE_COPY(SAEditor)

protected:
	// State change event (Used for switching the UI language at runtime.)
	void changeEvent(QEvent *event);

public:
	/** Public static functions **/

	/**
	 * Is the specified file supported by this editor?
	 * @return True if supported; false if not.
	 */
	static bool isFileSupported(const File *file);

public:
	/** Public functions **/

	/**
	 * Set the File to edit.
	 *
	 * @param file File to edit
	 * If the file isn't valid, it won't be set;
	 * check file() afterwards to verify.
	 *
	 * @return 0 on success; non-zero on error (and file will not be set).
	 * TODO: Error code constants?
	 */
	int setFile(File *file) final;

public slots:
	/**
	 * Save the data to the file.
	 * @return 0 on success; negative POSIX error code on error.
	 */
	int save(void) final;

	/**
	 * Reload the save data.
	 */
	void reload(void) final;

protected:
	/**
	 * Set the current save slot. [INTERNAL FUNCTION]
	 *
	 * This is called by the base class when the
	 * setCurrentSaveSlot() function is called.
	 * Subclasses should load the appropriate data
	 * from the specified save slot.
	 *
	 * If an error occurs, the save slot will not be changed.
	 *
	 * @return 0 on success; non-zero on error.
	 */
	int setCurrentSaveSlot_int(int saveSlot) final;

protected slots:
	/**
	 * Widget's modified state has been changed.
	 * @param modified New modified status
	 */
	void widgetHasBeenModified(bool modified);
};
