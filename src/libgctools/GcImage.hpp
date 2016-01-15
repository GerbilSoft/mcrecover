/***************************************************************************
 * GameCube Tools Library.                                                 *
 * GcImage.hpp: GameCube image format handler.                             *
 *                                                                         *
 * Copyright (c) 2012-2015 by David Korth.                                 *
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

#ifndef __LIBGCTOOLS_GCIMAGE_HPP__
#define __LIBGCTOOLS_GCIMAGE_HPP__

// C includes.
#include <stdint.h>
#include <string.h>

// GCN card definitions.
#include "card.h"

// Loader classes.
class GcImageLoader;
class DcImageLoader;

class GcImagePrivate;
class GcImage
{
	private:
		GcImage();
	public:
		~GcImage();
		// Copy constructor.
		GcImage(const GcImage &other);

	private:
		friend class GcImagePrivate;
		GcImagePrivate *const d;
		// Assign constructor. (TODO)
		GcImage &operator=(const GcImage &other);

		// Image loader classes.
		friend class GcImageLoader;
		friend class DcImageLoader;

	public:
		/**
		 * Convert this GcImage to ARGB32.
		 * Caller must delete the returned GcImage.
		 * @return New GcImage in ARGB32.
		 */
		GcImage *toRGB5A3(void) const;

		/**
		 * Internal pixel formats.
		 */
		enum PxFmt {
			// No image.
			PXFMT_NONE	= 0,

			// 256-color image.
			PXFMT_CI8	= 1,

			// 32-bit ARGB. (converted from RGB5A3)
			PXFMT_ARGB32	= 2,
		};

		/**
		 * Get the pixel format of the image.
		 * @return Pixel format.
		 */
		PxFmt pxFmt(void) const;

		/**
		 * Get the image data.
		 * @return Image data.
		 */
		const void *imageData(void) const;

		/**
		 * Get the image data length.
		 * @return Image data length.
		 */
		size_t imageData_len(void) const;

		/**
		 * Get the image palette.
		 * @return Pointer to 256-element ARGB888 palette, or NULL if no palette.
		 */
		const uint32_t *palette(void) const;

		/**
		 * Get the image width, in pixels.
		 * @return Image width.
		 */
		int width(void) const;

		/**
		 * Get the image height, in pixels.
		 * @return Image height.
		 */
		int height(void) const;
};

#endif /* __LIBGCTOOLS_GCIMAGE_HPP__ */
