/***************************************************************************
 * GameCube Memory Card Recovery Program [libmemcard]                      *
 * VmuFile.cpp: Dreamcast VMU file entry class.                            *
 *                                                                         *
 * Copyright (c) 2015-2018 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#ifndef __LIBMEMCARD_VMUFILE_HPP__
#define __LIBMEMCARD_VMUFILE_HPP__

#include "File.hpp"
// TODO: Remove vmu.h from here?
#include "vmu.h"

// Qt includes.
class QIODevice;

// Checksum algorithm class.
#include "Checksum.hpp"

class VmuCard;
class GcImage;

class VmuFilePrivate;
class VmuFile : public File
{
	Q_OBJECT
	typedef File super;

	public:
		/**
		 * Create a VmuFile for a VmuCard.
		 * This constructor is for valid files.
		 * @param card VmuCard.
		 * @param direntry Directory Entry pointer.
		 * @param mc_fat VMU FAT.
		 */
		VmuFile(VmuCard *card,
			const vmu_dir_entry *dirEntry,
			const vmu_fat *mc_fat);

		virtual ~VmuFile();

	protected:
		Q_DECLARE_PRIVATE(VmuFile)
	private:
		Q_DISABLE_COPY(VmuFile)

	public:
		/** TODO: Move encoding to File. **/

		/**
		 * Get the file's mode as a string.
		 * This is system-specific.
		 * @return File mode as a string.
		 */
		QString modeAsString(void) const final;

		/** Lost File information **/

		/**
		 * Get the default export filename.
		 * @return Default export filename.
		 */
		QString defaultExportFilename(void) const final;

		// TODO: Function to get format/extension of exported file.

		/**
		 * Export the file.
		 * @param filename Filename for the exported file.
		 * @return 0 on success; non-zero on error.
		 * TODO: Error code constants.
		 */
		int exportToFile(const QString &filename) final;

		/**
		 * Export the file.
		 * @param qioDevice QIODevice to write the data to.
		 * @return 0 on success; non-zero on error.
		 * TODO: Error code constants.
		 */
		int exportToFile(QIODevice *qioDevice) final;
		// TODO: Move these down to File.

	public:
		/** DC-specific functions. **/

		/**
		 * Get the monochrome ICONDATA_VMS icon.
		 * This is only valid for ICONDATA_VMS files.
		 * @return Monochrome ICONDATA_VMS icon, or nullptr if not found.
		 */
		const GcImage *vmu_icondata_mono(void) const;

		/**
		 * Get the color ICONDATA_VMS icon.
		 * This is only valid for ICONDATA_VMS files.
		 * @return Color ICONDATA_VMS icon, or nullptr if not found.
		 */
		const GcImage *vmu_icondata_color(void) const;
};

#endif /* __LIBMEMCARD_VMUFILE_HPP__ */
