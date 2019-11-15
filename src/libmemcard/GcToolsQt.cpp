/***************************************************************************
 * GameCube Memory Card Recovery Program [libmemcard]                      *
 * GcToolsQt.cpp: libgctools Qt wrappers.                                  *
 *                                                                         *
 * Copyright (c) 2012-2013 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#include "GcToolsQt.hpp"

// libgctools
#include "libgctools/GcImage.hpp"

// C++ includes.
#include <vector>
using std::vector;

/**
 * Convert a GcImage to QImage.
 * NOTE: The resulting QImage will depend on the
 * GcImage for the actual data. Do not delete the
 * GcImage until you're done using the QImage!
 * @param gcImage GcImage.
 * @return QImage using the GcImage data, or null QImage on error.
 */
QImage gcImageToQImage(const GcImage *gcImage)
{
	if (!gcImage)
		return QImage();

	QImage::Format imgFmt;
	GcImage::PxFmt pxFmt = gcImage->pxFmt();
	switch (pxFmt) {
		case GcImage::PXFMT_CI8:
			imgFmt = QImage::Format_Indexed8;
			break;
		case GcImage::PXFMT_ARGB32:
			imgFmt = QImage::Format_ARGB32;
			break;
		default:
			imgFmt = QImage::Format_Invalid;
			break;
	}

	if (imgFmt == QImage::Format_Invalid)
		return QImage();

	QImage qImg((const uint8_t*)gcImage->imageData(),
		gcImage->width(), gcImage->height(), imgFmt);

	// CI8 images: Set the palette.
	if (pxFmt == GcImage::PXFMT_CI8) {
		const uint32_t *palette = gcImage->palette();
		if (palette) {
			vector<uint32_t> vPalette;
			vPalette.assign(palette, palette + 256);
			qImg.setColorTable(QVector<QRgb>::fromStdVector(vPalette));
		}
	}

	return qImg;
}
