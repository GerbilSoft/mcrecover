/***************************************************************************
 * GameCube Memory Card Recovery Program [libmemcard]                      *
 * IconAnimHelper.hpp: Icon animation helper.                              *
 *                                                                         *
 * Copyright (c) 2012-2025 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#pragma once

// Qt includes.
#include <QtCore/QObject>
#include <QtGui/QPixmap>

class File;

class IconAnimHelperPrivate;

class IconAnimHelper : public QObject
{
	Q_OBJECT
	typedef QObject super;

	Q_PROPERTY(const File* file READ file WRITE setFile)
	Q_PROPERTY(bool animated READ isAnimated STORED false)
	Q_PROPERTY(QPixmap icon READ icon STORED false)

	public:
		IconAnimHelper();
		explicit IconAnimHelper(const File *file);
		virtual ~IconAnimHelper();

	protected:
		IconAnimHelperPrivate *const d_ptr;
		Q_DECLARE_PRIVATE(IconAnimHelper)
	private:
		Q_DISABLE_COPY(IconAnimHelper)

	public:
		/**
		 * Get the File this IconAnimHelper is handling.
		 * @return File.
		 */
		const File *file(void) const;

		/**
		 * Set the File this IconAnimHelper should handle.
		 * @param file File.
		 */
		void setFile(const File *file);

		/**
		 * Reset the animation state.
		 */
		void reset(void);

		/**
		 * Does this file have an animated icon?
		 * @return True if the icon is animated; false if not, or if no file is loaded.
		 */
		bool isAnimated(void) const;

		/**
		 * Get the current icon for this file.
		 * @return Current icon.
		 */
		QPixmap icon(void) const;

		/**
		 * Timer tick for the animation counter.
		 * @return True if the current icon has been changed; false if not.
		 */
		bool tick(void);

	protected slots:
		/**
		 * File object was destroyed.
		 * @param obj QObject that was destroyed.
		 */
		void file_destroyed_slot(QObject *obj = 0);
};
