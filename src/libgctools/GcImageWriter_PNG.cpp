/***************************************************************************
 * GameCube Tools Library.                                                 *
 * GcImageWriter_PNG.cpp: GameCube image writer. (PNG functions)           *
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

#include <config.libgctools.h>

#ifndef HAVE_PNG
#error GcImageWriter_PNG.cpp should only be compiled if libpng is available.
#endif

#include "GcImageWriter.hpp"
#include "GcImageWriter_p.hpp"
#include "GcImage.hpp"

// C includes.
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>

// C++ includes.
#include <vector>
using std::vector;

/**
 * PNG write function.
 * @param png_ptr	[in] PNG pointer.
 * @param buf		[in] Data to write.
 * @param len		[in] Size of buf.
 */
void GcImageWriterPrivate::png_io_write(png_structp png_ptr, png_bytep buf, png_size_t len)
{
	void *io_ptr = png_get_io_ptr(png_ptr);
	if (!io_ptr || len == 0)
		return;

	// Assuming the io_ptr is a vector<uint8_t>*.
	vector<uint8_t> *pngBuffer = reinterpret_cast<vector<uint8_t>*>(io_ptr);
	size_t pos = pngBuffer->size();
	pngBuffer->resize(pos + len);
	memcpy(&pngBuffer->data()[pos], buf, len);
}

/**
 * PNG flush function.
 * Required when writing PNG images.
 * This implementation is a no-op.
 * @param png_ptr	[in] PNG pointer.
 */
void GcImageWriterPrivate::png_io_flush(png_structp png_ptr)
{
	// Do nothing!
	((void)png_ptr);
}

/**
 * Write a PLTE chunk to a PNG image.
 * @param png_ptr	[in] PNG pointer.
 * @param info_ptr	[in] PNG info pointer.
 * @param palette	[in] Palette. (ARGB32 format)
 * @param num_entries	[in] Number of palette entries.
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
		png_pal[i].blue  = ( palette[i]        & 0xFF);
		png_pal[i].green = ((palette[i] >> 8)  & 0xFF);
		png_pal[i].red   = ((palette[i] >> 16) & 0xFF);
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

	// Initialize the internal buffer.
	vector<uint8_t> *pngBuffer = new vector<uint8_t>();
	pngBuffer->reserve(32768);	// 32 KB should cover most of the use cases.
	vector<const uint8_t*> row_pointers;

	// WARNING: Do NOT initialize any C++ objects past this point!
#ifdef PNG_SETJMP_SUPPORTED
	if (setjmp(png_jmpbuf(png_ptr))) {
		// PNG write failed.
		png_destroy_write_struct(&png_ptr, &info_ptr);
		delete pngBuffer;
		return -0x103;
	}
#endif /* PNG_SETJMP_SUPPORTED */

	// Initialize the memory write function.
	png_set_write_fn(png_ptr, pngBuffer, png_io_write, png_io_flush);

	// Initialize compression parameters.
	png_set_filter(png_ptr, 0, PNG_FILTER_NONE);
	png_set_compression_level(png_ptr, 5);	// TODO: Customizable?

	const int w = gcImage->width();
	const int h = gcImage->height();

	// Write the PNG header.
	int pitch;
	switch (gcImage->pxFmt()) {
		case GcImage::PXFMT_ARGB32:
			png_set_IHDR(png_ptr, info_ptr, w, h,
					8, PNG_COLOR_TYPE_RGB_ALPHA,
					PNG_INTERLACE_NONE,
					PNG_COMPRESSION_TYPE_DEFAULT,
					PNG_FILTER_TYPE_DEFAULT);
			pitch = (w * 4);
			break;

		case GcImage::PXFMT_CI8: {
			png_set_IHDR(png_ptr, info_ptr, w, h,
					8, PNG_COLOR_TYPE_PALETTE,
					PNG_INTERLACE_NONE,
					PNG_COMPRESSION_TYPE_DEFAULT,
					PNG_FILTER_TYPE_DEFAULT);
			pitch = w;

			// Set the palette and tRNS values.
			writePng_PLTE(png_ptr, info_ptr, gcImage->palette(), 256);
			break;
		}

		default:
			// Unsupported pixel format.
			png_destroy_write_struct(&png_ptr, (png_infopp)nullptr);
			delete pngBuffer;
			return -EINVAL;
	}

	// Write the PNG information to the file.
	png_write_info(png_ptr, info_ptr);

	// TODO: Byteswap image data on big-endian systems?
	//ppng_set_swap(png_ptr);
	// TODO: What format on big-endian?
	png_set_bgr(png_ptr);

	// Calculate the row pointers.
	const uint8_t *imageData = (const uint8_t*)gcImage->imageData();
	row_pointers.resize(h);
	for (int y = 0; y < h; y++, imageData += pitch)
		row_pointers[y] = imageData;

	// Write the image data.
	png_write_image(png_ptr, (png_bytepp)row_pointers.data());

	// Finished writing.
	png_write_end(png_ptr, info_ptr);
	png_destroy_write_struct(&png_ptr, &info_ptr);

	// Add the pngBuffer to the memBuffer.
	memBuffer.push_back(pngBuffer);
	return 0;
}

/**
 * Write an animated GcImage to the internal memory buffer in APNG format.
 * @param gcImages	[in] Vector of GcImage.
 * @param gcIconDelays	[in] Icon delays.
 * @return 0 on success; non-zero on error.
 */
int GcImageWriterPrivate::writeAPng(const vector<const GcImage*> *gcImages, const vector<int> *gcIconDelays)
{
	if (!APNG_is_supported())
		return -ENOSYS;

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

	// Initialize the internal buffer.
	vector<uint8_t> *pngBuffer = new vector<uint8_t>();
	pngBuffer->reserve(32768);	// 32 KB should cover most of the use cases.
	vector<const uint8_t*> row_pointers;

	// WARNING: Do NOT initialize any C++ objects past this point!
#ifdef PNG_SETJMP_SUPPORTED
	if (setjmp(png_jmpbuf(png_ptr))) {
		// PNG write failed.
		png_destroy_write_struct(&png_ptr, &info_ptr);
		delete pngBuffer;
		return -0x103;
	}
#endif /* PNG_SETJMP_SUPPORTED */

	// Initialize the memory write function.
	png_set_write_fn(png_ptr, pngBuffer, png_io_write, png_io_flush);

	// Initialize compression parameters.
	png_set_filter(png_ptr, 0, PNG_FILTER_NONE);
	png_set_compression_level(png_ptr, 5);	// TODO: Customizable?

	const GcImage *gcImage0 = gcImages->at(0);
	const int w = gcImage0->width();
	const int h = gcImage0->height();
	const GcImage::PxFmt pxFmt = gcImage0->pxFmt();

	// Write the PNG header.
	int pitch;
	switch (pxFmt) {
		case GcImage::PXFMT_ARGB32:
			png_set_IHDR(png_ptr, info_ptr, w, h,
					8, PNG_COLOR_TYPE_RGB_ALPHA,
					PNG_INTERLACE_NONE,
					PNG_COMPRESSION_TYPE_DEFAULT,
					PNG_FILTER_TYPE_DEFAULT);
			pitch = (w * 4);
			break;

		case GcImage::PXFMT_CI8: {
			png_set_IHDR(png_ptr, info_ptr, w, h,
					8, PNG_COLOR_TYPE_PALETTE,
					PNG_INTERLACE_NONE,
					PNG_COMPRESSION_TYPE_DEFAULT,
					PNG_FILTER_TYPE_DEFAULT);
			pitch = w;

			// Set the palette and tRNS values.
			writePng_PLTE(png_ptr, info_ptr, gcImage0->palette(), 256);
			break;
		}

		default:
			// Unsupported pixel format.
			png_destroy_write_struct(&png_ptr, (png_infopp)nullptr);
			delete pngBuffer;
			return -EINVAL;
	}

	// Write an acTL to indicate that this is an APNG.
	APNG_png_set_acTL(png_ptr, info_ptr, gcImages->size(), 0);

	// Write the PNG information to the file.
	png_write_info(png_ptr, info_ptr);

	// TODO: Byteswap image data on big-endian systems?
	//ppng_set_swap(png_ptr);
	// TODO: What format on big-endian?
	png_set_bgr(png_ptr);

	// Initialize the row pointers.
	row_pointers.resize(h);
	for (int i = 0; i < (int)gcImages->size(); i++) {
		// NOTE: NULL images should be removed by write().
		const GcImage *gcImage = gcImages->at(i);

		// NOTE: Icon delay is in units of 8 NTSC frames.
		const uint16_t iconDelay = (uint16_t)(gcIconDelays->at(i) * 8);
		static const uint16_t iconDelayDenom = 60;

		// Calculate the row pointers.
		const uint8_t *imageData = (const uint8_t*)gcImage->imageData();
		for (int y = 0; y < h; y++, imageData += pitch)
			row_pointers[y] = imageData;

		// Frame header.
		APNG_png_write_frame_head(png_ptr, info_ptr, (png_bytepp)row_pointers.data(),
				w, h, 0, 0,			// width, height, x offset, y offset
				iconDelay, iconDelayDenom,	// delay numerator and denominator
				PNG_DISPOSE_OP_NONE,
				PNG_BLEND_OP_SOURCE);

		// Write the image data.
		png_write_image(png_ptr, (png_bytepp)row_pointers.data());

		// Frame tail.
		APNG_png_write_frame_tail(png_ptr, info_ptr);
	}

	// Finished writing.
	png_write_end(png_ptr, info_ptr);
	png_destroy_write_struct(&png_ptr, &info_ptr);

	// Add the pngBuffer to the memBuffer.
	memBuffer.push_back(pngBuffer);
	return 0;
}

/**
 * Write an animated GcImage to the internal memory buffer in PNG FPF format.
 * @param gcImages	[in] Vector of GcImage.
 * @return 0 on success; non-zero on error.
 */
int GcImageWriterPrivate::writePng_FPF(const vector<const GcImage*> *gcImages)
{
	// PNG FPF (file per frame) stores each frame
	// in its own PNG file.
	for (int i = 0; i < (int)gcImages->size(); i++) {
		int ret = writePng(gcImages->at(i));
		if (ret != 0)
			return ret;
	}
	return 0;
}

/**
 * Write an animated GcImage to the internal memory buffer in PNG VS format.
 * @param gcImages	[in] Vector of GcImage.
 * @return 0 on success; non-zero on error.
 */
int GcImageWriterPrivate::writePng_VS(const vector<const GcImage*> *gcImages)
{
	// PNG VS is a regular PNG with all frames
	// stored as a vertical strip.
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

	// Initialize the internal buffer.
	vector<uint8_t> *pngBuffer = new vector<uint8_t>();
	pngBuffer->reserve(32768);	// 32 KB should cover most of the use cases.
	vector<const uint8_t*> row_pointers;

	// WARNING: Do NOT initialize any C++ objects past this point!
#ifdef PNG_SETJMP_SUPPORTED
	if (setjmp(png_jmpbuf(png_ptr))) {
		// PNG write failed.
		png_destroy_write_struct(&png_ptr, &info_ptr);
		delete pngBuffer;
		return -0x103;
	}
#endif /* PNG_SETJMP_SUPPORTED */

	// Initialize the memory write function.
	png_set_write_fn(png_ptr, pngBuffer, png_io_write, png_io_flush);

	// Initialize compression parameters.
	png_set_filter(png_ptr, 0, PNG_FILTER_NONE);
	png_set_compression_level(png_ptr, 5);	// TODO: Customizable?

	const GcImage *gcImage0 = gcImages->at(0);
	const int w = gcImage0->width();
	const int h = gcImage0->height();
	const GcImage::PxFmt pxFmt = gcImage0->pxFmt();

	// Calculate vertical strip height.
	const int vs_h = (h * gcImages->size());

	// Write the PNG header.
	int pitch;
	switch (pxFmt) {
		case GcImage::PXFMT_ARGB32:
			png_set_IHDR(png_ptr, info_ptr, w, vs_h,
					8, PNG_COLOR_TYPE_RGB_ALPHA,
					PNG_INTERLACE_NONE,
					PNG_COMPRESSION_TYPE_DEFAULT,
					PNG_FILTER_TYPE_DEFAULT);
			pitch = (w * 4);
			break;

		case GcImage::PXFMT_CI8: {
			png_set_IHDR(png_ptr, info_ptr, w, vs_h,
					8, PNG_COLOR_TYPE_PALETTE,
					PNG_INTERLACE_NONE,
					PNG_COMPRESSION_TYPE_DEFAULT,
					PNG_FILTER_TYPE_DEFAULT);
			pitch = w;

			// Set the palette and tRNS values.
			writePng_PLTE(png_ptr, info_ptr, gcImage0->palette(), 256);
			break;
		}

		default:
			// Unsupported pixel format.
			png_destroy_write_struct(&png_ptr, (png_infopp)nullptr);
			delete pngBuffer;
			return -EINVAL;
	}

	// Write the PNG information to the file.
	png_write_info(png_ptr, info_ptr);

	// TODO: Byteswap image data on big-endian systems?
	//ppng_set_swap(png_ptr);
	// TODO: What format on big-endian?
	png_set_bgr(png_ptr);

	// Initialize the row pointers.
	row_pointers.resize(vs_h);
	// Append each image to the PNG row pointer data, vertically.
	for (int i = 0, vs_y = 0; i < (int)gcImages->size(); i++, vs_y += h) {
		// NOTE: NULL images should be removed by write().
		const GcImage *gcImage = gcImages->at(i);

		// Calculate the row pointers.
		const uint8_t *imageData = (const uint8_t*)gcImage->imageData();
		for (int y = 0; y < h; y++, imageData += pitch)
			row_pointers[vs_y + y] = imageData;
	}

	// Write the image data.
	png_write_image(png_ptr, (png_bytepp)row_pointers.data());

	// Finished writing.
	png_write_end(png_ptr, info_ptr);
	png_destroy_write_struct(&png_ptr, &info_ptr);

	// Add the pngBuffer to the memBuffer.
	memBuffer.push_back(pngBuffer);
	return 0;
}

/**
 * Write an animated GcImage to the internal memory buffer in PNG HS format.
 * @param gcImages	[in] Vector of GcImage.
 * @return 0 on success; non-zero on error.
 */
int GcImageWriterPrivate::writePng_HS(const vector<const GcImage*> *gcImages)
{
	// PNG VS is a regular PNG with all frames
	// stored as a horizontal strip.
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

	// Initialize the internal buffer.
	vector<uint8_t> *pngBuffer = new vector<uint8_t>();
	pngBuffer->reserve(32768);	// 32 KB should cover most of the use cases.
	vector<uint8_t> imgBuf;		// Temporary image buffer.
	vector<const uint8_t*> row_pointers;

	// WARNING: Do NOT initialize any C++ objects past this point!
#ifdef PNG_SETJMP_SUPPORTED
	if (setjmp(png_jmpbuf(png_ptr))) {
		// PNG write failed.
		png_destroy_write_struct(&png_ptr, &info_ptr);
		delete pngBuffer;
		return -0x103;
	}
#endif /* PNG_SETJMP_SUPPORTED */

	// Initialize the memory write function.
	png_set_write_fn(png_ptr, pngBuffer, png_io_write, png_io_flush);

	// Initialize compression parameters.
	png_set_filter(png_ptr, 0, PNG_FILTER_NONE);
	png_set_compression_level(png_ptr, 5);	// TODO: Customizable?

	const GcImage *gcImage0 = gcImages->at(0);
	const int w = gcImage0->width();
	const int h = gcImage0->height();
	const GcImage::PxFmt pxFmt = gcImage0->pxFmt();

	// Calculate vertical strip width.
	const int vs_w = (w * gcImages->size());

	// Write the PNG header.
	int pitch;
	switch (pxFmt) {
		case GcImage::PXFMT_ARGB32:
			png_set_IHDR(png_ptr, info_ptr, vs_w, h,
					8, PNG_COLOR_TYPE_RGB_ALPHA,
					PNG_INTERLACE_NONE,
					PNG_COMPRESSION_TYPE_DEFAULT,
					PNG_FILTER_TYPE_DEFAULT);
			pitch = (w * 4);
			break;

		case GcImage::PXFMT_CI8: {
			png_set_IHDR(png_ptr, info_ptr, vs_w, h,
					8, PNG_COLOR_TYPE_PALETTE,
					PNG_INTERLACE_NONE,
					PNG_COMPRESSION_TYPE_DEFAULT,
					PNG_FILTER_TYPE_DEFAULT);
			pitch = w;

			// Set the palette and tRNS values.
			writePng_PLTE(png_ptr, info_ptr, gcImage0->palette(), 256);
			break;
		}

		default:
			// Unsupported pixel format.
			png_destroy_write_struct(&png_ptr, (png_infopp)nullptr);
			delete pngBuffer;
			return -EINVAL;
	}

	// Write the PNG information to the file.
	png_write_info(png_ptr, info_ptr);

	// TODO: Byteswap image data on big-endian systems?
	//ppng_set_swap(png_ptr);
	// TODO: What format on big-endian?
	png_set_bgr(png_ptr);

	// Create a temporary buffer for the horizontal image.
	const int vs_pitch = (pitch * gcImages->size());
	imgBuf.resize(vs_pitch * h);

	// Initialize the row pointers.
	row_pointers.resize(h);
	for (int y = 0, pos = 0; y < h; y++, pos += vs_pitch) {
		row_pointers[y] = (imgBuf.data() + pos);
	}

	// Append each image to the PNG row pointer data, horizontally.
	for (int i = 0; i < (int)gcImages->size(); i++) {
		// NOTE: NULL images should be removed by write().
		const GcImage *gcImage = gcImages->at(i);
		uint8_t *pos = imgBuf.data() + (pitch * i);

		// Calculate the row pointers.
		const uint8_t *imageData = (const uint8_t*)gcImage->imageData();
		for (int y = 0; y < h; y++, pos += vs_pitch, imageData += pitch)
			memcpy(pos, imageData, pitch);
	}

	// Write the image data.
	png_write_image(png_ptr, (png_bytepp)row_pointers.data());

	// Finished writing.
	png_write_end(png_ptr, info_ptr);
	png_destroy_write_struct(&png_ptr, &info_ptr);

	// Add the pngBuffer to the memBuffer.
	memBuffer.push_back(pngBuffer);
	return 0;
}

/**
 * Write an animated GcImage to the internal memory buffer in some PNG format.
 * @param gcImages	[in] Vector of GcImage.
 * @param gcIconDelays	[in] Icon delays.
 * @param animImgf	[in] Animated image format.
 * @return 0 on success; non-zero on error.
 */
int GcImageWriterPrivate::writePng_anim(const vector<const GcImage*> *gcImages,
					const vector<int> *gcIconDelays,
					GcImageWriter::AnimImageFormat animImgf)
{
	// NOTE: This has to be a separate function because
	// setjmp() in the writePng functions might clobber
	// the allocated vector for CI8_UNIQUE conversion.

	switch (animImgf) {
		case GcImageWriter::ANIMGF_PNG_FPF:
			// No image conversion necessary.
			return writePng_FPF(gcImages);
		case GcImageWriter::ANIMGF_APNG:
		case GcImageWriter::ANIMGF_PNG_HS:
		case GcImageWriter::ANIMGF_PNG_VS:
			// Image conversion is necessary.
			break;
		default:
			// Invalid image format.
			return -EINVAL;
	}

	// NOTE: APNG only supports a single palette.
	// If the icon is CI8_UNIQUE, it will need to be
	// converted to ARGB32.
	// TODO: Test this; I don't have any files with CI8_UNIQUE...
	vector<const GcImage*> *gcImagesARGB32 = gcImages_from_CI8_UNIQUE(gcImages);
	if (gcImagesARGB32) {
		gcImages = gcImagesARGB32;
	}

	int ret;
	switch (animImgf) {
		case GcImageWriter::ANIMGF_APNG:
			ret = writeAPng(gcImages, gcIconDelays);
			break;
		case GcImageWriter::ANIMGF_PNG_HS:
			ret = writePng_HS(gcImages);
			break;
		case GcImageWriter::ANIMGF_PNG_VS:
			ret = writePng_VS(gcImages);
			break;
		default:
			ret = -EINVAL;
			break;
	}

	// Check if we had to convert any icons to ARGB32.
	if (gcImagesARGB32) {
		for (int i = 0; i < (int)gcImagesARGB32->size(); i++) {
			delete const_cast<GcImage*>(gcImagesARGB32->at(i));
		}
		delete gcImagesARGB32;
	}

	return ret;
}
