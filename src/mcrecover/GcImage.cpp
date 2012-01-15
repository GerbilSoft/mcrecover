/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * GcImage.cpp: GameCube image format handler.                             *
 *                                                                         *
 * Copyright (c) 2011 by David Korth.                                      *
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

#include "GcImage.hpp"

// Byteswapping macros.
#include "byteswap.h"

// C includes.
#include <stdint.h>

// Qt includes.
#include <QtGui/QImage>
#include <QtGui/QPainter>


/**
 * Convert an RGB5A3 pixel to ARGB8888.
 * @param px16 RGB5A3 pixel.
 * @return ARGB888 pixel.
 */
static inline uint32_t RGB5A3_to_ARGB8888(uint16_t px16)
{
	uint32_t px32 = 0;
	
	px32 |= (((px16 << 3) & 0x0000F8) | ((px16 >> 2) & 0x000007));	// B
	px32 |= (((px16 << 6) & 0x00F800) | ((px16 << 3) & 0x000700));	// G
	px32 |= (((px16 << 9) & 0xF80000) | ((px16 << 6) & 0x070000));	// R
	
	// TODO: Fix alpha channel!
	// For now, just use white for alpha.
	if (px16 & 0x8000)
		px32 |= 0xFF000000U;
	
	return px32;
}


/**
 * Convert a GameCube CI8 image to QImage.
 * @param w Image width.
 * @param h Image height.
 * @param img_buf CI8 image buffer.
 * @param img_siz Size of image data. [must be >= (w*h)]
 * @param pal_buf Palette buffer.
 * @param pal_siz Size of palette data. [must be >= 0x200]
 * @return QImage, or empty QImage on error.
 */
QImage GcImage::FromCI8(int w, int h, const void *img_buf, int img_siz,
			const void *pal_buf, int pal_siz)
{
	// Verify parameters.
	if (w < 0 || h < 0)
		return QImage();
	if (img_siz < (w * h) || pal_siz < 0x200)
		return QImage();
	
	// CI8 uses 8x4 tiles.
	if (w % 8 != 0 || h % 4 != 0)
		return QImage();
	
	// Calculate the total number of tiles.
	const int tilesX = (w / 8);
	const int tilesY = (h / 4);
	
	// Convert the palette.
	QVector<QRgb> palette;
	palette.resize(256);
	uint16_t *pal5A3 = (uint16_t*)pal_buf;
	for (int i = 0; i < 256; i++)
	{
		palette[i] = RGB5A3_to_ARGB8888(be16_to_cpu(*pal5A3));
		pal5A3++;
	}
	
	// Temporary images.
	QImage qimg(w, h, QImage::Format_ARGB32);
	memset(qimg.bits(), 0x00, (w * h * sizeof(uint32_t)));
	QPainter painter(&qimg);
	const uint8_t *tile_ptr = (const uint8_t*)img_buf;
	
	for (int y = 0; y < tilesY; y++)
	{
		for (int x = 0; x < tilesX; x++)
		{
			// Let QImage handle the 256-color image directly.
			QImage tile(tile_ptr, 8, 4, QImage::Format_Indexed8);
			tile.setColorTable(palette);
			tile_ptr += (8*4);
			
			// Blit the tile to the final image.
			painter.drawImage(x*8, y*4, tile);
		}
	}
	
	// Image has been converted.
	return qimg;
}


/**
 * Convert a GameCube RGB5A3 image to QImage.
 * @param w Image width.
 * @param h Image height.
 * @param img_buf CI8 image buffer.
 * @param img_siz Size of image data. [must be >= (w*h)*2]
 * @return QImage, or empty QImage on error.
 */
QImage GcImage::FromRGB5A3(int w, int h, const void *img_buf, int img_siz)
{
	// NOTE: This should probably be RGB5A1,
	// but everything uses RGB5A3.
	
	// Verify parameters.
	if (w < 0 || h < 0)
		return QImage();
	if (img_siz < ((w * h) * 2))
		return QImage();
	
	// RGB5A3 uses 4x4 tiles.
	if (w % 4 != 0 || h % 4 != 0)
		return QImage();
	
	// Calculate the total number of tiles.
	const int tilesX = (w / 4);
	const int tilesY = (h / 4);
	const uint16_t *buf5A3 = (uint16_t*)img_buf;
	
	// Temporary images.
	QImage qimg(w, h, QImage::Format_ARGB32);
	memset(qimg.bits(), 0x00, (w * h * sizeof(uint32_t)));
	QPainter painter(&qimg);
	uint32_t *tile_buf = (uint32_t*)malloc(4 * 4 * sizeof(uint32_t));
	
	for (int y = 0; y < tilesY; y++)
	{
		for (int x = 0; x < tilesX; x++)
		{
			// Convert each tile to ARGB888 manually.
			uint32_t *tile_ptr = tile_buf;
			for (int i = 0; i < 4*4; i++)
			{
				*tile_ptr++ = RGB5A3_to_ARGB8888(be16_to_cpu(*buf5A3));
				
				// NOTE: buf5A3 must be incremented OUTSIDE of the
				// be16_to_cpu() macro! Otherwise, shenanigans will ensue.
				buf5A3++;
			}
			
			// Convert to QImage, then blit to the final image.
			QImage tile((uint8_t*)tile_buf, 4, 4, QImage::Format_ARGB32);
			painter.drawImage(x*4, y*4, tile);
		}
	}
	
	// Image has been converted.
	return qimg;
}
