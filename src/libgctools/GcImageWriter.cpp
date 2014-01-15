/***************************************************************************
 * GameCube Tools Library.                                                 *
 * GcImageWriter.cpp: GameCube image writer.                               *
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

#include "GcImageWriter.hpp"
#include "GcImage.hpp"

// C includes.
#include <errno.h>
#include <setjmp.h>
#include <stdint.h>
#include <stdlib.h>

// libpng
#include <png.h>

class GcImageWriterPrivate
{
	private:
		// Static class.
		GcImageWriterPrivate();
		~GcImageWriterPrivate();
		// TODO: Copy Qt's Q_DISABLE_COPY() macro.
		GcImageWriterPrivate(const GcImageWriterPrivate &);
		GcImageWriterPrivate &operator=(const GcImageWriterPrivate &);

	public:
		static int writePng(const GcImage *image, const char *filename);
};

/**
 * Write a GcImage to an image file.
 * @param gcImage	[in] GcImage.
 * @param filename	[in] Output filename. (UTF-8)
 * @return 0 on success; non-zero on error.
 */
int GcImageWriterPrivate::writePng(const GcImage *image, const char *filename)
{
	if (!image || !filename)
		return -EINVAL;

	// TODO: Support CI8.
	if (image->pxFmt() != GcImage::PXFMT_ARGB32)
		return -EINVAL;

	// TODO: fopen() wrapper to convert UTF-8 filename on Windows.
	FILE *fpng = fopen(filename, "wb");
	if (!fpng)
		return -errno;

	png_structp png_ptr;
	png_infop info_ptr;

	// Initialize libpng.
	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr) {
		// Could not create PNG write struct.
		fclose(fpng);
		return -0x101;
	}
	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {
		// Could not create PNG info struct.
		png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
		fclose(fpng);
		return -0x102;
	}

	if (setjmp(png_jmpbuf(png_ptr))) {
		// PNG write failed.
		png_destroy_write_struct(&png_ptr, &info_ptr);
		fclose(fpng);
		return -0x103;
	}

	// libpng initialization.
	png_init_io(png_ptr, fpng);
	png_set_filter(png_ptr, 0, PNG_FILTER_NONE);
	png_set_compression_level(png_ptr, 5);	// TODO: Customizable?

	const int w = image->width();
	const int h = image->height();

	// TODO: Support CI8.
	switch (image->pxFmt()) {
		case GcImage::PXFMT_ARGB32:
			png_set_IHDR(png_ptr, info_ptr, w, h,
					8, PNG_COLOR_TYPE_RGB_ALPHA,
					PNG_INTERLACE_NONE,
					PNG_COMPRESSION_TYPE_DEFAULT,
					PNG_FILTER_TYPE_DEFAULT);
			break;

		case GcImage::PXFMT_CI8:
		default:
			// Unsupported pixel format.
			png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
			fclose(fpng);
			return -EINVAL;
	}

	// Write the PNG information to the file.
	png_write_info(png_ptr, info_ptr);

	// TODO: Byteswap image data on big-endian systems?
	//ppng_set_swap(png_ptr);

	// Calculate the row pointers.
	const png_byte **row_pointers = static_cast<const png_byte**>(malloc(sizeof(png_byte*) * h));
	switch (image->pxFmt()) {
		case GcImage::PXFMT_ARGB32: {
			const uint8_t *imageData = (const uint8_t*)image->imageData();
			const int pitch = (w * 4);
			for (int y = 0; y < h; y++, imageData += pitch)
				row_pointers[y] = imageData;
			// TODO: What format on big-endian?
			png_set_bgr(png_ptr);
			break;
		}

		case GcImage::PXFMT_CI8:
		default:
			// Unsupported pixel format.
			free(row_pointers);
			png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
			fclose(fpng);
			return -EINVAL;
	}

	// Write the image data.
	png_write_rows(png_ptr, (png_bytepp)row_pointers, h);
	free(row_pointers);

	// Finished writing.
	png_write_end(png_ptr, info_ptr);
	png_destroy_write_struct(&png_ptr, &info_ptr);
	fclose(fpng);
	return 0;
}

/**
 * Write a GcImage to an image file.
 * @param gcImage	[in] GcImage.
 * @param filename	[in] Output filename. (UTF-8)
 * @param imgf		[in] Image format.
 * @return 0 on success; non-zero on error.
 */
int GcImageWriter::write(const GcImage *image, const char *filename, ImageFormat imgf)
{
	switch (imgf) {
		case IMGF_PNG:
			return GcImageWriterPrivate::writePng(image, filename);

		default:
			break;
	}

	// Invalid image format.
	return -EINVAL;
}
