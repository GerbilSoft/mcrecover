/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * VmuFile.cpp: Dreamcast VMU file entry class.                            *
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

#ifndef __MCRECOVER_CARD_VMUFILE_HPP__
#define __MCRECOVER_CARD_VMUFILE_HPP__

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
		virtual QString modeAsString(void) const override;

		/** Lost File information **/

		/**
		 * Get the default export filename.
		 * @return Default export filename.
		 */
		virtual QString defaultExportFilename(void) const override;

		// TODO: Function to get format/extension of exported file.

		/**
		 * Export the file.
		 * @param filename Filename for the exported file.
		 * @return 0 on success; non-zero on error.
		 * TODO: Error code constants.
		 */
		virtual int exportToFile(const QString &filename) override;

		/**
		 * Export the file.
		 * @param qioDevice QIODevice to write the data to.
		 * @return 0 on success; non-zero on error.
		 * TODO: Error code constants.
		 */
		virtual int exportToFile(QIODevice *qioDevice) override;
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

#endif /* __MCRECOVER_CARD_VMUFILE_HPP__ */
