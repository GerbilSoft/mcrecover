/***************************************************************************
 * GameCube Tools Library.                                                 *
 * GcImage.hpp: GameCube image format handler.                             *
 *                                                                         *
 * Copyright (c) 2012-2015 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
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
