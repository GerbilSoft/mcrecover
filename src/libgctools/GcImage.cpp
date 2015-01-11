/***************************************************************************
 * GameCube Tools Library.                                                 *
 * GcImage.cpp: GameCube image format handler.                             *
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

#include "GcImage.hpp"

// Byteswapping macros.
#include "util/byteswap.h"

/** GcImagePrivate **/
#include "GcImage_p.hpp"
using std::vector;

GcImagePrivate::GcImagePrivate()
	: imageData(nullptr)
	, imageData_len(0)
	, pxFmt(GcImage::PXFMT_NONE)
	, width(0)
	, height(0)
{ }

GcImagePrivate::~GcImagePrivate()
	{ free(imageData); }

GcImagePrivate::GcImagePrivate(const GcImagePrivate &other)
	: imageData_len(other.imageData_len)
	, palette(other.palette)
	, pxFmt(other.pxFmt)
	, width(other.width)
	, height(other.height)
{
	// Copy the image data.
	if (imageData_len == 0) {
		imageData = nullptr;
	} else {
		imageData = malloc(imageData_len);
		memcpy(imageData, other.imageData, imageData_len);
	}
}

/**
 * Initialize the GcImage.
 * @param w Width.
 * @param h Height.
 * @param pxFmt Pixel format.
 */
void GcImagePrivate::init(int w, int h, GcImage::PxFmt pxFmt)
{
	// Clear all existing image data.
	free(imageData);
	imageData = nullptr;
	imageData_len = 0;
	palette.clear();
	width = 0;
	height = 0;
	this->pxFmt = GcImage::PXFMT_NONE;

	if (w > 0 && h > 0) {
		size_t imgSize;
		switch (pxFmt) {
			case GcImage::PXFMT_CI8:
				imgSize = (w * h);
				break;
			case GcImage::PXFMT_ARGB32:
				imgSize = (w * h * 4);
				break;
			default:
				imgSize = 0;
				break;
		}

		if (imgSize > 0) {
			imageData = malloc(imgSize);
			if (imageData) {
				imageData_len = imgSize;
				width = w;
				height = h;
				this->pxFmt = pxFmt;
			}
		}
	}
}

/** GcImage **/

GcImage::GcImage()
	: d(new GcImagePrivate())
{ }

GcImage::~GcImage()
	{ delete d; }

GcImage::GcImage(const GcImage &other)
	: d(new GcImagePrivate(*other.d))
{ }

/**
 * Convert this GcImage to RGB5A3.
 * FIXME: Rename to "toARGB32()".
 * Caller must delete the returned GcImage.
 * @return New GcImage in RGB5A3.
 */
GcImage *GcImage::toRGB5A3(void) const
{
	switch (d->pxFmt) {
		case PXFMT_ARGB32:
			// Image is already ARGB32.
			return new GcImage(*this);

		case PXFMT_CI8: {
			// CI8. Convert to ARGB32.
			GcImage *gcImage = new GcImage();
			GcImagePrivate *const d_new = gcImage->d;
			d_new->init(d->width, d->height, PXFMT_ARGB32);

			const uint8_t *ci8 = reinterpret_cast<const uint8_t*>(d->imageData);
			uint32_t *rgb5A3 = reinterpret_cast<uint32_t*>(d_new->imageData);
			size_t len = d->imageData_len;
			for (; len >= 4; len -= 4, ci8 += 4, rgb5A3 += 4) {
				*(rgb5A3 + 0) = d->palette[*(ci8 + 0)];
				*(rgb5A3 + 1) = d->palette[*(ci8 + 1)];
				*(rgb5A3 + 2) = d->palette[*(ci8 + 2)];
				*(rgb5A3 + 3) = d->palette[*(ci8 + 3)];
			}
			// Just in case the image size isn't divisible by 4...
			for (; len > 0; len--, ci8++, rgb5A3++) {
				*rgb5A3 = d->palette[*ci8];
			}

			// Image is converted.
			return gcImage;
		}

		case PXFMT_NONE:
		default:
			// Invalid image format.
			break;
	}

	// Invalid image format.
	return nullptr;
}

GcImage::PxFmt GcImage::pxFmt(void) const
	{ return d->pxFmt; }
const void *GcImage::imageData(void) const
	{ return d->imageData; }
size_t GcImage::imageData_len(void) const
	{ return d->imageData_len; }
int GcImage::width(void) const
	{ return d->width; }
int GcImage::height(void) const
	{ return d->height; }

/**
 * Get the image palette.
 * @return Pointer to 256-element ARGB888 palette, or NULL if no palette.
 */
const uint32_t *GcImage::palette(void) const
{
	if (d->pxFmt != PXFMT_CI8 || d->palette.size() != 256)
		return nullptr;
	return d->palette.data();
}
