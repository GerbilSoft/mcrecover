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

#include "config.libgctools.h"
#include "GcImageWriter.hpp"
#include "GcImage.hpp"

// C includes.
#include <errno.h>
#include <setjmp.h>
#include <stdint.h>
#include <stdlib.h>

// C++ includes.
#include <vector>
using std::vector;

// libpng
#ifdef HAVE_PNG
#include <png.h>
#endif

/** GcImageWriterPrivate **/

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
		// Internal memory buffer.
		vector<uint8_t> memBuffer;

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
		 * Write a GcImage to the internal memory buffer in PNG format.
		 * @param gcImage	[in] GcImage.
		 * @return 0 on success; non-zero on error.
		 */
		int writePng(const GcImage *image);
};

GcImageWriterPrivate::GcImageWriterPrivate(GcImageWriter *const q)
	: q(q)
{ }

GcImageWriterPrivate::~GcImageWriterPrivate()
{ }

/**
 * PNG write function.
 * @param png_ptr PNG pointer.
 * @param buf Data to write.
 * @param len Size of buf.
 */
void GcImageWriterPrivate::png_io_write(png_structp png_ptr, png_bytep buf, png_size_t len)
{
	void *io_ptr = png_get_io_ptr(png_ptr);
	if (!io_ptr)
		return;

	// Assuming the io_ptr is a GcImageWriterPrivate.
	GcImageWriterPrivate *d = (GcImageWriterPrivate*)io_ptr;
	size_t pos = d->memBuffer.size();
	d->memBuffer.resize(pos + len);
	memcpy(&d->memBuffer.data()[pos], buf,len);
}

/**
 * PNG flush function.
 * Required when writing PNG images.
 * This implementation is a no-op.
 * @param png_ptr PNG pointer.
 */
void GcImageWriterPrivate::png_io_flush(png_structp png_ptr)
{
	// Do nothing!
	((void)png_ptr);
}

/**
 * Write a GcImage to the internal memory buffer in PNG format.
 * @param gcImage	[in] GcImage.
 * @return 0 on success; non-zero on error.
 */
int GcImageWriterPrivate::writePng(const GcImage *image)
{
	// Clear the internal memory buffer.
	   memBuffer.clear();

#ifdef HAVE_PNG
	if (!image)
		return -EINVAL;

	png_structp png_ptr;
	png_infop info_ptr;

	// Initialize libpng.
	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr) {
		// Could not create PNG write struct.
		return -0x101;
	}
	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {
		// Could not create PNG info struct.
		png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
		return -0x102;
	}

	if (setjmp(png_jmpbuf(png_ptr))) {
		// PNG write failed.
		png_destroy_write_struct(&png_ptr, &info_ptr);
		return -0x103;
	}

	// Initialize the internal buffer and memory write function.
	   memBuffer.reserve(32768);	// 32 KB should cover most of the use cases.
	png_set_write_fn(png_ptr, this, png_io_write, png_io_flush);

	// Initialize compression parameters.
	png_set_filter(png_ptr, 0, PNG_FILTER_NONE);
	png_set_compression_level(png_ptr, 5);	// TODO: Customizable?

	const int w = image->width();
	const int h = image->height();

	switch (image->pxFmt()) {
		case GcImage::PXFMT_ARGB32:
			png_set_IHDR(png_ptr, info_ptr, w, h,
					8, PNG_COLOR_TYPE_RGB_ALPHA,
					PNG_INTERLACE_NONE,
					PNG_COMPRESSION_TYPE_DEFAULT,
					PNG_FILTER_TYPE_DEFAULT);
			break;

		case GcImage::PXFMT_CI8: {
			png_set_IHDR(png_ptr, info_ptr, w, h,
					8, PNG_COLOR_TYPE_PALETTE,
					PNG_INTERLACE_NONE,
					PNG_COMPRESSION_TYPE_DEFAULT,
					PNG_FILTER_TYPE_DEFAULT);

			// Set the palette and tRNS values.
			png_color png_pal[256];
			uint8_t png_tRNS[256];
			const uint32_t *palette = image->palette();
			for (int i = 0; i < 256; i++) {
				png_pal[i].red   = ((palette[i] >> 16) & 0xFF);
				png_pal[i].green = ((palette[i] >> 8) & 0xFF);
				png_pal[i].blue  = (palette[i] & 0xFF);
				png_tRNS[i]      = ((palette[i] >> 24) & 0xFF);
			}
			png_set_PLTE(png_ptr, info_ptr, png_pal, sizeof(png_pal)/sizeof(png_pal[0]));
			png_set_tRNS(png_ptr, info_ptr, png_tRNS, sizeof(png_tRNS)/sizeof(png_tRNS[0]), nullptr);
			break;
		}

		default:
			// Unsupported pixel format.
			png_destroy_write_struct(&png_ptr, (png_infopp)nullptr);
			     memBuffer.clear();
			return -EINVAL;
	}

	// Write the PNG information to the file.
	png_write_info(png_ptr, info_ptr);

	// TODO: Byteswap image data on big-endian systems?
	//ppng_set_swap(png_ptr);

	// Calculate the row pointers.
	int pitch;
	switch (image->pxFmt()) {
		case GcImage::PXFMT_ARGB32:
			pitch = (w * 4);
			break;

		case GcImage::PXFMT_CI8:
			pitch = w;
			break;

		default:
			// Unsupported pixel format.
			png_destroy_write_struct(&png_ptr, (png_infopp)nullptr);
			     memBuffer.clear();
			return -EINVAL;
	}

	const uint8_t *imageData = (const uint8_t*)image->imageData();
	const png_byte **row_pointers = static_cast<const png_byte**>(malloc(sizeof(png_byte*) * h));
	for (int y = 0; y < h; y++, imageData += pitch)
		row_pointers[y] = imageData;

	// TODO: What format on big-endian?
	png_set_bgr(png_ptr);

	// Write the image data.
	png_write_rows(png_ptr, (png_bytepp)row_pointers, h);
	free(row_pointers);

	// Finished writing.
	png_write_end(png_ptr, info_ptr);
	png_destroy_write_struct(&png_ptr, &info_ptr);
	return 0;
#else
	// PNG support is not available.
	((void)image);
	return -EINVAL;
#endif
}

/** GcImageWriter **/

GcImageWriter::GcImageWriter()
	: d(new GcImageWriterPrivate(this))
{ }

GcImageWriter::~GcImageWriter()
	{ delete d; }

/**
 * Check if an image format is supported.
 * @param imgf Image format.
 * @return True if supported; false if not.
 */
bool GcImageWriter::isImageFormatSupported(ImageFormat imgf)
{
	switch (imgf) {
		case IMGF_PNG:	return true;
		default:	break;
	}

	return false;
}

/**
 * Get the file extension for the specified image format.
 * @param imgf Image format.
 * @return File extension (ASCII), without the dot, or nullptr if imgf is invalid.
 */
const char *GcImageWriter::extForImageFormat(ImageFormat imgf)
{
	switch (imgf) {
		case IMGF_PNG:	return "png";
		default:	break;
	}

	return nullptr;
}

/**
 * Check if an animated image format is supported.
 * @param animImgf Animated image format.
 * @return True if supported; false if not.
 */
bool GcImageWriter::isAnimImageFormatSupported(AnimImageFormat animImgf)
{
	switch (animImgf) {
		default:	break;
	}

	return false;
}

/**
 * Get the file extension for the specified animated image format.
 * @param animImgf Animated image format.
 * @return File extension (ASCII), without the dot, or nullptr if animImgf is invalid.
 */
const char *GcImageWriter::extForAnimImageFormat(AnimImageFormat animImgf)
{
	switch (animImgf) {
		case ANIMGF_APNG:
		case ANIMGF_PNG_FPF:
		case ANIMGF_PNG_VS:
		case ANIMGF_PNG_HS:	return "png";
		case ANIMGF_GIF:	return "gif";
		default:		break;
	}

	return nullptr;
}

/**
 * Write a GcImage to the internal memory buffer.
 * @param gcImage	[in] GcImage.
 * @param imgf		[in] Image format.
 * @return 0 on success; non-zero on error.
 */
int GcImageWriter::write(const GcImage *image, ImageFormat imgf)
{
	switch (imgf) {
		case IMGF_PNG:
			return d->writePng(image);

		default:
			break;
	}

	// Invalid image format.
	return -EINVAL;
}

/**
 * Get the internal memory buffer.
 * @return Internal memory buffer.
 */
const std::vector<uint8_t> *GcImageWriter::memBuffer(void) const
{
	return &d->memBuffer;
}
