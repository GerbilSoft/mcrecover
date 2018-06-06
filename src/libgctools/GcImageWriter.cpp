/***************************************************************************
 * GameCube Tools Library.                                                 *
 * GcImageWriter.cpp: GameCube image writer.                               *
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

#include "GcImageWriter.hpp"
#include "GcImageWriter_p.hpp"
#include "GcImage.hpp"

// GIF dlopen() wrapper.
#ifdef USE_GIF
#include "GIF_dlopen.h"
#endif /* USE_GIF */

// C includes.
#include <stdint.h>
#include <stdlib.h>

// C includes. (C++ namespace)
#include <cassert>
#include <cerrno>

// C++ includes.
#include <vector>
using std::vector;

/** GcImageWriterPrivate **/

GcImageWriterPrivate::GcImageWriterPrivate(GcImageWriter *const q)
	: q(q)
{ }

GcImageWriterPrivate::~GcImageWriterPrivate()
{
	// Delete all files.
	// WARNING: Not thread-safe!
	for (int i = 0; i < (int)memBuffer.size(); i++)
		delete memBuffer[i];
	memBuffer.clear();
}

/**
 * Check if a vector of gcImages is CI8_UNIQUE.
 * @param gcImages	[in] Vector of GcImage.
 * @return True if the gcImages are CI8_UNIQUE; false if not.
 */
bool GcImageWriterPrivate::is_gcImages_CI8_UNIQUE(const vector<const GcImage*> *gcImages)
{
	const GcImage *gcImage0 = gcImages->at(0);
	const GcImage::PxFmt pxFmt = gcImage0->pxFmt();

	bool is_CI8_UNIQUE = false;
	if (pxFmt == GcImage::PXFMT_CI8) {
		// Check if all the palettes are identical.
		const uint32_t *palette0 = gcImage0->palette();
		for (int i = 1; i < (int)gcImages->size(); i++) {
			const GcImage *gcImageN = gcImages->at(i);
			const uint32_t *paletteN = gcImageN->palette();
			if (memcmp(palette0, paletteN, (256*sizeof(*paletteN))) != 0) {
				// CI8_UNIQUE.
				is_CI8_UNIQUE = true;
				break;
			}
		}
	}

	return is_CI8_UNIQUE;
}

/**
 * Check if a vector of gcImages is CI8_UNIQUE.
 * If they are, convert them to ARGB32 and return the new vector.
 * @param gcImages	[in] Vector of GcImage.
 * @return Vector of ARGB32 GcImage if CI8_UNIQUE, or nullptr otherwise.
 */
vector<const GcImage*> *GcImageWriterPrivate::gcImages_from_CI8_UNIQUE(const vector<const GcImage*> *gcImages)
{
	// NOTE: APNG only supports a single palette.
	// If the icon is CI8_UNIQUE, it will need to be
	// converted to ARGB32.
	// TODO: Test this; I don't have any files with CI8_UNIQUE...
	vector<const GcImage*> *gcImagesARGB32 = nullptr;
	if (is_gcImages_CI8_UNIQUE(gcImages)) {
		// CI8_UNIQUE. Convert to ARGB32.
		gcImagesARGB32 = new vector<const GcImage*>(gcImages->size());
		for (int i = 0; i < (int)gcImages->size(); i++) {
			const GcImage *gcImageN = gcImages->at(i);
			if (gcImageN)
				gcImageN = gcImageN->toRGB5A3();
			gcImagesARGB32->at(i) = gcImageN;
		}
	}

	return gcImagesARGB32;
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
#ifdef HAVE_PNG
		case IMGF_PNG:
			return true;
#endif /* HAVE_PNG */

		default:
			break;
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
 * Get the name of the specified image image format.
 * @param imgf Image format.
 * @return Name of the image format, or nullptr if invalid.
 */
const char *GcImageWriter::nameOfImageFormat(ImageFormat imgf)
{
	switch (imgf) {
		case IMGF_PNG:	return "PNG";
		default:	break;
	}

	return nullptr;
}

/**
 * Get the description of the specified image image format.
 * @param imgf Image format.
 * @return Description of the image format, or nullptr if invalid.
 */
const char *GcImageWriter::descOfImageFormat(ImageFormat imgf)
{
	switch (imgf) {
		case IMGF_PNG:	return "PNG image";
		default:	break;
	}

	return nullptr;
}

/**
 * Look up an image format from its name.
 * @param imgf_str Image format name.
 * @return Image format, or IMGF_UNKNOWN if unknown.
 */
GcImageWriter::ImageFormat GcImageWriter::imageFormatFromName(const char *imgf_str)
{
	if (!imgf_str) {
		return IMGF_UNKNOWN;
	} else if (!strcasecmp(imgf_str, "PNG")) {
		return IMGF_PNG;
	}

	// Unknown image format.
	return IMGF_UNKNOWN;
}

/**
 * Check if an animated image format is supported.
 * @param animImgf Animated image format.
 * @return True if supported; false if not.
 */
bool GcImageWriter::isAnimImageFormatSupported(AnimImageFormat animImgf)
{
	switch (animImgf) {
#ifdef HAVE_PNG
		case ANIMGF_APNG:
			return !!APNG_is_supported();
#endif /* HAVE_PNG */
#ifdef USE_GIF
		case ANIMGF_GIF:
			return (GifDlVersion() != 0);
#endif /* USE_GIF */
#ifdef HAVE_PNG
		case ANIMGF_PNG_FPF:
		case ANIMGF_PNG_VS:
		case ANIMGF_PNG_HS:
			return true;
#endif /* HAVE_PNG */
		default:
			break;
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
 * Get the name of the specified animated image format.
 * @param animImgf Animated image format.
 * @return Name of the animated image format, or nullptr if invalid.
 */
const char *GcImageWriter::nameOfAnimImageFormat(AnimImageFormat animImgf)
{
	switch (animImgf) {
		case ANIMGF_APNG:	return "APNG";
		case ANIMGF_GIF:	return "GIF";
		case ANIMGF_PNG_FPF:	return "PNG-FPF";
		case ANIMGF_PNG_VS:	return "PNG-VS";
		case ANIMGF_PNG_HS:	return "PNG-HS";
		default:		break;
	}

	return nullptr;
}

/**
 * Get the description of the specified animated image format.
 * @param animImgf Animated image format.
 * @return Description of the animated image format, or nullptr if invalid.
 */
const char *GcImageWriter::descOfAnimImageFormat(AnimImageFormat animImgf)
{
	switch (animImgf) {
		case ANIMGF_APNG:	return "Animated PNG image";
		case ANIMGF_GIF:	return "Animated GIF image";
		case ANIMGF_PNG_FPF:	return "PNG, file per frame";
		case ANIMGF_PNG_VS:	return "PNG, vertical strip";
		case ANIMGF_PNG_HS:	return "PNG, horizontal strip";
		default:		break;
	}

	return nullptr;
}

/**
 * Look up an animated image format from its name.
 * @param animImgf_str Animated image format name.
 * @return Animated image format, or IMGF_UNKNOWN if unknown.
 */
GcImageWriter::AnimImageFormat GcImageWriter::animImageFormatFromName(const char *animImgf_str)
{
	if (!animImgf_str) {
		return ANIMGF_UNKNOWN;
	} else if (!strcasecmp(animImgf_str, "APNG")) {
		return ANIMGF_APNG;
	} else if (!strcasecmp(animImgf_str, "GIF")) {
		return ANIMGF_GIF;
	} else if (!strcasecmp(animImgf_str, "PNG-FPF") ||
		   !strcasecmp(animImgf_str, "PNG_FPF") ||
		   !strcasecmp(animImgf_str, "PNG FPF")) {
		return ANIMGF_PNG_FPF;
	} else if (!strcasecmp(animImgf_str, "PNG-VS") ||
		   !strcasecmp(animImgf_str, "PNG_VS") ||
		   !strcasecmp(animImgf_str, "PNG VS")) {
		return ANIMGF_PNG_VS;
	} else if (!strcasecmp(animImgf_str, "PNG-HS") ||
		   !strcasecmp(animImgf_str, "PNG_HS") ||
		   !strcasecmp(animImgf_str, "PNG HS")) {
		return ANIMGF_PNG_HS;
	}

	// Unknown animated image format.
	return ANIMGF_UNKNOWN;
}

/**
 * Get the internal memory buffer. (first file only)
 * @return Internal memory buffer, or nullptr if no files are in memory.
 */
const vector<uint8_t> *GcImageWriter::memBuffer(void) const
{
	assert(!d->memBuffer.empty());
	if (d->memBuffer.empty())
		return nullptr;
	return d->memBuffer[0];
}

/**
 * Get the internal memory buffer for the specified file.
 * @param idx File number.
 * @return Internal memory buffer, or nullptr if the file index is invalid.
 */
const std::vector<uint8_t> *GcImageWriter::memBuffer(int idx) const
{
	assert(!d->memBuffer.empty());
	assert(idx >= 0);
	assert(idx < (int)d->memBuffer.size());
	if (idx < 0 || idx >= (int)d->memBuffer.size())
		return nullptr;
	return d->memBuffer[idx];
}

/**
 * Get the number of files currently in memory.
 * @return Number of files.
 */
int GcImageWriter::numFiles(void) const
{
	return (int)d->memBuffer.size();
}

/**
 * Clear the internal memory buffer.
 */
void GcImageWriter::clearMemBuffer(void)
{
	// WARNING: Not thread-safe!
	for (int i = 0; i < (int)d->memBuffer.size(); i++)
		delete d->memBuffer[i];
	d->memBuffer.clear();
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
#ifdef HAVE_PNG
		case IMGF_PNG:
			return d->writePng(gcImage);
#endif /* HAVE_PNG */
		default:
			break;
	}

	// Invalid image format.
	return -ENOSYS;
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
	assert(gcImages != nullptr);
	assert(!gcImages->empty());
	assert(gcImages->at(0) != nullptr);
	if (!gcImages || gcImages->empty() || !gcImages->at(0))
		return -EINVAL;
	if (!isAnimImageFormatSupported(animImgf))
		return -ENOSYS;

	// Adjust icon delays for NULL images.
	// NOTE: Assuming image 0 is always valid.

	// GcImages with NULL adjustments.
	vector<const GcImage*> adjGcImages;
	adjGcImages.reserve(gcImages->size());
	adjGcImages.push_back(gcImages->at(0));

	// Icon delays with NULL adjustments.
	vector<int> adjGcIconDelays;
	adjGcIconDelays.reserve(gcIconDelays->size());
	adjGcIconDelays.push_back(gcIconDelays->at(0));

	// Verify that all icons have the correct parameters.
	const GcImage *gcImage0 = gcImages->at(0);
	const int w = gcImage0->width();
	const int h = gcImage0->height();
	const GcImage::PxFmt pxFmt = gcImage0->pxFmt();
	for (int i = 1, lastNonNullAdjIdx = 0; i < (int)gcImages->size(); i++) {
		const GcImage *gcImageN = gcImages->at(i);
		if (!gcImageN) {
			// NULL image.
			// Assume it's the same as the previous image.
			adjGcIconDelays[lastNonNullAdjIdx] += gcIconDelays->at(i);
			continue;
		}

		// This icon is not NULL.
		lastNonNullAdjIdx = (int)adjGcIconDelays.size();
		adjGcImages.push_back(gcImages->at(i));
		adjGcIconDelays.push_back(gcIconDelays->at(i));

		if (gcImageN->width() != w ||
		    gcImageN->height() != h ||
		    gcImageN->pxFmt() != pxFmt)
		{
			// Animated icon is invalid.
			// TODO: Use a better error code.
			return -EINVAL;
		}
	}

	switch (animImgf) {
#ifdef HAVE_PNG
		case ANIMGF_APNG:
		case ANIMGF_PNG_FPF:
		case ANIMGF_PNG_VS:
		case ANIMGF_PNG_HS:
			return d->writePng_anim(&adjGcImages, &adjGcIconDelays, animImgf);
#endif /* HAVE_PNG */
#ifdef USE_GIF
		case ANIMGF_GIF:
			return d->writeGif_anim(&adjGcImages, &adjGcIconDelays);
#endif /* USE_GIF */
		default:
			break;
	}

	// Invalid image format.
	return -EINVAL;
}
