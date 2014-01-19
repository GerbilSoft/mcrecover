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
		 * Write a GcImage to the internal memory buffer in PNG format.
		 * @param gcImage	[in] GcImage.
		 * @return 0 on success; non-zero on error.
		 */
		int writePng(const GcImage *gcImage);

		/**
		 * Write an animated GcImage to the internal memory buffer in APNG format.
		 * @param gcImages	[in] Vector of GcImage.
		 * @param gcIconDelays	[in] Icon delays.
		 * @return 0 on success; non-zero on error.
		 */
		int writeAPng(const vector<const GcImage*> *gcImages, const vector<int> *gcIconDelays);
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
 * Write a PLTE chunk to a PNG image.
 * @param png_ptr PNG pointer.
 * @param info_ptr PNG info pointer.
 * @param palette Palette. (ARGB32 format)
 * @param num_entries Number of palette entries.
 * @return 0 on success; non-zero on error.
 */
int GcImageWriterPrivate::writePng_PLTE(
		png_structp png_ptr, png_infop info_ptr,
		const uint32_t *palette, int num_entries)
{
	if (num_entries < 0 || num_entries > 256)
		return -1;

	// Maximum size.
	png_color png_pal[256];
	uint8_t png_tRNS[256];

	// Convert the palette.
	for (int i = 0; i < num_entries; i++) {
		png_pal[i].red   = ((palette[i] >> 16) & 0xFF);
		png_pal[i].green = ((palette[i] >> 8) & 0xFF);
		png_pal[i].blue  = (palette[i] & 0xFF);
		png_tRNS[i]      = ((palette[i] >> 24) & 0xFF);
	}

	// Write the PLTE and tRNS chunks.
	png_set_PLTE(png_ptr, info_ptr, png_pal, num_entries);
	png_set_tRNS(png_ptr, info_ptr, png_tRNS, num_entries, nullptr);
	return 0;
}

/**
 * Write a GcImage to the internal memory buffer in PNG format.
 * @param gcImage	[in] GcImage.
 * @return 0 on success; non-zero on error.
 */
int GcImageWriterPrivate::writePng(const GcImage *gcImage)
{
	// Clear the internal memory buffer.
	memBuffer.clear();

#ifdef HAVE_PNG
	if (!gcImage)
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

	const int w = gcImage->width();
	const int h = gcImage->height();

	// Write the PNG header.
	switch (gcImage->pxFmt()) {
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
			writePng_PLTE(png_ptr, info_ptr, gcImage->palette(), 256);
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
	// TODO: What format on big-endian?
	png_set_bgr(png_ptr);

	// Calculate the row pointers.
	int pitch;
	switch (gcImage->pxFmt()) {
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

	const uint8_t *imageData = (const uint8_t*)gcImage->imageData();
	vector<const uint8_t*> row_pointers;
	row_pointers.resize(h);
	for (int y = 0; y < h; y++, imageData += pitch)
		row_pointers[y] = imageData;

	// Write the image data.
	png_write_image(png_ptr, (png_bytepp)row_pointers.data());

	// Finished writing.
	png_write_end(png_ptr, info_ptr);
	png_destroy_write_struct(&png_ptr, &info_ptr);
	return 0;
#else
	// PNG support is not available.
	((void)gcImage);
	return -EINVAL;
#endif
}

/**
 * Write an animated GcImage to the internal memory buffer in APNG format.
 * @param gcImages	[in] Vector of GcImage.
 * @param gcIconDelays	[in] Icon delays.
 * TODO: Add icon speeds.
 * @return 0 on success; non-zero on error.
 */
int GcImageWriterPrivate::writeAPng(const vector<const GcImage*> *gcImages, const vector<int> *gcIconDelays)
{
	// TODO
	((void)gcIconDelays);

	// Clear the internal memory buffer.
	memBuffer.clear();

#if defined(HAVE_PNG) && defined(HAVE_PNG_APNG)
	if (!gcImages || gcImages->empty() || !gcImages->at(0))
		return -EINVAL;

	// Adjust icon delays for NULL images.
	// NOTE: Assuming image 0 is always valid.
	int totalImages = 1; // Total number of non-NULL images.
	// Icon delays with NULL adjustments.
	vector<int> adjIconDelays = *gcIconDelays;

	// Verify that all icons have the correct parameters.
	const GcImage *gcImage0 = gcImages->at(0);
	const int w = gcImage0->width();
	const int h = gcImage0->height();
	const GcImage::PxFmt pxFmt = gcImage0->pxFmt();
	for (int i = 1, lastNonNullIdx = 0; i < (int)gcImages->size(); i++) {
		const GcImage *gcImageN = gcImages->at(i);
		if (!gcImageN) {
			// NULL image.
			// Assume it's the same as the previous image.
			adjIconDelays[lastNonNullIdx] += gcIconDelays->at(i);
			continue;
		}

		// This icon is not NULL.
		lastNonNullIdx = i;
		totalImages++;

		if (gcImageN->width() != w ||
		    gcImageN->height() != h ||
		    gcImageN->pxFmt() != pxFmt)
		{
			// Animated icon is invalid.
			// TODO: Use a better error code.
			return -EINVAL;
		}
	}

	// NOTE: APNG only supports a single palette.
	// If the icon is CI8_UNIQUE, it will need to be
	// converted to ARGB32.
	// TODO: Test this; I don't have any files with CI8_UNIQUE...
	vector<const GcImage*> gcImagesARGB32;
	bool is_CI8_UNIQUE = false;
	if (pxFmt == GcImage::PXFMT_CI8) {
		const uint32_t *palette0 = gcImage0->palette();
		for (int i = 1; i < (int)gcImages->size(); i++) {
			const GcImage *gcImageN = gcImages->at(i);
			if (gcImageN) {
				const uint32_t *paletteN = gcImageN->palette();
				if (memcmp(palette0, paletteN, (256*sizeof(*paletteN))) != 0) {
					// CI8_UNIQUE.
					is_CI8_UNIQUE = true;
					break;
				}
			}
		}
	}

	if (is_CI8_UNIQUE) {
		// CI8_UNIQUE. Convert to ARGB32.
		gcImagesARGB32.resize(gcImages->size());
		for (int i = 0; i < (int)gcImages->size(); i++) {
			const GcImage *gcImageN = gcImages->at(i);
			if (gcImageN)
				gcImageN = gcImageN->toRGB5A3();
			gcImagesARGB32[i] = gcImageN;
		}

		// Use the converted images.
		gcImages = &gcImagesARGB32;
	}

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

	// Write the PNG header.
	switch (pxFmt) {
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
			writePng_PLTE(png_ptr, info_ptr, gcImage0->palette(), 256);
			break;
		}

		default:
			// Unsupported pixel format.
			png_destroy_write_struct(&png_ptr, (png_infopp)nullptr);
			     memBuffer.clear();
			return -EINVAL;
	}

	// Write an acTL to indicate that this is an APNG.
	// NOTE: totalImages excludes NULL images.
	png_set_acTL(png_ptr, info_ptr, totalImages, 0);

	// Write the PNG information to the file.
	png_write_info(png_ptr, info_ptr);

	// TODO: Byteswap image data on big-endian systems?
	//ppng_set_swap(png_ptr);
	// TODO: What format on big-endian?
	png_set_bgr(png_ptr);

	// Initialize the row pointers.
	vector<const uint8_t*> row_pointers;
	row_pointers.resize(h);

	for (int i = 0; i < (int)gcImages->size(); i++) {
		const GcImage *gcImage = gcImages->at(i);
		if (!gcImage)
			continue;

		// NOTE: Icon delay is in units of 8 NTSC frames.
		const int iconDelay = (adjIconDelays[i] * 8);
		static const int iconDelayDenom = 60;

		// Calculate the row pointers.
		int pitch;
		switch (pxFmt) {
			case GcImage::PXFMT_ARGB32:
				pitch = (w * 4);
				break;

			case GcImage::PXFMT_CI8: {
				pitch = w;
				break;
			}

			default:
				// Unsupported pixel format.
				png_destroy_write_struct(&png_ptr, (png_infopp)nullptr);
				memBuffer.clear();
				return -EINVAL;
		}

		const uint8_t *imageData = (const uint8_t*)gcImage->imageData();
		for (int y = 0; y < h; y++, imageData += pitch)
			row_pointers[y] = imageData;

		// Frame header.
		png_write_frame_head(png_ptr, info_ptr, (png_bytepp)row_pointers.data(),
				w, h, 0, 0,			// width, height, x offset, y offset
				iconDelay, iconDelayDenom,	// delay numerator and denominator
				PNG_DISPOSE_OP_NONE,
				PNG_BLEND_OP_SOURCE);

		// Write the image data.
		png_write_image(png_ptr, (png_bytepp)row_pointers.data());

		// Frame tail.
		png_write_frame_tail(png_ptr, info_ptr);
	}

	// Finished writing.
	png_write_end(png_ptr, info_ptr);
	png_destroy_write_struct(&png_ptr, &info_ptr);

	// Check if we had to convert any icons to ARGB32.
	if (!gcImagesARGB32.empty()) {
		for (int i = 0; i < (int)gcImagesARGB32.size(); i++) {
			delete const_cast<GcImage*>(gcImagesARGB32[i]);
		}
		gcImagesARGB32.clear();
	}

	return 0;
#else
	// PNG support is not available.
	((void)gcImage);
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
		case IMGF_PNG:
#ifdef HAVE_PNG
			return true;
#else
			return false;
#endif

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
		case ANIMGF_APNG:
			// TODO: Make APNG dlopen()able.
#if defined(HAVE_PNG) && defined(HAVE_PNG_APNG)
			return true;
#else
			return false;
#endif

		case ANIMGF_GIF:
#ifdef HAVE_GIF
			return true;
#else
			return false;
#endif

		case ANIMGF_PNG_FPF:
		case ANIMGF_PNG_VS:
		case ANIMGF_PNG_HS:
			// TODO
			return false;

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
 * Get the internal memory buffer.
 * @return Internal memory buffer.
 */
const vector<uint8_t> *GcImageWriter::memBuffer(void) const
{
	return &d->memBuffer;
}

/**
 * Write a GcImage to the internal memory buffer.
 * @param gcImage	[in] GcImage.
 * @param imgf		[in] Image format.
 * @return 0 on success; non-zero on error.
 */
int GcImageWriter::write(const GcImage *gcImage, ImageFormat imgf)
{
	switch (imgf) {
		case IMGF_PNG:
			return d->writePng(gcImage);

		default:
			break;
	}

	// Invalid image format.
	return -EINVAL;
}

/**
 * Write an animated GcImage to the internal memory buffer.
 * @param gcImages	[in] Vector of GcImage.
 * @param gcIconDelays	[in] Icon delays.
 * @param animImgf	[in] Animated image format.
 * @return 0 on success; non-zero on error.
 */
int GcImageWriter::write(const vector<const GcImage*> *gcImages,
			 const vector<int> *gcIconDelays,
			 AnimImageFormat animImgf)
{
	switch (animImgf) {
		case ANIMGF_APNG:
			return d->writeAPng(gcImages, gcIconDelays);

		default:
			break;
	}

	// Invalid image format.
	return -EINVAL;
}
