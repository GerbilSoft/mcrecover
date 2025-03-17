/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * FileView.hpp: File view widget.                                         *
 *                                                                         *
 * Copyright (c) 2012-2025 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#pragma once

#include <QWidget>

class File;
Q_DECLARE_OPAQUE_POINTER(const File*)
Q_DECLARE_METATYPE(const File*)

class FileViewPrivate;
class FileView : public QWidget
{
	Q_OBJECT
	typedef QWidget super;

	Q_PROPERTY(const File* file READ file WRITE setFile)

public:
	explicit FileView(QWidget *parent = 0);
	virtual ~FileView();

protected:
	FileViewPrivate *const d_ptr;
	Q_DECLARE_PRIVATE(FileView)
private:
	Q_DISABLE_COPY(FileView)

public:
	/**
	 * Get the File being displayed.
	 * @return File
	 */
	const File *file(void) const;

	/**
	 * Set the File being displayed.
	 * @param file File
	 */
	void setFile(const File *file);

protected:
	// State change event (Used for switching the UI language at runtime.)
	void changeEvent(QEvent *event);

public slots:
	/**
	 * Pause animation.
	 * Should be used if e.g. the window is minimized.
	 * NOTE: This uses an internal counter; the number of resumes
	 * must match the number of pauses to resume animation.
	 */
	void pauseAnimation(void);

	/**
	 * Resume animation.
	 * Should be used if e.g. the window is un-minimized.
	 * NOTE: This uses an internal counter; the number of resumes
	 * must match the number of pauses to resume animation.
	 */
	void resumeAnimation(void);

protected slots:
	/**
	 * File object was destroyed.
	 * @param obj QObject that was destroyed
	 */
	void file_destroyed_slot(QObject *obj = 0);

	/**
	 * Animation timer slot.
	 */
	void animTimer_slot(void);

	/**
	 * XML button was pressed.
	 */
	void on_btnXML_clicked(void);

	/**
	 * Edit button was pressed.
	 */
	void on_btnEdit_clicked(void);
};
