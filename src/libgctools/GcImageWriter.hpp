/***************************************************************************
 * GameCube Tools Library.                                                 *
 * GcImageWriter.hpp: GameCube image writer.                               *
 *                                                                         *
 * Copyright (c) 2012-2016 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#ifndef __LIBGCTOOLS_GCIMAGEWRITER_HPP__
#define __LIBGCTOOLS_GCIMAGEWRITER_HPP__

// C includes.
#include <stdint.h>

// C++ includes.
#include <vector>

class GcImage;

/**
 * GcImageWriter class.
 * Writes GcImage objects to image files.
 * 
 * NOTE: All const char* functions use ASCII.
 */
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
			IMGF_UNKNOWN	= 0,
			IMGF_PNG,	// PNG
			IMGF_MAX
		};

		/**
		 * Animated image formats.
		 */
		enum AnimImageFormat {
			ANIMGF_UNKNOWN	= 0,
			ANIMGF_APNG,	// APNG
			ANIMGF_GIF,	// GIF
			ANIMGF_PNG_FPF,	// File per frame
			ANIMGF_PNG_VS,	// PNG (vertical strip)
			ANIMGF_PNG_HS,	// PNG (horizontal strip)
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
		 * Get the name of the specified image image format.
		 * @param imgf Image format.
		 * @return Name of the image format, or nullptr if invalid.
		 */
		static const char *nameOfImageFormat(ImageFormat imgf);

		/**
		 * Get the description of the specified image image format.
		 * @param imgf Image format.
		 * @return Description of the image format, or nullptr if invalid.
		 */
		static const char *descOfImageFormat(ImageFormat imgf);

		/**
		 * Look up an image format from its name.
		 * @param imgf_str Image format name.
		 * @return Image format, or IMGF_UNKNOWN if unknown.
		 */
		static ImageFormat imageFormatFromName(const char *imgf_str);

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
		 * Get the name of the specified animated image format.
		 * @param animImgf Animated image format.
		 * @return Name of the animated image format, or nullptr if invalid.
		 */
		static const char *nameOfAnimImageFormat(AnimImageFormat animImgf);

		/**
		 * Get the description of the specified animated image format.
		 * @param animImgf Animated image format.
		 * @return Description of the animated image format, or nullptr if invalid.
		 */
		static const char *descOfAnimImageFormat(AnimImageFormat animImgf);

		/**
		 * Look up an animated image format from its name.
		 * @param animImgf_str Animated image format name.
		 * @return Animated image format, or IMGF_UNKNOWN if unknown.
		 */
		static AnimImageFormat animImageFormatFromName(const char *animImgf_str);

		/**
		 * Get the internal memory buffer. (first file only)
		 * @return Internal memory buffer, or nullptr if no files are in memory.
		 */
		const std::vector<uint8_t> *memBuffer(void) const;

		/**
		 * Get the internal memory buffer for the specified file.
		 * @param idx File number.
		 * @return Internal memory buffer, or nullptr if the file index is invalid.
		 */
		const std::vector<uint8_t> *memBuffer(int idx) const;

		/**
		 * Get the number of files currently in memory.
		 * @return Number of files.
		 */
		int numFiles(void) const;

		/**
		 * Clear the internal memory buffer.
		 */
		void clearMemBuffer(void);

		/**
		 * Write a GcImage to the internal memory buffer.
		 * @param gcImage	[in] GcImage.
		 * @param imgf		[in] Image format.
		 * @return 0 on success; non-zero on error.
		 */
		int write(const GcImage *gcImage, ImageFormat imgf);

		/**
		 * Write an animated GcImage to the internal memory buffer.
		 * @param gcImages	[in] Vector of GcImage.
		 * @param gcIconDelays	[in] Icon delays.
		 * @param animImgf	[in] Animated image format.
		 * @return 0 on success; non-zero on error.
		 */
		int write(const std::vector<const GcImage*> *gcImages,
			  const std::vector<int> *gcIconDelays,
			  AnimImageFormat animImgf);
};

#endif /* __LIBGCTOOLS_CHECKSUM_HPP__ */
