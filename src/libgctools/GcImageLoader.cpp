/***************************************************************************
 * GameCube Tools Library.                                                 *
 * GcImageLoader.cpp: GameCube image loader.                               *
 * Converts GameCube images from native formats to GcImage.                *
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

#include "GcImageLoader.hpp"
#include "GcImage_p.hpp"

// Byteswapping macros.
#include "util/byteswap.h"

// C includes. (C++ namespace)
#include <cstring>

/**
 * Convert an RGB5A3 pixel to ARGB32.
 * @param px16 RGB5A3 pixel.
 * @return ARGB32 pixel.
 */
static inline uint32_t RGB5A3_to_ARGB32(uint16_t px16)
{
	uint32_t px32 = 0;

	// NOTE: Pixels are byteswapped.
	if (px16 & 0x8000) {
		// RGB5
		px32 |= (((px16 << 3) & 0x0000F8) | ((px16 >> 2) & 0x000007));	// B
		px32 |= (((px16 << 6) & 0x00F800) | ((px16 << 3) & 0x000700));	// G
		px32 |= (((px16 << 9) & 0xF80000) | ((px16 << 6) & 0x070000));	// R
		px32 |= 0xFF000000U; // no alpha channel
	} else {
		// RGB4A3
		px32 |= (((px16 & 0x000F) << 4) | (px16 & 0x000F));		// B
		px32 |= (((px16 & 0x00F0) << 4) | ((px16 & 0x00F0) << 8));	// G
		px32 |= (((px16 & 0x0F00) << 8) | ((px16 & 0x0F00) << 12));	// R

		// Calculate the alpha channel.
		uint8_t a = ((px16 >> 7) & 0xE0);
		a |= (a >> 3);
		a |= (a >> 3);

		// Apply the alpha channel.
		px32 |= (a << 24);
	}

	return px32;
}

/**
 * Blit an ARGB32 tile to an ARGB32 linear image buffer.
 * @param pixel		[in] Pixel type.
 * @param tileW		[in] Tile width.
 * @param tileH		[in] Tile height.
 * @param imgBuf	[out] Linear image buffer.
 * @param pitch		[in] Pitch of image buffer, in pixels.
 * @param tileBuf	[in] Tile buffer.
 * @param tileX		[in] Horizontal tile number.
 * @param tileY		[in] Vertical tile number.
 */
template<typename pixel, int tileW, int tileH>
static inline void BlitTile(pixel *imgBuf, int pitch,
			    const pixel *tileBuf, int tileX, int tileY)
{
	// Go to the first pixel for this tile.
	imgBuf += ((tileY * tileH * pitch) + (tileX * tileW));

	for (int y = tileH; y != 0; y--) {
		memcpy(imgBuf, tileBuf, (tileW * sizeof(pixel)));
		imgBuf += pitch;
		tileBuf += tileW;
	}
}

/**
 * Convert a GameCube CI8 image to GcImage.
 * @param w Image width.
 * @param h Image height.
 * @param img_buf CI8 image buffer.
 * @param img_siz Size of image data. [must be >= (w*h)]
 * @param pal_buf Palette buffer.
 * @param pal_siz Size of palette data. [must be >= 0x200]
 * @return QImage, or empty QImage on error.
 */
GcImage *GcImageLoader::fromCI8(int w, int h,
			const uint8_t *img_buf, int img_siz,
			const uint16_t *pal_buf, int pal_siz)
{
	// Verify parameters.
	if (w < 0 || h < 0)
		return nullptr;
	if (img_siz < (w * h) || pal_siz < 0x200)
		return nullptr;

	// CI8 uses 8x4 tiles.
	if (w % 8 != 0 || h % 4 != 0)
		return nullptr;

	// Calculate the total number of tiles.
	const int tilesX = (w / 8);
	const int tilesY = (h / 4);

	// Create a GcImage.
	GcImage *gcImage = new GcImage();
	GcImagePrivate *const d = gcImage->d;
	d->init(w, h, GcImage::PXFMT_CI8);

	// Convert the palette.
	// TODO: Optimize using pointers instead of indexes?
	d->palette.resize(256);
	for (int i = 0; i < 256; i++) {
		d->palette[i] = RGB5A3_to_ARGB32(be16_to_cpu(pal_buf[i]));
	}

	// Tile pointer.
	const uint8_t *tileBuf = img_buf;

	for (int y = 0; y < tilesY; y++) {
		for (int x = 0; x < tilesX; x++) {
			// Decode the current tile.
			BlitTile<uint8_t, 8, 4>((uint8_t*)d->imageData, w, tileBuf, x, y);
			tileBuf += (8 * 4);
		}
	}

	// Image has been converted.
	return gcImage;
}


/**
 * Convert a GameCube RGB5A3 image to GcImage.
 * @param w Image width.
 * @param h Image height.
 * @param img_buf CI8 image buffer.
 * @param img_siz Size of image data. [must be >= (w*h)*2]
 * @return GcImage, or nullptr on error.
 */
GcImage *GcImageLoader::fromRGB5A3(int w, int h, const uint16_t *img_buf, int img_siz)
{
	// Verify parameters.
	if (w < 0 || h < 0)
		return nullptr;
	if (img_siz < ((w * h) * 2))
		return nullptr;

	// RGB5A3 uses 4x4 tiles.
	if (w % 4 != 0 || h % 4 != 0)
		return nullptr;

	// Calculate the total number of tiles.
	const int tilesX = (w / 4);
	const int tilesY = (h / 4);

	// Create a GcImage.
	GcImage *gcImage = new GcImage();
	GcImagePrivate *const d = gcImage->d;
	d->init(w, h, GcImage::PXFMT_ARGB32);

	// Temporary tile buffer.
	uint32_t tileBuf[32];

	for (int y = 0; y < tilesY; y++) {
		for (int x = 0; x < tilesX; x++) {
			// Convert each tile to ARGB888 manually.
			// TODO: Optimize using pointers instead of indexes?
			for (int i = 0; i < 4*4; i++, img_buf++) {
				tileBuf[i] = RGB5A3_to_ARGB32(be16_to_cpu(*img_buf));
			}

			// Blit the tile to the main image buffer.
			BlitTile<uint32_t, 4, 4>((uint32_t*)d->imageData, w, tileBuf, x, y);
		}
	}

	// Image has been converted.
	return gcImage;
}
