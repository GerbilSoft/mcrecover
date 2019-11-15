/***************************************************************************
 * GameCube Tools Library.                                                 *
 * DcImageLoader.hpp: Dreamcast image loader.                              *
 * Converts Dreamcast images from native formats to GcImage.               *
 *                                                                         *
 * Copyright (c) 2012-2015 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#ifndef __LIBGCTOOLS_DCIMAGELOADER_HPP__
#define __LIBGCTOOLS_DCIMAGELOADER_HPP__

// C includes.
#include <stdint.h>

#include "GcImage.hpp"
class DcImageLoader
{
	private:
		DcImageLoader();
		~DcImageLoader();
		DcImageLoader(const DcImageLoader &other);
		DcImageLoader &operator=(const DcImageLoader &other);

	public:
		/**
		 * Convert a Dreamcast 16-color image to GcImage.
		 * @param w Image width.
		 * @param h Image height.
		 * @param img_buf 16-color image buffer.
		 * @param img_siz Size of image data. [must be >= (w*h)/2]
		 * @param pal_buf Palette buffer. (ARGB4444)
		 * @param pal_siz Size of palette buffer. [must be >= 0x20]
		 * @return GcImage, or nullptr on error.
		 */
		static GcImage *fromPalette16(int w, int h,
					const uint8_t *img_buf, int img_siz,
					const uint16_t *pal_buf, int pal_siz);

		/**
		 * Convert a Dreamcast ARGB4444 image to GcImage.
		 * @param w Image width.
		 * @param h Image height.
		 * @param img_buf ARGB4444 image buffer.
		 * @param img_siz Size of image data. [must be >= (w*h)*2]
		 * @return GcImage, or nullptr on error.
		 */
		static GcImage *fromARGB4444(int w, int h, const uint16_t *img_buf, int img_siz);

		/**
		 * Convert a Dreamcast monochrome image to GcImage.
		 * TODO: Optional two-color palette?
		 * @param w Image width. (must be a multiple of 8)
		 * @param h Image height.
		 * @param img_buf Monochrome image buffer.
		 * @param img_siz Size of image data. [must be >= (w*h)/8]
		 * @return GcImage, or nullptr on error.
		 */
		static GcImage *fromMonochrome(int w, int h, const uint8_t *img_buf, int img_siz);
};

#endif /* __LIBGCTOOLS_GCIMAGE_HPP__ */
