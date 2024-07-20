/***************************************************************************
 * GameCube Tools Library.                                                 *
 * GcImageWriter_GIF.cpp: GameCube image writer. (GIF functions)           *
 *                                                                         *
 * Copyright (c) 2012-2024 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#include <config.libgctools.h>

#include "GcImageWriter.hpp"
#include "GcImageWriter_p.hpp"
#include "GcImage.hpp"

// C includes.
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>

// C++ includes.
#include <memory>
#include <vector>
using std::unique_ptr;
using std::vector;

#ifdef USE_INTERNAL_GIF
// giflib-5.2.1 declares GifQuantizeBuffer in getarg.h.
#include "getarg.h"
#else /* !USE_INTERNAL_GIF */
// giflib-4.2 doesn't have QuantizeBuffer().
// To prevent issues, we're including our own copy of
// giflib-5.2.1's GifQuantizeBuffer() if we're not
// using an internal copy of giflib.
extern "C"
int gcn_GifQuantizeBuffer(unsigned int Width, unsigned int Height,
                   int *ColorMapSize, GifByteType * RedInput,
                   GifByteType * GreenInput, GifByteType * BlueInput,
                   GifByteType * OutputBuffer,
                   GifColorType * OutputColorMap);
#define GifQuantizeBuffer(Width, Height, ColorMapSize, RedInput, GreenInput, BlueInput, OutputBuffer, OutputColorMap) \
	gcn_GifQuantizeBuffer((Width), (Height), (ColorMapSize), (RedInput), (GreenInput), (BlueInput), (OutputBuffer), (OutputColorMap))
#endif /* !USE_INTERNAL_GIF */

/**
 * GIF write function.
 * @param gif	[in] GifFileType pointer.
 * @param buf	[in] Data to write.
 * @param len	[in] Size of buf.
 * @return Number of bytes written.
 */
int GcImageWriterPrivate::gif_output_func(GifFileType *gif, const GifByteType *buf, int len)
{
	void *userData = GifDlGetUserData(gif);
	if (!userData || len <= 0)
		return 0;

	// Assuming the UserData is a vector<uint8_t>*.
	vector<uint8_t> *gifBuffer = static_cast<vector<uint8_t>*>(userData);
	size_t pos = gifBuffer->size();
	gifBuffer->resize(pos + len);
	memcpy(&gifBuffer->data()[pos], buf, len);
	return len;
}

/**
 * Convert a GcImage palette to a GIF palette.
 * @param colorMap	[out] GIF ColorMapObject.
 * @param palette	[in] GcImage palette. (must have 256 entries)
 * @return Index of transparent color, or -1 if no transparent color.
 */
int GcImageWriterPrivate::paletteToGifColorMap(ColorMapObject *colorMap, const uint32_t *palette)
{
	int trans_idx = -1;
	// TODO: Verify the size of colorMap?
	GifColorType *color = GifDlGetColorMapArray(colorMap);
	for (int i = 0; i < 256; i++, color++, palette++) {
		color->Red   = ((*palette >> 16) & 0xFF);
		color->Green = ((*palette >>  8) & 0xFF);
		color->Blue  = ( *palette        & 0xFF);

		// Check for full transparency.
		// NOTE: GIF doesn't support alpha-transparency;
		// semi-transparent pixels will be opaque.
		// TODO: If multiple entries are fully-transparent,
		// convert them all to the first entry.
		if (((*palette >> 24) & 0xFF) == 0) {
			// Color is fully transparent.
			if (trans_idx < 0) {
				trans_idx = i;
			}
		}
	}

	// Return the index of the first fully-transparent color.
	return trans_idx;
}

/**
 * Add a loop extension block to the GIF image.
 * @param gif		[in] GIF image.
 * @param loopCount	[in] Loop count. (0 == infinite)
 * @return GIF_OK on success; GIF_ERROR on error.
 */
int GcImageWriterPrivate::gif_addLoopExtension(GifFileType *gif, uint16_t loopCount)
{
	static const char nsle[] = "NETSCAPE2.0";
	int ret;

	// Loop block data:
	// - 0: '1' for Netscape looping extension.
	// - 1, 2: Loop count. (16LE; 0 == infinite)
	uint8_t loopData[3];
	loopData[0] = 1;			// Netscape looping extension.
	loopData[1] = loopCount & 0xFF;		// loopCount LSB
	loopData[2] = (loopCount >> 8) & 0xFF;	// loopCount MSB

	// Start an extension block.
	ret = EGifDlPutExtensionLeader(gif, APPLICATION_EXT_FUNC_CODE);
	if (ret != GIF_OK) {
		// Failed to start the extension block.
		return ret;
	}

	// Netscape loop extension.
	ret = EGifDlPutExtensionBlock(gif, sizeof(nsle)-1, nsle);
	if (ret != GIF_OK) {
		// Failed to add the extension block.
		return ret;
	}

	// Write the loop extension data.
	ret = EGifDlPutExtensionBlock(gif, sizeof(loopData), loopData);
	if (ret != GIF_OK) {
		// Failed to write the loop block data.
		return ret;
	}

	// Finish the extension block.
	ret = EGifDlPutExtensionTrailer(gif);

	// Done adding the loop extension block.
	return ret;
}

/**
 * Add a graphics control block to a GIF frame.
 * @param gif		[in] GIF image.
 * @param trans_idx	[in] Transparent color index. (-1 for no transparency)
 * @param iconDelay	[in] Icon delay, in centiseconds.
 * @return GIF_OK on success; GIF_ERROR on error.
 */
int GcImageWriterPrivate::gif_addGraphicsControlBlock(GifFileType *gif, int trans_idx, uint16_t iconDelay)
{
	/**
	 * Graphics control block.
	 * Byte 0: Bitfield:
	 * - Bit 0: Transparent flag (1 if a transparent color is present)
	 * - Bit 1: User Input flag (1 if it should wait for user input before showing the next frame)
	 * - Bits 2-4: Disposal method (0 = undef; 1 = leave in place; 2 = bg color; 3 = prev state)
	 * - Bits 5-7: Reserved
	 * Bytes 1-2: Delay. (16LE; centiseconds)
	 * Byte 4: Transparent color index.
	 */

	// TODO: Use EGifGCBToSavedExtension() in giflib-5.x.
	// This requires the image to be written first...
	uint8_t animctrl[4];
	if (trans_idx >= 0) {
		animctrl[0] = 1;
		animctrl[3] = trans_idx & 0xFF;
	} else {
		animctrl[0] = 0;
		animctrl[3] = 0xFF;
	}

	// Icon delay.
	animctrl[1] = iconDelay & 0xFF;
	animctrl[2] = (iconDelay >> 8) & 0xFF;

	return EGifDlPutExtension(gif, GRAPHICS_EXT_FUNC_CODE, sizeof(animctrl), animctrl);
}

/**
 * Write an ARGB32 image to a GIF.
 * This will reduce the image to 256 colors first.
 * @param gif		[in] GIF image.
 * @param gcImage	[in] GcImage to write.
 * @param colorMap	[in] Color map object to use.
 * @return GIF_OK on success; GIF_ERROR on error.
 */
int GcImageWriterPrivate::gif_writeARGB32Image(GifFileType *gif,
		const GcImage *gcImage, ColorMapObject *colorMap)
{
	// Split the image into separate Red/Green/Blue buffers.
	// TODO: Transparency?
	const size_t bufSz = gcImage->width() * gcImage->height();
	const size_t fullBufSz = bufSz * 4;
	unique_ptr<GifByteType[]> full(new GifByteType[fullBufSz]);
	GifByteType *red = full.get();
	GifByteType *green = red + bufSz;
	GifByteType *blue = green + bufSz;
	GifByteType *out = blue + bufSz;

	const uint32_t *src = (const uint32_t*)gcImage->imageData();
	for (size_t i = bufSz; i > 0; i--, src++) {
		*red++   = ((*src >> 16) & 0xFF);
		*green++ = ((*src >>  8) & 0xFF);
		*blue++  = ( *src        & 0xFF);
	}

	// Reset the buffer pointers.
	red = full.get();
	green = red + bufSz;
	blue = green + bufSz;

	// Quantize the image buffer.
	int colorCount = 256;
	GifDlSetColorMapCount(colorMap, colorCount);
	int ret = GifQuantizeBuffer(gcImage->width(), gcImage->height(),
			&colorCount, red, green, blue, out,
			GifDlGetColorMapArray(colorMap));
	GifDlSetColorMapCount(colorMap, colorCount);
	if (ret != GIF_OK) {
		// Error!
		return ret;
	}

	// Start the frame.
	ret = EGifDlPutImageDesc(gif, 0, 0, gcImage->width(), gcImage->height(), false, colorMap);
	if (ret != GIF_OK) {
		// Error!
		return ret;
	}

	// Write the entire image.
	ret = EGifDlPutLine(gif, out, bufSz);
	if (ret != GIF_OK) {
		// Error!
		return ret;
	}

	// Image written.
	return GIF_OK;
}

/**
 * Write an animated GcImage to the internal memory buffer in some GIF format.
 * @param gcImages	[in] Vector of GcImage.
 * @param gcIconDelays	[in] Icon delays.
 * @return 0 on success; non-zero on error.
 */
int GcImageWriterPrivate::writeGif_anim(const vector<const GcImage*> *gcImages,
					const vector<int> *gcIconDelays)
{
	// Make sure giflib is usable.
	if (GifDlVersion() == 0) {
		// giflib is NOT usable.
		return 0;
	}

	// All frames should be the same size.
	const GcImage *gcImage0 = gcImages->at(0);
	const int w = gcImage0->width();
	const int h = gcImage0->height();

	// Color Map object for palettes.
	ColorMapObject *colorMap = GifDlMakeMapObject(256, nullptr);
	if (!colorMap) {
		// Error allocating a color map.
		return -1;
	}

	bool is_CI8_UNIQUE = false;
	if (gcImage0->pxFmt() == GcImage::PXFMT_CI8) {
		// May be CI8 or CI8_UNIQUE.
		is_CI8_UNIQUE = is_gcImages_CI8_UNIQUE(gcImages);
		if (!is_CI8_UNIQUE) {
			// Convert the palette from the first frame.
			paletteToGifColorMap(colorMap, gcImage0->palette());
		}
	}

	// Initialize the internal buffer.
	vector<uint8_t> *gifBuffer = new vector<uint8_t>();
	gifBuffer->reserve(32768);	// 32 KB should cover most of the use cases.

	// TODO: Make use of the giflib error code.
	int err = GIF_OK;
	GifFileType *gif = EGifDlOpen(gifBuffer, gif_output_func, &err);
	if (!gif) {
		// Error!
		delete gifBuffer;
		GifDlFreeMapObject(colorMap);
		return -1;
	}

	// Set the GIF version to GIF89a.
	// (Required for animated GIFs.)
	// NOTE: giflib-4.2+ is supposed to automatically set
	// this based on the written extension blocks, but it
	// doesn't seem to be working...
	EGifDlSetGifVersion(gif, true);

	// Put the screen description for the first frame.
	// NOTE: colorMap is only specified if the image
	// uses a global palette. For CI8_UNIQUE, each
	// frame will have its own local palette.
	if (EGifDlPutScreenDesc(gif, w, h, 8, 0, (is_CI8_UNIQUE ? nullptr : colorMap)) != GIF_OK) {
		// Error!
		EGifDlCloseFile(gif, &err);
		delete gifBuffer;
		GifDlFreeMapObject(colorMap);
		return -2;
	}

	// Add the loop extension block.
	if (gif_addLoopExtension(gif, 0) != GIF_OK) {
		// Error!
		EGifDlCloseFile(gif, &err);
		delete gifBuffer;
		GifDlFreeMapObject(colorMap);
		return -3;
	}

	// Write the frames.
	for (int i = 0; i < (int)gcImages->size(); i++) {
		// NOTE: NULL images should be removed by write().
		const GcImage *gcImage = gcImages->at(i);

		// NOTE: Icon delay is in units of 4 NTSC frames.
		// FIXME: Support PAL; handle extra beginning frame and reduced ending frame.
		const float fIconDelay = (float)(gcIconDelays->at(i) * 4 * 100) / 60.0f;
		const uint16_t uIconDelay = (uint16_t)fIconDelay;

		// Graphics control block.
		// TODO: Transparent color index.
		if (gif_addGraphicsControlBlock(gif, -1, uIconDelay) != GIF_OK) {
			// Error!
			EGifDlCloseFile(gif, &err);
			delete gifBuffer;
			GifDlFreeMapObject(colorMap);
			return -5;
		}

		if (is_CI8_UNIQUE) {
			// Update the ColorMap for this frame.
			paletteToGifColorMap(colorMap, gcImage->palette());
		}

		switch (gcImage->pxFmt()) {
			case GcImage::PXFMT_CI8:
				// Start the frame.
				if (EGifDlPutImageDesc(gif, 0, 0, w, h, false,
				    (is_CI8_UNIQUE ? colorMap : nullptr)) != GIF_OK)
				{
					// Error!
					EGifDlCloseFile(gif, &err);
					delete gifBuffer;
					GifDlFreeMapObject(colorMap);
					return -6;
				}

				// Write the entire image.
				if (EGifDlPutLine(gif, (uint8_t*)gcImage->imageData(),
				    gcImage->imageData_len()) != GIF_OK)
				{
					// Error!
					EGifDlCloseFile(gif, &err);
					delete gifBuffer;
					GifDlFreeMapObject(colorMap);
					return -7;
				}
				break;

			case GcImage::PXFMT_ARGB32:
				// Reduce the image to 256 colors.
				// TODO: Use a similar palette for all images?
				// Otherwise it might look weird...
				if (gif_writeARGB32Image(gif, gcImage, colorMap) != GIF_OK) {
					// Error!
					EGifDlCloseFile(gif, &err);
					delete gifBuffer;
					GifDlFreeMapObject(colorMap);
					return -8;
				}
				break;

			default:
				// Unsupported pixel format.
				delete gifBuffer;
				GifDlFreeMapObject(colorMap);
				return -9;
		}
	}

	GifDlFreeMapObject(colorMap);
	EGifDlCloseFile(gif, &err);

	// Add the gifBuffer to the memBuffer.
	memBuffer.push_back(gifBuffer);
	return 0;
}
