/***************************************************************************
 * GameCube Tools Library.                                                 *
 * GcImageWriter.hpp: GameCube image writer.                               *
 *                                                                         *
 * Copyright (c) 2012-2014 by David Korth.                                 *
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

#ifndef __LIBGCTOOLS_GCIMAGEWRITER_HPP__
#define __LIBGCTOOLS_GCIMAGEWRITER_HPP__

// C includes.
#include <stdint.h>

// C++ includes.
#include <vector>

class GcImage;

class GcImageWriterPrivate;

class GcImageWriter
{
	public:
		GcImageWriter();
		~GcImageWriter();

	private:
		friend class GcImageWriterPrivate;
		GcImageWriterPrivate *const d;
		// TODO: Copy Qt's Q_DISABLE_COPY() macro.
		GcImageWriter(const GcImageWriter &);
		GcImageWriter &operator=(const GcImageWriter &);

	public:
		/**
		 * Image formats.
		 */
		enum ImageFormat {
			IMGF_PNG	= 0,	// PNG

			IMGF_MAX
		};

		/**
		 * Animated image formats.
		 */
		enum AnimImageFormat {
			ANIMGF_APNG	= 0,	// APNG
			ANIMGF_GIF	= 1,	// GIF
			ANIMGF_PNG_FPF	= 2,	// File per frame
			ANIMGF_PNG_VS	= 3,	// PNG (vertical strip)
			ANIMGF_PNG_HS	= 4,	// PNG (horizontal strip)

			ANIMGF_MAX
		};

		/**
		 * Check if an image format is supported.
		 * @param imgf Image format.
		 * @return True if supported; false if not.
		 */
		static bool isImageFormatSupported(ImageFormat imgf);

		/**
		 * Get the file extension for the specified image format.
		 * @param imgf Image format.
		 * @return File extension (ASCII), without the dot, or nullptr if imgf is invalid.
		 */
		static const char *extForImageFormat(ImageFormat imgf);

		/**
		 * Check if an animated image format is supported.
		 * @param animImgf Animated image format.
		 * @return True if supported; false if not.
		 */
		static bool isAnimImageFormatSupported(AnimImageFormat animImgf);

		/**
		 * Get the file extension for the specified animated image format.
		 * @param animImgf Animated image format.
		 * @return File extension (ASCII), without the dot, or nullptr if animImgf is invalid.
		 */
		static const char *extForAnimImageFormat(AnimImageFormat animImgf);

		/**
		 * Write a GcImage to the internal memory buffer.
		 * @param gcImage	[in] GcImage.
		 * @param imgf		[in] Image format.
		 * @return 0 on success; non-zero on error.
		 */
		int write(const GcImage *image, ImageFormat imgf);

		/**
		 * Get the internal memory buffer.
		 * @return Internal memory buffer.
		 */
		const std::vector<uint8_t> *memBuffer(void) const;
};

#endif /* __LIBGCTOOLS_CHECKSUM_HPP__ */
