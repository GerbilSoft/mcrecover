/***************************************************************************
 * GameCube Memory Card Recovery Program.                                  *
 * GcImage.hpp: GameCube image format handler.                             *
 *                                                                         *
 * Copyright (c) 2011 by David Korth.                                      *
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

#ifndef __MCRECOVER_GCIMAGE_HPP__
#define __MCRECOVER_GCIMAGE_HPP__

#include <QtCore/qglobal.h>
#include <QtGui/QImage>

class GcImage
{
	private:
		GcImage() { }
		~GcImage() { }
		Q_DISABLE_COPY(GcImage)
	
	public:
		/**
		 * Convert a GameCube CI8 image to QImage.
		 * @param w Image width.
		 * @param h Image height.
		 * @param buf CI8 image buffer.
		 * @param siz Size of data. [must be (w*h)+0x200]
		 * @return QImage, or empty QImage on error.
		 */
		static QImage FromCI8(int w, int h, const void *buf, int siz);
		
		/**
		 * Convert a GameCube RGB5A3 image to QImage.
		 * @param w Image width.
		 * @param h Image height.
		 * @param buf CI8 image buffer.
		 * @param siz Size of data. [must be (w*h)*2]
		 * @return QImage, or empty QImage on error.
		 */
		static QImage FromRGB5A3(int w, int h, const void *buf, int siz);
};

#endif /* __MCRECOVER_GCIMAGE_HPP__ */
