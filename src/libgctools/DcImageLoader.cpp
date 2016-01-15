/***************************************************************************
 * GameCube Tools Library.                                                 *
 * DcImageLoader.cpp: Dreamcast image loader.                              *
 * Converts Dreamcast images from native formats to GcImage.               *
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

#include "DcImageLoader.hpp"
#include "GcImage_p.hpp"

// Byteswapping macros.
#include "util/byteswap.h"

// C includes. (C++ namespace)
#include <cstring>
/**
 * Convert an ARGB4444 pixel to ARGB32.
 * @param px16 RGB5A3 pixel.
 * @return ARGB32 pixel.
 */
static inline uint32_t ARGB4444_to_ARGB32(uint16_t px16)
{
	uint32_t px32 = 0;
	px32 |= (((px16 & 0x000F) << 4)  |  (px16 & 0x000F));		// B
	px32 |= (((px16 & 0x00F0) << 4)  | ((px16 & 0x00F0) << 8));	// G
	px32 |= (((px16 & 0x0F00) << 8)  | ((px16 & 0x0F00) << 12));	// R
	px32 |= (((px16 & 0xF000) << 12) | ((px16 & 0xF000) << 16));	// A
	return px32;
}

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
GcImage *DcImageLoader::fromPalette16(int w, int h,
			const uint8_t *img_buf, int img_siz,
			const uint16_t *pal_buf, int pal_siz)
{
	// Verify parameters.
	if (w < 0 || h < 0)
		return nullptr;
	if (img_siz < ((w * h) / 2) || pal_siz < 0x20)
		return nullptr;

	// Create a GcImage.
	// TODO: Make an internal 16-color format?
	GcImage *gcImage = new GcImage();
	GcImagePrivate *const d = gcImage->d;
	d->init(w, h, GcImage::PXFMT_CI8);

	// Convert the palette.
	// TODO: Optimize using pointers instead of indexes?
	// TODO: Clear the top 240 entries?
	d->palette.resize(256);
	for (int i = 0; i < 16; i++) {
		d->palette[i] = ARGB4444_to_ARGB32(le16_to_cpu(pal_buf[i]));
	}

	uint8_t *px_dest = (uint8_t*)d->imageData;
	for (int i = img_siz; i > 0; i--, img_buf++, px_dest += 2) {
		*px_dest = (*img_buf >> 4);
		*(px_dest+1) = (*img_buf & 0xF);
	}

	// Image has been converted.
	return gcImage;
}

/**
 * Convert a Dreamcast ARGB4444 image to GcImage.
 * TODO: Split DC images into a separate class with a common base class?
 * @param w Image width.
 * @param h Image height.
 * @param img_buf ARGB4444 image buffer.
 * @param img_siz Size of image data. [must be >= (w*h)*2]
 * @return GcImage, or nullptr on error.
 */
GcImage *DcImageLoader::fromARGB4444(int w, int h, const uint16_t *img_buf, int img_siz)
{
	// Verify parameters.
	if (w < 0 || h < 0)
		return nullptr;
	if (img_siz < ((w * h) * 2))
		return nullptr;

	// NOTE: Dreamcast bitmaps are not tiled.

	// Create a GcImage.
	GcImage *gcImage = new GcImage();
	GcImagePrivate *const d = gcImage->d;
	d->init(w, h, GcImage::PXFMT_ARGB32);

	uint32_t *px_dest = (uint32_t*)d->imageData;
	for (int i = img_siz; i > 0; i--, img_buf++, px_dest++) {
		*px_dest = ARGB4444_to_ARGB32(le16_to_cpu(*img_buf));
	}

	// Image has been converted.
	return gcImage;
}

/**
 * Convert a Dreamcast monochrome image to GcImage.
 * TODO: Optional two-color palette?
 * @param w Image width. (must be a multiple of 8)
 * @param h Image height.
 * @param img_buf Monochrome image buffer.
 * @param img_siz Size of image data. [must be >= (w*h)/8]
 * @return GcImage, or nullptr on error.
 */
GcImage *DcImageLoader::fromMonochrome(int w, int h,
			const uint8_t *img_buf, int img_siz)
{
	// Verify parameters.
	if (w < 0 || h < 0)
		return nullptr;
	if (img_siz < ((w * h) / 8))
		return nullptr;

	// Image width must be a multiple of 8.
	if (w % 8 != 0)
		return nullptr;

	// Create a GcImage.
	// TODO: Make an internal 2-color format?
	GcImage *gcImage = new GcImage();
	GcImagePrivate *const d = gcImage->d;
	d->init(w, h, GcImage::PXFMT_CI8);

	// Convert the palette.
	// TODO: Optimize using pointers instead of indexes?
	// TODO: Clear the top 254 entries?
	d->palette.resize(256);
	d->palette[0] = 0xFFFFFFFF;	// white
	d->palette[1] = 0xFF000000;	// black

	// NOTE: MSB == left-most pixel.
	uint8_t *px_dest = (uint8_t*)d->imageData;
	for (int i = img_siz; i > 0; i--, img_buf++) {
		uint8_t px_src = *img_buf;
		for (int j = 0; j < 8; j++, px_src <<= 1) {
			// TODO: Which is faster:
			// - !!(px_src & 0x80)
			// - (px_src & 0x80) >> 7
			*px_dest++ = ((px_src & 0x80) >> 7);
		}
	}

	// Image has been converted.
	return gcImage;
}
