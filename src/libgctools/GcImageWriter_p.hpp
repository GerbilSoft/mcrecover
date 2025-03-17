/***************************************************************************
 * GameCube Tools Library.                                                 *
 * GcImageWriter_p.hpp: GameCube image writer. (PRIVATE)                   *
 *                                                                         *
 * Copyright (c) 2012-2025 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#pragma once

#include <config.libgctools.h>
#include "GcImageWriter.hpp"
class GcImage;

// C includes.
#include <stdint.h>

// C++ includes.
#include <vector>

// libpng
#ifdef HAVE_PNG
#include <png.h>
#include "APNG_dlopen.h"
#endif /* HAVE_PNG */

// giflib
#include "GIF_dlopen.h"

// TODO: Split PNG and GIF into separate classes.
// Need to make a common class for the CI8_UNIQUE functions.

class GcImageWriterPrivate
{
public:
	explicit GcImageWriterPrivate(GcImageWriter *const q);
	~GcImageWriterPrivate();

private:
	GcImageWriter *const q;
	// TODO: Copy Qt's Q_DISABLE_COPY() macro.
	GcImageWriterPrivate(const GcImageWriterPrivate &);
	GcImageWriterPrivate &operator=(const GcImageWriterPrivate &);

public:
	// Internal memory buffers.
	// Each call to write() creates a new buffer.
	std::vector<std::vector<uint8_t>* > memBuffer;

private:
	/**
	 * Check if a vector of gcImages is CI8_UNIQUE.
	 * @param gcImages	[in] Vector of GcImage
	 * @return True if the gcImages are CI8_UNIQUE; false if not.
	 */
	static bool is_gcImages_CI8_UNIQUE(
		const std::vector<const GcImage*> *gcImages);

	/**
	 * Check if a vector of gcImages is CI8_UNIQUE.
	 * If they are, convert them to ARGB32 and return the new vector.
	 * @param gcImages	[in] Vector of GcImage
	 * @return Vector of ARGB32 GcImage if CI8_UNIQUE, or nullptr otherwise.
	 */
	static std::vector<const GcImage*> *gcImages_from_CI8_UNIQUE(
		const std::vector<const GcImage*> *gcImages);

#ifdef HAVE_PNG
	/**
	 * PNG write function.
	 * @param png_ptr	[in] PNG pointer
	 * @param buf		[in] Data to write
	 * @param len		[in] Size of buf
	 */
	static void png_io_write(png_structp png_ptr, png_bytep buf, png_size_t len);

	/**
	 * PNG flush function.
	 * Required when writing PNG images.
	 * This implementation is a no-op.
	 * @param png_ptr	[in] PNG pointer
	 */
	static void png_io_flush(png_structp png_ptr);

	/**
	 * Write a PLTE chunk to a PNG image.
	 * @param png_ptr	[in] PNG pointer
	 * @param info_ptr	[in] PNG info pointer
	 * @param palette	[in] Palette (ARGB32 format)
	 * @param num_entries	[in] Number of palette entries
	 * @return 0 on success; non-zero on error.
	 */
	static int writePng_PLTE(png_structp png_ptr, png_infop info_ptr,
				 const uint32_t *palette, int num_entries);

	/**
	 * Write an animated GcImage to the internal memory buffer in APNG format.
	 * @param gcImages	[in] Vector of GcImage
	 * @param gcIconDelays	[in] Icon delays
	 * @return 0 on success; non-zero on error.
	 */
	int writeAPng(const std::vector<const GcImage*> *gcImages, const std::vector<int> *gcIconDelays);

	/**
	 * Write an animated GcImage to the internal memory buffer in PNG FPF format.
	 * @param gcImages	[in] Vector of GcImage
	 * @return 0 on success; non-zero on error.
	 */
	int writePng_FPF(const std::vector<const GcImage*> *gcImages);

	/**
	 * Write an animated GcImage to the internal memory buffer in PNG VS format.
	 * @param gcImages	[in] Vector of GcImage
	 * @return 0 on success; non-zero on error.
	 */
	int writePng_VS(const std::vector<const GcImage*> *gcImages);

	/**
	 * Write an animated GcImage to the internal memory buffer in PNG HS format.
	 * @param gcImages	[in] Vector of GcImage
	 * @return 0 on success; non-zero on error.
	 */
	int writePng_HS(const std::vector<const GcImage*> *gcImages);
#endif /* HAVE_PNG */

#ifdef USE_GIF
	/**
	 * GIF write function.
	 * @param gif	[in] GifFileType pointer
	 * @param buf	[in] Data to write
	 * @param len	[in] Size of buf
	 * @return Number of bytes written.
	 */
	static int gif_output_func(GifFileType *gif, const GifByteType *buf, int len);

	/**
	 * Convert a GcImage palette to a GIF palette.
	 * @param colorMap	[out] GIF ColorMapObject
	 * @param palette	[in] GcImage palette (must have 256 entries)
	 * @return Index of transparent color, or -1 if no transparent color.
	 */
	static int paletteToGifColorMap(ColorMapObject *colorMap, const uint32_t *palette);

	/**
	 * Add a loop extension block to the GIF image.
	 * @param gif		[in] GIF image
	 * @param loopCount	[in] Loop count (0 == infinite)
	 * @return GIF_OK on success; GIF_ERROR on error.
	 */
	static int gif_addLoopExtension(GifFileType *gif, uint16_t loopCount);

	/**
	 * Add a graphics control block to a GIF frame.
	 * @param gif		[in] GIF image
	 * @param trans_idx	[in] Transparent color index (-1 for no transparency)
	 * @param iconDelay	[in] Icon delay, in centiseconds
	 */
	static int gif_addGraphicsControlBlock(GifFileType *gif, int trans_idx, uint16_t iconDelay);

	/**
	 * Write an ARGB32 image to a GIF.
	 * This will reduce the image to 256 colors first.
	 * @param gif		[in] GIF image
	 * @param gcImage	[in] GcImage to write
	 * @param colorMap	[in] Color map object to use
	 * @return GIF_OK on success; GIF_ERROR on error.
	 */
	static int gif_writeARGB32Image(GifFileType *gif,
			const GcImage *gcImage, ColorMapObject *colorMap);
#endif /* USE_GIF */

public:
#ifdef HAVE_PNG
	/**
	 * Write a GcImage to the internal memory buffer in PNG format.
	 * @param gcImage	[in] GcImage
	 * @return 0 on success; non-zero on error.
	 */
	int writePng(const GcImage *gcImage);

	/**
	 * Write an animated GcImage to the internal memory buffer in some PNG format.
	 * @param gcImages	[in] Vector of GcImage
	 * @param gcIconDelays	[in] Icon delays
	 * @param animImgf	[in] Animated image format
	 * @return 0 on success; non-zero on error.
	 */
	int writePng_anim(const std::vector<const GcImage*> *gcImages,
			  const std::vector<int> *gcIconDelays,
			  GcImageWriter::AnimImageFormat animImgf);
#endif /* HAVE_PNG */

#ifdef USE_GIF
	/**
	 * Write an animated GcImage to the internal memory buffer in GIF format.
	 * @param gcImages	[in] Vector of GcImage
	 * @param gcIconDelays	[in] Icon delays
	 * @return 0 on success; non-zero on error.
	 */
	int writeGif_anim(const std::vector<const GcImage*> *gcImages,
			  const std::vector<int> *gcIconDelays);
#endif /* USE_GIF */
};
