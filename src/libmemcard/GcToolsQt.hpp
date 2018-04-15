/***************************************************************************
 * GameCube Memory Card Recovery Program [libmemcard]                      *
 * GcToolsQt.hpp: libgctools Qt wrappers.                                  *
 *                                                                         *
 * Copyright (c) 2012-2013 by David Korth.                                 *
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
