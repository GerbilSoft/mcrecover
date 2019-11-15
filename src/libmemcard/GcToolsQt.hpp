/***************************************************************************
 * GameCube Memory Card Recovery Program [libmemcard]                      *
 * GcToolsQt.hpp: libgctools Qt wrappers.                                  *
 *                                                                         *
 * Copyright (c) 2012-2013 by David Korth.                                 *
 * SPDX-License-Identifier: GPL-2.0-or-later                               *
 ***************************************************************************/

#ifndef __MCRECOVER_GCTOOLSQT_HPP__
#define __MCRECOVER_GCTOOLSQT_HPP__

// libgctools classes.
class GcImage;

// Qt includes.
#include <QtGui/QImage>

/**
 * Convert a GcImage to QImage.
 * NOTE: The resulting QImage will depend on the
 * GcImage for the actual data. Do not delete the
 * GcImage until you're done using the QImage!
 * @param gcImage GcImage.
 * @return QImage using the GcImage data, or null QImage on error.
 */
QImage gcImageToQImage(const GcImage *gcImage);

#endif /* __MCRECOVER_GCTOOLSQT_HPP__ */
