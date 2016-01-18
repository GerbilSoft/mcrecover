/***************************************************************************
 * GameCube Tools Library.                                                 *
 * GcImageWriter_p.hpp: GameCube image writer. (PRIVATE)                   *
 *                                                                         *
 * Copyright (c) 2012-2016 by David Korth.                                 *
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

#ifndef __LIBGCTOOLS_GCIMAGE_P_HPP__
#define __LIBGCTOOLS_GCIMAGE_P_HPP__

#include <config.libgctools.h>
#include "GcImageWriter.hpp"
class GcImage;

// C++ includes.
#include <vector>

// libpng
#ifdef HAVE_PNG
#include <png.h>
#include "APNG_dlopen.h"
#endif /* HAVE_PNG */

// giflib
#ifdef HAVE_GIF
#include <gif_lib.h>
#endif /* HAVE_GIF */

class GcImageWriterPrivate
{
	public:
		GcImageWriterPrivate(GcImageWriter *const q);
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
		 * @param gcImages	[in] Vector of GcImage.
		 * @return True if the gcImages are CI8_UNIQUE; false if not.
		 */
		bool is_gcImages_CI8_UNIQUE(const std::vector<const GcImage*> *gcImages);

		/**
		 * Check if a vector of gcImages is CI8_UNIQUE.
		 * If they are, convert them to ARGB32 and return the new vector.
		 * @param gcImages	[in] Vector of GcImage.
		 * @return Vector of ARGB32 GcImage if CI8_UNIQUE, or nullptr otherwise.
		 */
		std::vector<const GcImage*> *gcImages_from_CI8_UNIQUE(const std::vector<const GcImage*> *gcImages);

#ifdef HAVE_PNG
		/**
		 * PNG write function.
		 * @param png_ptr PNG pointer.
		 * @param buf Data to write.
		 * @param len Size of buf.
		 */
		static void png_io_write(png_structp png_ptr, png_bytep buf, png_size_t len);

		/**
		 * PNG flush function.
		 * Required when writing PNG images.
		 * This implementation is a no-op.
		 * @param png_ptr PNG pointer.
		 */
		static void png_io_flush(png_structp png_ptr);

		/**
		 * Write a PLTE chunk to a PNG image.
		 * @param png_ptr PNG pointer.
		 * @param info_ptr PNG info pointer.
		 * @param palette Palette. (ARGB32 format)
		 * @param num_entries Number of palette entries.
		 * @return 0 on success; non-zero on error.
		 */
		int writePng_PLTE(png_structp png_ptr, png_infop info_ptr,
				  const uint32_t *palette, int num_entries);

		/**
		 * Write an animated GcImage to the internal memory buffer in APNG format.
		 * @param gcImages	[in] Vector of GcImage.
		 * @param gcIconDelays	[in] Icon delays.
		 * @return 0 on success; non-zero on error.
		 */
		int writeAPng(const std::vector<const GcImage*> *gcImages, const std::vector<int> *gcIconDelays);

		/**
		 * Write an animated GcImage to the internal memory buffer in PNG FPF format.
		 * @param gcImages	[in] Vector of GcImage.
		 * @return 0 on success; non-zero on error.
		 */
		int writePng_FPF(const std::vector<const GcImage*> *gcImages);

		/**
		 * Write an animated GcImage to the internal memory buffer in PNG VS format.
		 * @param gcImages	[in] Vector of GcImage.
		 * @return 0 on success; non-zero on error.
		 */
		int writePng_VS(const std::vector<const GcImage*> *gcImages);

		/**
		 * Write an animated GcImage to the internal memory buffer in PNG HS format.
		 * @param gcImages	[in] Vector of GcImage.
		 * @return 0 on success; non-zero on error.
		 */
		int writePng_HS(const std::vector<const GcImage*> *gcImages);
#endif /* HAVE_PNG */

#ifdef HAVE_GIF
		/**
		 * GIF write function.
		 * @param gif GifFileType pointer.
		 * @param buf Data to write.
		 * @param len Size of buf.
		 * @return Number of bytes written.
		 */
		static int gif_output_func(GifFileType *gif, const GifByteType *buf, int len);

		/**
		 * Convert a GcImage palette to a GIF palette.
		 * @param colorMap	[out] GIF ColorMapObject.
		 * @param palette	[in] GcImage palette. (must have 256 entries)
		 * @return Index of transparent color, or -1 if no transparent color.
		 */
		static int paletteToGifColorMap(ColorMapObject *colorMap, const uint32_t *palette);
#endif /* HAVE_GIF */

	public:
#ifdef HAVE_PNG
		/**
		 * Write a GcImage to the internal memory buffer in PNG format.
		 * @param gcImage	[in] GcImage.
		 * @return 0 on success; non-zero on error.
		 */
		int writePng(const GcImage *gcImage);

		/**
		 * Write an animated GcImage to the internal memory buffer in some PNG format.
		 * @param gcImages	[in] Vector of GcImage.
		 * @param gcIconDelays	[in] Icon delays.
		 * @param animImgf	[in] Animated image format.
		 * @return 0 on success; non-zero on error.
		 */
		int writePng_anim(const std::vector<const GcImage*> *gcImages,
				  const std::vector<int> *gcIconDelays,
				  GcImageWriter::AnimImageFormat animImgf);
#endif /* HAVE_PNG */

#ifdef HAVE_GIF
		/**
		 * Write an animated GcImage to the internal memory buffer in GIF format.
		 * @param gcImages	[in] Vector of GcImage.
		 * @param gcIconDelays	[in] Icon delays.
		 * @return 0 on success; non-zero on error.
		 */
		int writeGif_anim(const std::vector<const GcImage*> *gcImages,
				  const std::vector<int> *gcIconDelays);
#endif /* HAVE_GIF */
};

#endif /* __LIBGCTOOLS_GCIMAGE_P_HPP__ */
