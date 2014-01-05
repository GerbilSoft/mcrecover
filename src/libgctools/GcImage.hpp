/***************************************************************************
 * GameCube Tools Library.                                                 *
 * GcImage.hpp: GameCube image format handler.                             *
 *                                                                         *
 * Copyright (c) 2012-2013 by David Korth.                                 *
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

class GcImagePrivate;

class GcImage
{
	private:
		GcImage();
	public:
		~GcImage();

	private:
		friend class GcImagePrivate;
		GcImagePrivate *const d;
		// TODO: Copy Qt's Q_DISABLE_COPY() macro.
		GcImage(const GcImage &);
		GcImage &operator=(const GcImage &);

	public:
		/**
		 * Convert a GameCube CI8 image to GcImage.
		 * @param w Image width.
		 * @param h Image height.
		 * @param img_buf CI8 image buffer.
		 * @param img_siz Size of image data. [must be >= (w*h)]
		 * @param pal_buf Palette buffer.
		 * @param pal_siz Size of palette data. [must be >= 0x200]
		 * @return GcImage, or nullptr on error.
		 */
		static GcImage *fromCI8(int w, int h,
					const uint8_t *img_buf, int img_siz,
					const uint16_t *pal_buf, int pal_siz);

		/**
		 * Convert a GameCube RGB5A3 image to GcImage.
		 * @param w Image width.
		 * @param h Image height.
		 * @param img_buf CI8 image buffer.
		 * @param img_siz Size of image data. [must be >= (w*h)*2]
		 * @return GcImage, or nullptr on error.
		 */
		static GcImage *fromRGB5A3(int w, int h, const uint16_t *img_buf, int img_siz);

		/**
		 * Internal pixel formats.
		 */
		enum PxFmt {
			// No image.
			PXFMT_NONE	= 0,

			// 256-color image.
			PXFMT_CI8	= 1,

			// 32-bit ARGB.
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
