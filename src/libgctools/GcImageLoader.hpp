/***************************************************************************
 * GameCube Tools Library.                                                 *
 * GcImageLoader.hpp: GameCube image loader.                               *
 * Converts GameCube images from native formats to GcImage.                *
 *                                                                         *
 * Copyright (c) 2012-2015 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#ifndef __LIBGCTOOLS_GCIMAGELOADER_HPP__
#define __LIBGCTOOLS_GCIMAGELOADER_HPP__

// C includes.
#include <stdint.h>

#include "GcImage.hpp"
class GcImageLoader
{
	private:
		GcImageLoader();
		~GcImageLoader();
		GcImageLoader(const GcImageLoader &other);
		GcImageLoader &operator=(const GcImageLoader &other);

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
};

#endif /* __LIBGCTOOLS_GCIMAGELOADER_HPP__ */
